#!/usr/bin/env python3
from __future__ import annotations
import abc
import argparse
import fractions
import math
import operator
import sys
import typing as typ

import z3
from z3 import z3util

PARSER = argparse.ArgumentParser(
    description='Calculate coverage from samples file and smt2 file')
PARSER.add_argument('-s',
                    '--samples',
                    metavar='FILE',
                    type=open,
                    required=True,
                    help="File to load samples from")
PARSER.add_argument('-f',
                    '--formula',
                    metavar='FILE',
                    type=open,
                    required=True,
                    help="Formula file (smt2)")
PARSER.add_argument('--use-c-api',
                    action='store_true',
                    help="Use Z3 C API calls to load samples, if applicable")
PARSER.add_argument('-m', '--metric',
                    required=True,
                    choices=["satisfies", "wire_coverage"])

CTX = z3.Context()


class Metric(abc.ABC):
    def __init__(self, formula: str):
        self._solver = z3.Solver(ctx=CTX)
        self._total = 0
        self._satisfies = 0
        self._solver.from_string(formula)

    @abc.abstractmethod
    def count_sample(self, sample: list[tuple[str, int]]):
        pass

    @property
    def result(self) -> fractions.Fraction:
        return fractions.Fraction(self._satisfies, self._total)


class SatisfiesMetric(Metric):
    def __init__(self, formula: str, use_c_api: bool = False):
        super().__init__(formula)
        self._intsort = z3.IntSort(ctx=CTX)
        self._use_c_api = use_c_api
        #self._vars = {str(var): var for var in z3util.get_vars(g.as_expr())}

    def _add_sample_via_c_api(self, sample: list[tuple[str, int]]):
        for var, value in sample:
            numeral = z3.Z3_mk_numeral(CTX.ref(), str(value),
                                       self._intsort.ast)
            z3.Z3_inc_ref(CTX.ref(), numeral)
            symbol = z3.Z3_mk_string_symbol(CTX.ref(), var)
            const = z3.Z3_mk_const(CTX.ref(), symbol, self._intsort.ast)
            #z3.Z3_inc_ref(CTX.ref(), const)
            eq = z3.Z3_mk_eq(CTX.ref(), const, numeral)
            #z3.Z3_inc_ref(CTX.ref(), const)
            z3.Z3_solver_assert(CTX.ref(), self._solver.solver, eq)
            # Now they are Z3's?
            z3.Z3_dec_ref(CTX.ref(), numeral)
            #z3.Z3_dec_ref(CTX.ref(), const)
            #z3.Z3_dec_ref(CTX.ref(), eq)

    def _add_sample_via_smtlib(self, sample: list[tuple[str, int]]):
        self._solver.from_string("".join(
            f"(declare-fun {var} () Int)\n(assert (= {var} {value}))"
            for var, value in sample))

    def _add_sample(self, sample: list[tuple[str, int]]):
        if self._use_c_api:
            self._add_sample_via_c_api(sample)
        else:
            self._add_sample_via_smtlib(sample)

    def _check_sample(self, sample: list[tuple[str,
                                               int]]) -> z3.CheckSatResult:
        # Easiest way to do this seems to just ask Z3...
        # Hope this isn't **too** costly.
        self._solver.push()
        self._add_sample(sample)
        r = self._solver.check()
        self._solver.pop()
        return r

    def count_sample(self, sample: list[tuple[str, int]]):
        result = self._check_sample(sample)
        if result == z3.sat:
            self._satisfies += 1
        self._total += 1

        if self._total % 1000 == 0:
            print(f"{self._satisfies}/{self._total}")


