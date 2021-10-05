from typing import Any, Callable, Dict, List, Optional, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_binop import BINOP_EDE_TYPE_DICT, BinopExpr, BinopType, TypeCheckError_InvalidBinop
from ede_ast.ede_context import CtxEntryType
from ede_ast.ede_definition import ObjDef
from ede_ast.ede_expr import ArrayExpr, IdentifierExpr, ObjInitExpr, TupleExpr
from ede_ast.ede_literal import LIT_EDE_TYPE_DICT, BoolLiteral, CharLiteral, IntLiteral, Literal, StringLiteral
from ede_ast.ede_module import Module
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, VarDeclStmt
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol
from ede_ast.ede_typesystem import EdeArray, EdeObject, EdeTuple, EdeType, EdeUnit, TCContext, TCCtxEntry, TSType, TypeCheckError
from ede_utils import Result, Success

TCResult = Result[EdeType]

class TypecheckVisitor:
    '''
    Type checks the node within the given environment and updates
    the node's stored ede type.
    '''

    @staticmethod
    def visit(node: Node, ctx: TCContext) -> TCResult:
        result = VISITORS[type(node)](node, ctx)
        
        if result.is_error():
            node.set_ede_type(None)
            return result
        else:
            node.set_ede_type(result.get())
            return Success(node.get_ede_type())

def visit_IdentifierExpr(expr: IdentifierExpr, ctx: TCContext) -> TCResult:
    entry = ctx.get(expr.id, expr.position, True)
    if entry.is_error():
        return entry.error()
    elif entry.get().type != CtxEntryType.VARIABLE:
        return TypeCheckError.UnknownVariable(expr.id, expr.position)

    return Success(entry.get().ede_type)

def visit_BinopExpr(expr: BinopExpr, ctx: TCContext) -> TCResult:
    # typecheck LHS; return if fails
    left_type = TypecheckVisitor.visit(expr.left, ctx)
    if left_type.is_error():
        return left_type

    # typecheck RHS; return if fails
    right_type = TypecheckVisitor.visit(expr.right, ctx)
    if right_type.is_error():
        return right_type

    # set pattern and associated type; fail if pattern is not defined above
    # TODO: when we implement user defined types and inheritance and eq operator overloading, we'll have to update this
    expr.type_pattern = (left_type.get(), right_type.get(), expr.op)
    
    if expr.op == BinopType.ASSIGN:
        return left_type if left_type.get() == right_type.get() else TypeCheckError.InvalidAssignment(left_type.get(), right_type.get(), expr.position)
    elif expr.type_pattern in BINOP_EDE_TYPE_DICT:
        return Success(BINOP_EDE_TYPE_DICT[expr.type_pattern])
    else:
        return TypeCheckError_InvalidBinop(expr.op, left_type.get(), right_type.get(), expr.position)

def visit_Literal(expr: Literal[Any], ctx: TCContext) -> TCResult:
    return Success(LIT_EDE_TYPE_DICT[expr.get_lit_type()])

def visit_VarDeclStmt(stmt: VarDeclStmt, ctx: TCContext) -> TCResult:
    ede_type: Optional[EdeType] = None

    if stmt.type_symbol is not None:
        ts_res = TypecheckVisitor.visit(stmt.type_symbol, ctx)
        if ts_res.is_error():
            return ts_res

        ede_type = ts_res.get()

    if stmt.expr is not None:
        expr_res = TypecheckVisitor.visit(stmt.expr, ctx)
        if expr_res.is_error():
            return expr_res

        if ede_type == None:
            ede_type = expr_res.get()
        elif expr_res.get() != ede_type:
            return TypeCheckError.InvalidAssignment(ede_type, expr_res.get(), stmt.position)

    decl_res = ctx.add(stmt.id, TCCtxEntry(CtxEntryType.VARIABLE, cast(EdeType, ede_type), stmt.position), False)
    if decl_res is not None:
        return decl_res

    return Success(EdeUnit)

def visit_NameTypeSymbol(n: NameTypeSymbol, ctx: TCContext) -> TCResult:
    get_res = ctx.get(n.name, n.position, True)
    if get_res.is_error():
        return get_res.error()
    elif get_res.get().type != CtxEntryType.TYPENAME:
        return TypeCheckError.UnresolvableTypeName(n.name, n.position)
        
    return Success(get_res.get().ede_type)

def visit_ArrayTypeSymbol(a: ArrayTypeSymbol, ctx: TCContext) -> TCResult:
    inner_res = TypecheckVisitor.visit(a.inner, ctx)
    if inner_res.is_error():
        return inner_res
        
    return Success(EdeArray(inner_res.get()))

def visit_TupleTypeSymbol(t: TupleTypeSymbol, ctx: TCContext) -> TCResult:
    inners : List[EdeType] = []

    for inner in t.inners:
        inner_res = TypecheckVisitor.visit(inner, ctx)
        if inner_res.is_error():
            return inner_res
    
        inners.append(inner_res.get())
        
    return Success(EdeTuple(inners))

def visit_Block(b: Block, ctx: TCContext) -> TCResult:
    sub_env = TCContext(ctx)
    last_tc_res: Optional[TCResult] = None

    for stmt in b.stmts:
        last_tc_res = TypecheckVisitor.visit(stmt, sub_env)
        if last_tc_res.is_error():
            return last_tc_res

    return cast(TCResult, last_tc_res) if last_tc_res is not None else Success(EdeUnit)

