from typing import Any, Callable, Dict, Optional, Tuple, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_binop import BINOP_EXEC_FUNCS, BinopExpr, BinopType
from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, Literal, StringLiteral
from ede_ast.ede_stmt import ExprStmt, VarDeclStmt
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, RecordTypeSymbol, TupleTypeSymbol, TypeSymbol
from ede_ast.ede_typesystem import EdeType, Environment
from ede_ast.ede_visitors.ede_typecheck_visitor import TypecheckVisitor
from ede_utils import Error, Result, Success
from interpreter import ExecContext, ExecValue

ExecResult = Optional[ExecValue]

class ExecutionVisitor:
    '''
    Under the assumption of a successful type checking, simulates execution
    of the node in the given execution context.
    '''

    @staticmethod
    def visit(node: Node, ctx: ExecContext) -> ExecResult:
        assert node.get_node_type() is not None # technically this is redunant since get_node_type asserts already
        return VISITORS[type(node)](node, ctx)

    @staticmethod
    def visit_in(node: Node, env: Environment, ctx: ExecContext) -> Result[ExecResult]:
        if env is not None:
            tc_res = TypecheckVisitor.visit(node, env)
            if tc_res.is_error():
                return cast(Error, tc_res)
        
        return Success(ExecutionVisitor.visit(node, ctx))

def visit_IdentifierExpr(expr: IdentifierExpr, ctx: ExecContext) -> ExecResult:
    return ctx.get(expr.id)

def visit_BinopExpr(expr: BinopExpr, ctx: ExecContext) -> ExecResult:
    if expr.op == BinopType.ASSIGN:
        id = cast(IdentifierExpr, expr.left).id

        # execute RHS; return if exception
        right_res = cast(ExecValue, ExecutionVisitor.visit(expr.right, ctx))
        if right_res.is_exception():
            return right_res

        # set the value of the id to the value of the expression
        ctx.set(id, right_res, expr.position)
        return right_res
    else:
        # execute LHS; return if exception
        left_res = cast(ExecValue, ExecutionVisitor.visit(expr.left, ctx))
        if left_res.is_exception():
            return left_res

        # execute RHS; return if exception
        right_res = cast(ExecValue, ExecutionVisitor.visit(expr.right, ctx))
        if right_res.is_exception():
            return right_res

        # execute function associated with pattern
        return BINOP_EXEC_FUNCS[cast(Tuple[EdeType, EdeType, BinopType], expr.type_pattern)](left_res, right_res, expr.position, ctx)

def visit_Literal(expr: Literal[Any], ctx: ExecContext) -> ExecResult:
    return ExecValue(expr.value)

def visit_VarDeclStmt(stmt: VarDeclStmt, ctx: ExecContext) -> ExecResult:
    ctx.set(stmt.id, ExecutionVisitor.visit(stmt.expr, ctx) if stmt.expr is not None else None, stmt.position)
    return None

def visit_TypeSymbol(t: TypeSymbol, ctx: ExecContext) -> ExecResult:
    assert False, 'Type symbols are not executable'

VISITORS : Dict[Type[Any], Callable[[Any, ExecContext], ExecResult]] = {
    ExprStmt: lambda node, env: ExecutionVisitor.visit(cast(ExprStmt, node).expr, env),
    IdentifierExpr: visit_IdentifierExpr,
    BinopExpr: visit_BinopExpr,
    IntLiteral: visit_Literal,
    CharLiteral: visit_Literal,
    StringLiteral: visit_Literal,
    BoolLiteral: visit_Literal,
    VarDeclStmt: visit_VarDeclStmt,
    PrimitiveTypeSymbol: visit_TypeSymbol,
    ArrayTypeSymbol: visit_TypeSymbol,
    TupleTypeSymbol: visit_TypeSymbol,
    RecordTypeSymbol: visit_TypeSymbol,
    NameTypeSymbol: visit_TypeSymbol
}