class ManualSatisfiesMetric(Metric):
    def __init__(self, formula: str, statistics: NodeStatistics = None):
        super().__init__(formula)
        self._statistics = statistics
        expr = z3.And(self._solver.assertions())
        self._evaluator = self._build_evaluator(expr)

    def count_sample(self, sample: list[tuple[str, int]]):
        model = dict(sample)

        if self._evaluator(model):
            self._satisfies += 1
        self._total += 1

        if self._total % 1000 == 0:
            print(f"{self._satisfies}/{self._total}")

    @property
    def result(self) -> fractions.Fraction:
        if self._statistics:
            return self._statistics.result
        return super().result

    def _build_evaluator(self, expr):
        return self._build_bool(expr)

    def _build_bool(self, expr):
        assert (z3.is_bool(expr))
        if self._statistics:
            self._statistics.register_node(expr.get_id(), "bool")

        if z3.is_and(expr):
            return self._build_nary(expr, all, self._build_bool)
        elif z3.is_or(expr):
            return self._build_nary(expr, any, self._build_bool)
        elif z3.is_not(expr):
            return self._build_unary_not(expr)
        elif z3.is_le(expr):
            return self._build_binary(expr, operator.le, self._build_int)
        elif z3.is_lt(expr):
            return self._build_binary(expr, operator.lt, self._build_int)
        elif z3.is_gt(expr):
            return self._build_binary(expr, operator.gt, self._build_int)
        elif z3.is_ge(expr):
            return self._build_binary(expr, operator.ge, self._build_int)
        elif z3.is_eq(expr):
            return self._build_binary(expr, operator.eq, self._build_int)
        raise Exception(f"Unhandled: {expr}")

    def _build_int(self, expr):
        assert (z3.is_int(expr))
        if self._statistics:
            self._statistics.register_node(expr.get_id(), "int")

        if z3.is_add(expr):
            return self._build_nary(expr, sum, self._build_int)
        elif z3.is_sub(expr):
            return self._build_binary(expr, operator.sub, self._build_int)
        elif z3.is_mul(expr):
            return self._build_nary(expr, math.prod, self._build_int)
        elif z3.is_app_of(expr, z3.Z3_OP_UMINUS):
            return self._build_unary_minus(expr)
        elif z3.is_int_value(expr):
            return self._build_leaf_literal(expr)
        elif z3.is_const(expr):
            return self._build_leaf_symbol(expr)
        raise Exception(f"Unhandled: {expr}")

    def _build_nary(self, expr, op, subtype):
        children = [subtype(subexpr) for subexpr in expr.children()]
        node_id = expr.get_id()

        def e(model):
            value = op(child(model) for child in children)
            if self._statistics:
                self._statistics.evaluate_node(node_id, value)
            return value

        return e

    def _build_binary(self, expr, op, subtype):
        left, right = [subtype(subexpr) for subexpr in expr.children()]
        node_id = expr.get_id()

        def e(model):
            value = op(left(model), right(model))
            if self._statistics:
                self._statistics.evaluate_node(node_id, value)
            return value

        return e

    def _build_unary_not(self, expr):
        child = self._build_bool(expr.arg(0))
        node_id = expr.get_id()

        def e(model):
            value = not child(model)
            if self._statistics:
                self._statistics.evaluate_node(node_id, value)
            return value

        return e

    def _build_unary_minus(self, expr):
        child = self._build_int(expr.arg(0))
        node_id = expr.get_id()

        def e(model):
            value = -child(model)
            if self._statistics:
                self._statistics.evaluate_node(node_id, value)
            return value

        return e

    def _build_leaf_symbol(self, expr):
        name = expr.decl().name()
        node_id = expr.get_id()

        def e(model):
            value = model[name]
            if self._statistics:
                self._statistics.evaluate_node(node_id, value)
            return value

        return e

    def _build_leaf_literal(self, expr):
        value = expr.as_long()
        node_id = expr.get_id()

        def e(model):
            if self._statistics:
                self._statistics.evaluate_node(node_id, value)
            return value

        return e


class NodeStatistics(abc.ABC):
    def __init__(self):
        self._storage = {}

    @abc.abstractmethod
    def register_node(self, node_id: int, sort: str):
        pass

    @abc.abstractmethod
    def evaluate_node(self, node_id: int, value: typ.Any):
        pass

    @property
    @abc.abstractmethod
    def result(self) -> fractions.Fraction:
        pass


class WireCoverageStatistics(NodeStatistics):
    MASK = 2**64-1

    def register_node(self, node_id: int, sort: str):
        if sort == "bool":
            self._storage[node_id] = (False, False, sort)
        elif sort == "int":
            self._storage[node_id] = (0, 0, sort)
        else:
            raise Exception(f"Unhandled: {sort}")

    def evaluate_node(self, node_id: int, value: typ.Any):
        old_true, old_false, sort = self._storage[node_id]
        if sort == "bool":
            self._storage[node_id] = (old_true or value,
                                      old_false or not value,
                                      sort)
        elif sort == "int":
            self._storage[node_id] = (old_true | (value & self.MASK),
                                      old_false | ((value & self.MASK) ^ self.MASK),
                                      sort)
        else:
            raise Exception(f"Unhandled: {sort}")


    @property
    def result(self) -> fractions.Fraction:
        count = 0
        total = 0
        for true_count, false_count, sort in self._storage.values():
            if sort == "bool":
                total += 1
                if true_count and false_count:
                    count += 1
            elif sort == "int":
                total += bin(self.MASK).count('1')
                count += bin(true_count & false_count).count('1')
            else:
                raise Exception(f"Unhandled: {sort}")

        return fractions.Fraction(count, total)

def _load_formula(f: typ.TextIO) -> str:
    return f.read()


def _apply_metric(metric: Metric, samples: typ.Iterator[list[tuple[str,
                                                                   int]]]):
    for sample in samples:
        metric.count_sample(sample)


def _parse_samples(f: typ.TextIO) -> typ.Iterator[list[tuple[str, int]]]:
    def to_tuple(sample):
        var, value = sample.split(':')
        return var, int(value.strip('()').replace(' ', ''))

    for line in f:
        p = line.split(' ', maxsplit=1)[1].strip('; \n').split(';')
        yield [to_tuple(x) for x in p]


def main():
    args = PARSER.parse_args(sys.argv[1:])

    formula = _load_formula(args.formula)
    samples = _parse_samples(args.samples)

    if args.metric == 'satisfies':
        metric = SatisfiesMetric(formula, use_c_api=args.use_c_api)
    elif args.metric == 'wire_coverage':
        metric = ManualSatisfiesMetric(formula, statistics=WireCoverageStatistics())

    _apply_metric(metric, samples)
    print(metric.result)


if __name__ == '__main__':
    main()