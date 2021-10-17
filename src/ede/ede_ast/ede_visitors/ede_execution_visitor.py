import sys
from typing import Any, Callable, Dict, Optional, TextIO, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_context import CtxEntryType
from ede_ast.ede_definition import FuncDef, ObjDef
from ede_ast.ede_expr import ArrayInitExpr, DefaultExpr, FuncCallExpr, IdentifierExpr, ObjInitExpr, BinopExpr, BinopType, TupleInitExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, Literal, StringLiteral, UnitLiteral
from ede_ast.ede_module import Module
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, ReturnStmt, VarDeclStmt
from ede_ast.ede_type_symbol import TypeSymbol
from ede_ast.ede_typesystem import EdeFunc, EdeObject, EdePrimitive, TCContext, TSPrimitiveType
from ede_ast.ede_visitors.ede_typecheck_visitor import TypecheckVisitor
from ede_utils import Result, Success
from interpreter import ArrayValue, ExecContext, ExecEntry, ExecException, ExecValue, ObjectValue, TupleValue

class ExecutionVisitor:
    '''
    Under the assumption of a successful type checking, simulates execution
    of the node in the given execution context.
    '''

    Output_Stream : TextIO = sys.stdout

    @staticmethod
    def visit(node: Node, ctx: ExecContext) -> ExecValue:
        return VISITORS[type(node)](node, ctx)

    @staticmethod
    def visit_in(node: Node, env: TCContext, ctx: ExecContext) -> Result[ExecValue]:
        if env is not None:
            tc_res = TypecheckVisitor.visit(node, env)
            if tc_res.is_error():
                return tc_res.error()
        
        return Success(ExecutionVisitor.visit(node, ctx))

    @staticmethod
    def output(stuff: Any, new_line: bool = True) -> None:
        ExecutionVisitor.Output_Stream.write(str(stuff) + '\n' if new_line else '')

