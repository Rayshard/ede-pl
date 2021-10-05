from typing import Any, Callable, Dict, Tuple, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_binop import BINOP_EXEC_FUNCS, BinopExpr, BinopType
from ede_ast.ede_expr import ArrayExpr, IdentifierExpr, ObjInitExpr, TupleExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, Literal, StringLiteral
from ede_ast.ede_module import Module
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, VarDeclStmt
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol, TypeSymbol
from ede_ast.ede_typesystem import EdeType, Environment
from ede_ast.ede_visitors.ede_typecheck_visitor import TypecheckVisitor
from ede_utils import Result, Success
from interpreter import ArrayValue, ExecContext, ExecValue, ObjectValue, TupleValue

class ExecutionVisitor:
    '''
    Under the assumption of a successful type checking, simulates execution
    of the node in the given execution context.
    '''

    @staticmethod
    def visit(node: Node, ctx: ExecContext) -> ExecValue:
        assert node.get_node_type() is not None # technically this is redunant since get_node_type asserts already
        return VISITORS[type(node)](node, ctx)

    @staticmethod
    def visit_in(node: Node, env: Environment, ctx: ExecContext) -> Result[ExecValue]:
        if env is not None:
            tc_res = TypecheckVisitor.visit(node, env)
            if tc_res.is_error():
                return tc_res.error()
        
        return Success(ExecutionVisitor.visit(node, ctx))

def visit_IdentifierExpr(expr: IdentifierExpr, ctx: ExecContext) -> ExecValue:
    get_res = ctx.get(expr.id)

    assert get_res is not None, 'Type checking should have ensured that the id was assigned'
    return get_res

def visit_BinopExpr(expr: BinopExpr, ctx: ExecContext) -> ExecValue:
    if expr.op == BinopType.ASSIGN:
        id = cast(IdentifierExpr, expr.left).id

        # execute RHS; return if exception
        right_res = ExecutionVisitor.visit(expr.right, ctx)
        if right_res.is_exception():
            return right_res

        # set the value of the id to the value of the expression
        ctx.set(id, right_res, expr.position)
        return right_res
    else:
        # execute LHS; return if exception
        left_res = ExecutionVisitor.visit(expr.left, ctx)
        if left_res.is_exception():
            return left_res

        # execute RHS; return if exception
        right_res = ExecutionVisitor.visit(expr.right, ctx)
        if right_res.is_exception():
            return right_res

        # execute function associated with pattern
        return BINOP_EXEC_FUNCS[cast(Tuple[EdeType, EdeType, BinopType], expr.type_pattern)](left_res, right_res, expr.position, ctx)

def visit_Literal(expr: Literal[Any], ctx: ExecContext) -> ExecValue:
    return ExecValue(expr.value)

def visit_VarDeclStmt(stmt: VarDeclStmt, ctx: ExecContext) -> ExecValue:
    expr_res = ExecutionVisitor.visit(stmt.expr, ctx) if stmt.expr is not None else None
    if expr_res is not None and expr_res.is_exception():
        return expr_res

    ctx.set(stmt.id, expr_res, stmt.position)
    return ExecValue.UNIT()

def visit_IfElseStmt(stmt: IfElseStmt, ctx: ExecContext) -> ExecValue:
    cond_res = ExecutionVisitor.visit(stmt.condition, ctx)
    if cond_res.is_exception():
        return cond_res

    if cond_res.to_bool():
        return ExecutionVisitor.visit(stmt.thenClause, ctx)
    elif stmt.elseClause is not None:
        return ExecutionVisitor.visit(stmt.elseClause, ctx)

    return ExecValue.UNIT()

def visit_TypeSymbol(t: TypeSymbol, ctx: ExecContext) -> ExecValue:
    assert False, 'Type symbols are not executable'

def visit_Block(b: Block, ctx: ExecContext) -> ExecValue:
    sub_ctx = ExecContext(ctx)

    for stmt in b.stmts:
        exec_res = ExecutionVisitor.visit(stmt, sub_ctx)
        if exec_res.is_exception():
            return exec_res

    return ExecValue.UNIT()

def visit_ArrayExpr(a: ArrayExpr, ctx: ExecContext) -> ExecValue:
    value = ArrayValue([])

    for expr in a.exprs:
        expr_value = ExecutionVisitor.visit(expr, ctx)
        if expr_value.is_exception():
            return expr_value

        value.values.append(expr_value)

    return ExecValue(value)

def visit_TupleExpr(t: TupleExpr, ctx: ExecContext) -> ExecValue:
    value = TupleValue([])

    for expr in t.exprs:
        expr_value = ExecutionVisitor.visit(expr, ctx)
        if expr_value.is_exception():
            return expr_value

        value.values.append(expr_value)

    return ExecValue(value)

def visit_ObjInitExpr(oi: ObjInitExpr, ctx: ExecContext) -> ExecValue:
    obj_type = cast(EdeObject, name_tc_res.get())
    expected_members = obj_type.get_members()
    inited_members = []

    for id, value in oi.items.items():
        if id.value in inited_members:
            return TypeCheckError.Reinitialization(id.value, id.position)
        elif id.value not in expected_members:
            return TypeCheckError.UnexpectedInitialization(id.value, id.position)

        expected_type = expected_members[id.value]
        value_type_res = TypecheckVisitor.visit(value, env)
        if value_type_res.is_error():
            return value_type_res
        elif value_type_res.get() != expected_type:
            return TypeCheckError.UnexpectedType(value_type_res.get(), expected_type, value.position)

        inited_members.append(id)

def visit_Module(m: Module, ctx: ExecContext) -> ExecValue:
    sub_ctx = ExecContext(ctx)

    for stmt in m.stmts:
        exec_res = ExecutionVisitor.visit(stmt, sub_ctx)
        if exec_res.is_exception():
            return exec_res

    return ExecValue.UNIT()

def visit_ExprStmt(e: ExprStmt, ctx: ExecContext) -> ExecValue:
    value = ExecutionVisitor.visit(e.expr, ctx)
    print(value)
    return ExecValue.UNIT()

VISITORS : Dict[Type[Any], Callable[[Any, ExecContext], ExecValue]] = {
    ExprStmt: visit_ExprStmt,
    IdentifierExpr: visit_IdentifierExpr,
    Module: visit_Module,
    ArrayExpr: visit_ArrayExpr,
    TupleExpr: visit_TupleExpr,
    ObjInitExpr: visit_ObjInitExpr,
    BinopExpr: visit_BinopExpr,
    IfElseStmt: visit_IfElseStmt,
    IntLiteral: visit_Literal,
    CharLiteral: visit_Literal,
    StringLiteral: visit_Literal,
    BoolLiteral: visit_Literal,
    VarDeclStmt: visit_VarDeclStmt,
    PrimitiveTypeSymbol: visit_TypeSymbol,
    ArrayTypeSymbol: visit_TypeSymbol,
    TupleTypeSymbol: visit_TypeSymbol,
    NameTypeSymbol: visit_TypeSymbol,
    Block: visit_Block
}