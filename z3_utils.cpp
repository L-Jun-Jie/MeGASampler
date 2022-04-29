//
// Created by batchen on 28/04/2022.
//

#include "z3_utils.h"

z3::expr negate_condition(const z3::expr& cond){
  if (cond.num_args() < 2){
    throw UnsupportedOperator();
  } else {
    const z3::expr& arg0 = cond.arg(0);
    const z3::expr& arg1 = cond.arg(1);
    Z3_decl_kind op = get_op(cond);
    if (is_op_le(op)) return arg0 > arg1;
    if (is_op_lt(op)) return arg0 >= arg1;
    if (is_op_ge(op)) return arg0 < arg1;
    if (is_op_gt(op)) return arg0 <= arg1;
    if (is_op_eq(op)) return arg0 != arg1;
    if (is_op_distinct(op)) return arg0 == arg1;
    throw UnsupportedOperator();
  }
}

bool model_eval_to_bool(const z3::model& model, const z3::expr& bool_expr){
  assert(bool_expr.is_bool());
  return model.eval(bool_expr, true).is_true();
}

bool is_lt(const z3::expr& expr){
  auto op = get_op(expr);
  return is_op_lt(op);
}

bool is_gt(const z3::expr& expr){
  auto op = get_op(expr);
  return is_op_gt(op);
}

bool is_eq(const z3::expr& expr){
  auto op = get_op(expr);
  return is_op_eq(op);
}

bool is_distinct(const z3::expr& expr){
  auto op = get_op(expr);
  return is_op_distinct(op);
}

bool is_le(const z3::expr& expr){
  auto op = get_op(expr);
  return is_op_le(op);
}

bool is_ge(const z3::expr& expr){
  auto op = get_op(expr);
  return is_op_ge(op);
}

Z3_decl_kind get_op(const z3::expr& expr){
  assert(expr.is_app());
  return expr.decl().decl_kind();
}

bool is_binary_boolean(const z3::expr& expr){
  Z3_decl_kind op = get_op(expr);
  return (is_op_lt(op) || is_op_le(op) || is_op_gt(op) || is_op_ge(op) || is_op_eq(op) || is_op_distinct(op));
}

z3::expr simplify_strict_to_nonstrict(const z3::expr& expr){
  assert(expr.num_args() == 2);
  const z3::expr& arg0 = expr.arg(0);
  const z3::expr& arg1 = expr.arg(1);
  Z3_decl_kind op = get_op(expr);
  if (is_op_gt(op)){
    return arg0 >= arg1 + 1;
  } else if (is_op_lt(op)){
    return arg0 <= arg1 - 1;
  } else {
    throw UnsupportedOperator();
  }
}

std::string op_to_string(Z3_decl_kind op){
  if (is_op_le(op)){
    return "<=";
  } else if (is_op_lt(op)){
    return "<";
  } else if (is_op_ge(op)){
    return ">=";
  } else if (is_op_gt(op)){
    return ">";
  } else if (is_op_eq(op)){
    return "==";
  } else if (is_op_distinct(op)){
    return "!=";
  } else if (is_op_and(op)){
    return "and";
  } else if (is_op_or(op)){
    return "or";
  } else if (is_op_add(op)){
    return "+";
  } else if (is_op_sub(op)){
    return "-";
  } else if (is_op_mul(op)){
    return "*";
  } else if (is_op_div(op)){
    return "/";
  } else if (is_op_mod(op)){
    return "%";
  } else if (is_op_rem(op)){
    return "%";
  } else if (is_op_uminus(op)){
    return "-";
  } else {
    throw UnsupportedOperator();
  }
}

bool is_op_le(Z3_decl_kind op) {return op==Z3_OP_LE || op==Z3_OP_SLEQ;}
bool is_op_lt(Z3_decl_kind op) {return op==Z3_OP_LT || op==Z3_OP_SLT;}
bool is_op_ge(Z3_decl_kind op) {return op==Z3_OP_GE || op==Z3_OP_SGEQ;}
bool is_op_gt(Z3_decl_kind op) {return op==Z3_OP_GT || op==Z3_OP_SGT;}
bool is_op_eq(Z3_decl_kind op) {return op==Z3_OP_EQ;}
bool is_op_distinct(Z3_decl_kind op) {return op==Z3_OP_DISTINCT;}
bool is_op_add(Z3_decl_kind op) {return op==Z3_OP_ADD || op==Z3_OP_BADD;}
bool is_op_sub(Z3_decl_kind op) {return op==Z3_OP_SUB || op==Z3_OP_BSUB;}
bool is_op_mul(Z3_decl_kind op) {return op==Z3_OP_MUL || op==Z3_OP_BMUL;}
bool is_op_div(Z3_decl_kind op) {return op==Z3_OP_DIV || op==Z3_OP_IDIV || op==Z3_OP_BSDIV || op==Z3_OP_BSDIV0 || op==Z3_OP_BUDIV || op==Z3_OP_BUDIV0;}
bool is_op_rem(Z3_decl_kind op) {return op==Z3_OP_REM || op==Z3_OP_BSREM || op==Z3_OP_BSREM0 || op==Z3_OP_BUREM || op==Z3_OP_BUREM0;}
bool is_op_mod(Z3_decl_kind op) {return op==Z3_OP_MOD || op==Z3_OP_BSMOD || op==Z3_OP_BSMOD0;}
bool is_op_and(Z3_decl_kind op) {return op==Z3_OP_AND || op==Z3_OP_BAND || op==Z3_OP_BREDAND;}
bool is_op_or(Z3_decl_kind op) {return op==Z3_OP_OR || op==Z3_OP_BOR || op==Z3_OP_BREDOR;}
bool is_op_not(Z3_decl_kind op) {return op==Z3_OP_NOT;}
bool is_op_uminus(Z3_decl_kind op) {return op==Z3_OP_UMINUS;}
bool is_op_select(Z3_decl_kind op) {return op==Z3_OP_SELECT;}
bool is_op_store(Z3_decl_kind op) {return op==Z3_OP_STORE;}
bool is_op_ite(Z3_decl_kind op) {return op==Z3_OP_ITE;}
bool is_op_uninterpreted(Z3_decl_kind op) {return op==Z3_OP_UNINTERPRETED;}