from .z3types import Ast, ContextObj, OptimizeObj

def Z3_mk_eq(ctx: ContextObj, a: Ast, b: Ast) -> Ast: ...
def Z3_mk_div(ctx: ContextObj, a: Ast, b: Ast) -> Ast: ...
def Z3_mk_numeral(ctx: ContextObj, a: str, b: Ast) -> Ast: ...
def Z3_inc_ref(ctx: ContextObj, a: Ast) -> None: ...
def Z3_dec_ref(ctx: ContextObj, a: Ast) -> None: ...
def Z3_mk_string_symbol(ctx: ContextObj, symbol: str) -> Ast: ...
def Z3_mk_const(ctx: ContextObj, symbol: Ast, sort: Ast) -> Ast: ...
def Z3_optimize_assert(ctx: ContextObj, optimizer: OptimizeObj, a: Ast): ...
