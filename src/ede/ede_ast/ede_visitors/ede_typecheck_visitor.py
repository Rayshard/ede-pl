from typing import Any, Callable, Dict, List, Optional, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_binop import BINOP_EDE_TYPE_DICT, BinopExpr, BinopType, TypeCheckError_InvalidBinop
from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_literal import LIT_EDE_TYPE_DICT, BoolLiteral, CharLiteral, IntLiteral, Literal, StringLiteral
from ede_ast.ede_stmt import ExprStmt, VarDeclStmt
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, RecordTypeSymbol, TupleTypeSymbol
from ede_ast.ede_typesystem import EdeArray, EdeRecord, EdeTuple, EdeType, EdeUnit, EnvEntry, EnvEntryType, Environment, TypeCheckError
from ede_utils import Error, Result, Success

TCResult = Result[EdeType]

class TypecheckVisitor:
    '''
    Type checks the node within the given environment and updates
    the node's stored ede type.
    '''

    @staticmethod
    def visit(node: Node, env: Environment) -> TCResult:
        result = VISITORS[type(node)](node, env)
        
        if result.is_error():
            node.set_ede_type(None)
            return result
        else:
            node.set_ede_type(result.get())
            return Success(node.get_ede_type())

def visit_IdentifierExpr(expr: IdentifierExpr, env: Environment) -> TCResult:
    entry = env.get(expr.id, expr.position, True)
    if entry.is_error():
        return cast(Error, entry)
    elif entry.get().type != EnvEntryType.VARIABLE:
        return TypeCheckError.UnknownVariable(expr.id, expr.position)

    return Success(entry.get().ede_type)

def visit_BinopExpr(expr: BinopExpr, env: Environment) -> TCResult:
    # typecheck LHS; return if fails
    left_type = TypecheckVisitor.visit(expr.left, env)
    if left_type.is_error():
        return left_type

    # typecheck RHS; return if fails
    right_type = TypecheckVisitor.visit(expr.right, env)
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

def visit_Literal(expr: Literal[Any], env: Environment) -> TCResult:
    return Success(LIT_EDE_TYPE_DICT[expr.get_lit_type()])

def visit_VarDeclStmt(stmt: VarDeclStmt, env: Environment) -> TCResult:
    ede_type: Optional[EdeType] = None

    if stmt.type_symbol is not None:
        ts_res = TypecheckVisitor.visit(stmt.type_symbol, env)
        if ts_res.is_error():
            return ts_res

        ede_type = ts_res.get()

    if stmt.expr is not None:
        expr_res = TypecheckVisitor.visit(stmt.expr, env)
        if expr_res.is_error():
            return expr_res

        if ede_type == None:
            ede_type = expr_res.get()
        elif expr_res.get() != ede_type:
            return TypeCheckError.InvalidAssignment(ede_type, expr_res.get(), stmt.position)

    decl_res = env.declare(stmt.id, EnvEntry(EnvEntryType.VARIABLE, cast(EdeType, ede_type), stmt.position), False)
    if decl_res is not None:
        return decl_res

    return Success(EdeUnit)

def visit_NameTypeSymbol(n: NameTypeSymbol, env: Environment) -> TCResult:
    get_res = env.get(n.name, n.position, True)
    if get_res.is_error():
        return cast(Error, get_res)
    elif get_res.get().type != EnvEntryType.TYPENAME:
        return TypeCheckError.UnresolvableTypeName(n.name, n.position)
        
    return Success(get_res.get().ede_type)

def visit_ArrayTypeSymbol(a: ArrayTypeSymbol, env: Environment) -> TCResult:
    inner_res = TypecheckVisitor.visit(a.inner, env)
    if inner_res.is_error():
        return inner_res
        
    return Success(EdeArray(inner_res.get()))

def visit_TupleTypeSymbol(t: TupleTypeSymbol, env: Environment) -> TCResult:
    inners : List[EdeType] = []

    for inner in t.inners:
        inner_res = TypecheckVisitor.visit(inner, env)
        if inner_res.is_error():
            return inner_res
    
        inners.append(inner_res.get())
        
    return Success(EdeTuple(inners))

def visit_RecordTypeSymbol(r: RecordTypeSymbol, env: Environment) -> TCResult:
    items : Dict[str, EdeType] = {}

    for name, item in r.items.items():
        item_res = TypecheckVisitor.visit(item, env)
        if item_res.is_error():
            return item_res
    
        items[name] = item_res.get()
        
    return Success(EdeRecord(items))

VISITORS : Dict[Type[Any], Callable[[Any, Environment], TCResult]] = {
    ExprStmt: lambda node, env: TypecheckVisitor.visit(cast(ExprStmt, node).expr, env),
    IdentifierExpr: visit_IdentifierExpr,
    BinopExpr: visit_BinopExpr,
    IntLiteral: visit_Literal,
    CharLiteral: visit_Literal,
    StringLiteral: visit_Literal,
    BoolLiteral: visit_Literal,
    VarDeclStmt: visit_VarDeclStmt,
    PrimitiveTypeSymbol: lambda node, env: Success(cast(PrimitiveTypeSymbol, node).primitive),
    ArrayTypeSymbol: visit_ArrayTypeSymbol,
    TupleTypeSymbol: visit_TupleTypeSymbol,
    RecordTypeSymbol: visit_RecordTypeSymbol,
    NameTypeSymbol: visit_NameTypeSymbol
}