def visit_BinopExpr(expr: BinopExpr, ctx: ExecContext) -> ExecValue:
    if expr.op == BinopType.ASSIGN:
        id = cast(IdentifierExpr, expr.left).id

        # execute RHS; return if exception
        right_res = ExecutionVisitor.visit(expr.right, ctx)
        if right_res.is_exception():
            return right_res

        # set the value of the id to the value of the expression
        # ** because we want the position of the variables entry to always be it's declaration position, this set function will be slow
        ctx.set(id, ExecEntry(CtxEntryType.VARIABLE, expr.get_ede_type(), right_res, ctx.get(id, expr.position, True).get().pos))
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
        match expr.type_pattern:
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.ADD):
                return ExecValue(left_res.to_int() + right_res.to_int())
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.SUB):
                return ExecValue(left_res.to_int() - right_res.to_int())
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.MUL):
                return ExecValue(left_res.to_int() * right_res.to_int())
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.DIV):
                return ExecValue(left_res.to_int() // right_res.to_int() if right_res.to_int() != 0 else ExecException.DivisionByZero(expr.position))
            case (EdePrimitive(prim_type=TSPrimitiveType.STR), EdePrimitive(prim_type=TSPrimitiveType.STR), BinopType.ADD):
                return ExecValue(left_res.to_str() + right_res.to_str())
            case _:
                raise Exception("Case not handled")

def visit_Literal(expr: Literal[Any], ctx: ExecContext) -> ExecValue:
    return ExecValue(expr.value)

def visit_VarDeclStmt(stmt: VarDeclStmt, ctx: ExecContext) -> ExecValue:
    expr_res : Optional[ExecValue] = None
    ede_type = EdePrimitive.UNIT()
    
    if stmt.expr is None:
        ede_type = cast(TypeSymbol, stmt.type_symbol).get_ede_type()
        expr_res = ExecValue.get_default_value(ede_type)
    else:
        ede_type = stmt.expr.get_ede_type() if stmt.type_symbol is None else stmt.type_symbol.get_ede_type()
        expr_res = ExecutionVisitor.visit(stmt.expr, ctx)
        if expr_res.is_exception():
            return expr_res

    ctx.add(stmt.id, ExecEntry(CtxEntryType.VARIABLE, ede_type, expr_res, stmt.position), False)
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

def visit_DefaultExpr(d: DefaultExpr, ctx: ExecContext) -> ExecValue:
    return ExecValue.get_default_value(d.get_ede_type())

def visit_Block(b: Block, ctx: ExecContext) -> ExecValue:
    sub_ctx = ExecContext(ctx)

    for stmt in b.stmts:
        exec_res = ExecutionVisitor.visit(stmt, sub_ctx)
        if exec_res.is_exception():
            return exec_res

    return ExecValue.UNIT()

def visit_ArrayInitExpr(ai: ArrayInitExpr, ctx: ExecContext) -> ExecValue:
    value = ArrayValue([])

    for expr in ai.exprs:
        expr_value = ExecutionVisitor.visit(expr, ctx)
        if expr_value.is_exception():
            return expr_value

        value.values.append(expr_value)

    return ExecValue(value)

def visit_TupleInitExpr(ti: TupleInitExpr, ctx: ExecContext) -> ExecValue:
    value = TupleValue([])

    for expr in ti.exprs:
        expr_value = ExecutionVisitor.visit(expr, ctx)
        if expr_value.is_exception():
            return expr_value

        value.values.append(expr_value)

    return ExecValue(value)

def visit_ObjInitExpr(oi: ObjInitExpr, ctx: ExecContext) -> ExecValue:
    members : Dict[str, ExecValue] = {}

    for id, value in oi.items.items():
        value_type_res = ExecutionVisitor.visit(value, ctx)
        if value_type_res.is_exception():
            return value_type_res
        
        members[id.value] = value_type_res

    expected_members = cast(EdeObject, oi.get_ede_type()).get_members()
    for id, type in expected_members.items():
        if id not in members:
            members[id] = ExecValue.get_default_value(type)

    return ExecValue(ObjectValue(oi.name, members))

def visit_ObjDef(o: ObjDef, ctx: ExecContext) -> ExecValue:
    # technically these don't execute but it'll allow us to add info to the context

    ctx.add(o.name, ExecEntry(CtxEntryType.TYPENAME, o.get_ede_type(), ExecValue.UNIT(), o.position), False)
    return ExecValue.UNIT()

def visit_FuncDef(f: FuncDef, ctx: ExecContext) -> ExecValue:
    # technically these don't execute but it'll allow us to add info to the context

    ctx.add(f.name, ExecEntry(CtxEntryType.FUNCTION, f.get_ede_type(), ExecValue(f.body), f.position), False)
    return ExecValue.UNIT()

def visit_Module(m: Module, ctx: ExecContext) -> ExecValue:
    sub_ctx = ExecContext(ctx)
    sub_ctx = ctx
    for definition in m.defs:
        def_res = ExecutionVisitor.visit(definition, sub_ctx)
        if def_res.is_exception():
            return def_res

    for stmt in m.stmts:
        exec_res = ExecutionVisitor.visit(stmt, sub_ctx)
        if exec_res.is_exception():
            return exec_res

    return ExecValue.UNIT()

def visit_ExprStmt(e: ExprStmt, ctx: ExecContext) -> ExecValue:
    value = ExecutionVisitor.visit(e.expr, ctx)
    ExecutionVisitor.output(value)
    return ExecValue.UNIT()

def visit_ReturnStmt(r: ReturnStmt, ctx: ExecContext) -> ExecValue:
    value = ExecutionVisitor.visit(r.expr, ctx)
    ctx.ret_stack.append(value)
    return value

def visit_FuncCallExpr(fc: FuncCallExpr, ctx: ExecContext) -> ExecValue:
    func_def = ctx.get(fc.name, fc.position, True).get()
    
    # Evaluate arguments and add them to the sub-context
    params = list(cast(EdeFunc, func_def.ede_type).get_args().keys())
    sub_ctx = ExecContext(ctx)

    for key, expr in dict(zip(params, fc.args)).items():
        value = ExecutionVisitor.visit(expr, sub_ctx)
        sub_ctx.add(key, ExecEntry(CtxEntryType.VARIABLE, expr.get_ede_type(), value, expr.position), False)

    # Evaluate the function body (this pushes the return value onto the return stack)
    ExecutionVisitor.visit(func_def.value.to_func_ptr(), sub_ctx)

    return ctx.ret_stack.pop()

VISITORS : Dict[Type[Any], Callable[[Any, ExecContext], ExecValue]] = {
    ExprStmt: visit_ExprStmt,
    IdentifierExpr: lambda i, ctx: ctx.get(cast(IdentifierExpr, i).id, cast(IdentifierExpr, i).position, True).get().value,
    ReturnStmt: visit_ReturnStmt,
    Module: visit_Module,
    ArrayInitExpr: visit_ArrayInitExpr,
    TupleInitExpr: visit_TupleInitExpr,
    ObjInitExpr: visit_ObjInitExpr,
    ObjDef: visit_ObjDef,
    FuncDef: visit_FuncDef,
    BinopExpr: visit_BinopExpr,
    IfElseStmt: visit_IfElseStmt,
    UnitLiteral: visit_Literal,
    IntLiteral: visit_Literal,
    CharLiteral: visit_Literal,
    StringLiteral: visit_Literal,
    BoolLiteral: visit_Literal,
    VarDeclStmt: visit_VarDeclStmt,
    Block: visit_Block,
    DefaultExpr: visit_DefaultExpr,
    FuncCallExpr: visit_FuncCallExpr,
}