def visit_Module(m: Module, ctx: TCContext) -> TCResult:
    sub_env = TCContext(ctx)

    for definition in m.defs:
        tc_res = TypecheckVisitor.visit(definition, sub_env)
        if tc_res.is_error():
            return tc_res

    for stmt in m.stmts:
        last_tc_res = TypecheckVisitor.visit(stmt, sub_env)
        if last_tc_res.is_error():
            return last_tc_res

    return Success(EdeUnit)

def visit_IfElseStmt(stmt: IfElseStmt, ctx: TCContext) -> TCResult:
    cond_res = TypecheckVisitor.visit(stmt.condition, ctx)
    if cond_res.is_error():
        return cond_res

    then_res = TypecheckVisitor.visit(stmt.thenClause, ctx)
    if then_res.is_error():
        return then_res

    if stmt.elseClause is not None:
        else_res = TypecheckVisitor.visit(stmt.elseClause, ctx)
        if else_res.is_error():
            return else_res
        elif else_res.get() != then_res.get():
            return TypeCheckError.IncompatibleIfElseClause(then_res.get(), else_res.get(), stmt.elseClause.position)

    return then_res

def visit_ArrayExpr(a: ArrayExpr, ctx: TCContext) -> TCResult:
    if len(a.exprs) == 0:
        # TODO
        raise Exception('Not handled')
    else:
        last_type: Optional[EdeType] = None

        for expr in a.exprs:
            type_res = TypecheckVisitor.visit(expr, ctx)
            if type_res.is_error():
                return type_res

            if last_type is None:
                last_type = type_res.get()
            elif type_res.get() != last_type:
                return TypeCheckError.UnexpectedType(type_res.get(), last_type, expr.position)

        return Success(EdeArray(cast(EdeType, last_type)))

def visit_TupleExpr(t: TupleExpr, ctx: TCContext) -> TCResult:
    inner_types: List[EdeType] = []

    for expr in t.exprs:
        inner_type_res = TypecheckVisitor.visit(expr, ctx)
        if inner_type_res.is_error():
            return inner_type_res

        inner_types.append(inner_type_res.get())

    return Success(EdeTuple(inner_types))

def visit_ObjInitExpr(oi: ObjInitExpr, ctx: TCContext) -> TCResult:
    # Check that the object is defined
    name_tc_res = visit_NameTypeSymbol(NameTypeSymbol(oi.name, oi.position), ctx)
    if name_tc_res.is_error():
        return name_tc_res
    elif name_tc_res.get().get_ts_type() != TSType.OBJECT:
        return TypeCheckError.UndefinedObject(oi.name, oi.position)

    obj_type = cast(EdeObject, name_tc_res.get())
    expected_members = obj_type.get_members()
    inited_members = []

    for id, value in oi.items.items():
        if id.value in inited_members:
            return TypeCheckError.Reinitialization(id.value, id.position)
        elif id.value not in expected_members:
            return TypeCheckError.UnexpectedInitialization(id.value, id.position)

        expected_type = expected_members[id.value]
        value_type_res = TypecheckVisitor.visit(value, ctx)
        if value_type_res.is_error():
            return value_type_res
        elif value_type_res.get() != expected_type:
            return TypeCheckError.UnexpectedType(value_type_res.get(), expected_type, value.position)

        inited_members.append(id)

    return name_tc_res

def visit_ObjDef(o: ObjDef, ctx: TCContext) -> TCResult:
    # Check that the object is not already defined
    name_get_res = ctx.get(o.name, o.position, True)
    if name_get_res.is_success():
        return TypeCheckError.IDConflict(o.name, name_get_res.get().pos, o.position)
         
    members : Dict[str, EdeType] = {}

    for id, type_symbol in o.members.items():
        if id.value in members:
            return TypeCheckError.Redefinition(id.value, id.position)
        
        mem_type_res = TypecheckVisitor.visit(type_symbol, ctx)
        if mem_type_res.is_error():
            return mem_type_res
        
        members[id.value] = mem_type_res.get()

    obj_type = EdeObject(o.name, members)

    ctx.add(o.name, TCCtxEntry(CtxEntryType.TYPENAME, obj_type, o.position), False)
    return Success(obj_type)

VISITORS : Dict[Type[Any], Callable[[Any, TCContext], TCResult]] = {
    ExprStmt: lambda node, ctx: TypecheckVisitor.visit(cast(ExprStmt, node).expr, ctx),
    IdentifierExpr: visit_IdentifierExpr,
    ObjDef: visit_ObjDef,
    BinopExpr: visit_BinopExpr,
    ArrayExpr: visit_ArrayExpr,
    TupleExpr: visit_TupleExpr,
    ObjInitExpr: visit_ObjInitExpr,
    IfElseStmt: visit_IfElseStmt,
    Module: visit_Module,
    IntLiteral: visit_Literal,
    CharLiteral: visit_Literal,
    StringLiteral: visit_Literal,
    BoolLiteral: visit_Literal,
    VarDeclStmt: visit_VarDeclStmt,
    PrimitiveTypeSymbol: lambda node, ctx: Success(cast(PrimitiveTypeSymbol, node).primitive),
    ArrayTypeSymbol: visit_ArrayTypeSymbol,
    TupleTypeSymbol: visit_TupleTypeSymbol,
    NameTypeSymbol: visit_NameTypeSymbol,
    Block: visit_Block
}