from typing import Any, Callable, Dict, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_binop import BinopExpr
from ede_ast.ede_expr import ArrayExpr, Expression, IdentifierExpr, ObjInitExpr, TupleExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, Literal, StringLiteral
from ede_ast.ede_module import Module
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, Statement, VarDeclStmt
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol, TypeSymbol
from ede_ast.ede_typesystem import EdeArray, EdeObject, EdePrimitive, EdeTuple, EdeType

JSON = Dict[str, Any]

class JsonVisitor:
    @staticmethod
    def visit(obj: Any) -> JSON:
        result = VISITORS[type(obj)](obj)
        
        if isinstance(obj, Statement):
            result['stmt_type'] = obj.get_stmt_type().name
        if isinstance(obj, Expression):
            result['expr_type'] = obj.get_expr_type().name
        if isinstance(obj, Node):
            result['node_type'] = obj.get_node_type().name
        if isinstance(obj, EdeType):
            result['ts_type'] = obj.get_ts_type().name

        return result 

def visit_BinopExpr(expr: BinopExpr) -> JSON:
    return {
        "left": JsonVisitor.visit(expr.left),
        "right": JsonVisitor.visit(expr.right),
        "op": expr.op.name,
    }

def visit_Literal(expr: Literal[Any]) -> JSON:
    return {
        "type": expr.get_lit_type().name,
        "value": expr.value
    }

def visit_VarDeclStmt(stmt: VarDeclStmt) -> JSON:
    return {
        "id": stmt.id,
        "type_symbol": str(stmt.type_symbol) if stmt.type_symbol is not None else None,
        "expr": JsonVisitor.visit(stmt.expr) if stmt.expr is not None else None,
    }

def visit_TypeSymbol(ts: TypeSymbol) -> JSON:
    return {"repr": str(ts)}

def visit_Module(m: Module) -> JSON:
    return {
        "name": m.name,
        "statments": [JsonVisitor.visit(stmt) for stmt in m.stmts]
    }

def visit_IfElseStmt(stmt: IfElseStmt) -> JSON:
    return {
        'condition': JsonVisitor.visit(stmt.condition),
        'then': JsonVisitor.visit(stmt.thenClause),
        'else': JsonVisitor.visit(stmt.elseClause) if stmt.elseClause is not None else None,
    }

def visit_ObjInitExpr(oi: ObjInitExpr) -> JSON:
    return {
        "name": oi.name,
        "items": {id.value: JsonVisitor.visit(expr) for id, expr in oi.items.items()}
    }

VISITORS : Dict[Type[Any], Callable[[Any], JSON]]= {
    ExprStmt: lambda node: JsonVisitor.visit(cast(ExprStmt, node).expr),
    IdentifierExpr: lambda i: {"id": cast(IdentifierExpr, i).id},
    Module: visit_Module,
    ArrayExpr: lambda a: {"elements": [JsonVisitor.visit(expr) for expr in cast(ArrayExpr, a).exprs]},
    TupleExpr: lambda t: {"elements": [JsonVisitor.visit(expr) for expr in cast(TupleExpr, t).exprs]},
    ObjInitExpr: visit_ObjInitExpr,
    IfElseStmt: visit_IfElseStmt,
    BinopExpr: visit_BinopExpr,
    IntLiteral: visit_Literal,
    CharLiteral: visit_Literal,
    StringLiteral: visit_Literal,
    BoolLiteral: visit_Literal,
    VarDeclStmt: visit_VarDeclStmt,
    PrimitiveTypeSymbol: visit_TypeSymbol,
    ArrayTypeSymbol: visit_TypeSymbol,
    TupleTypeSymbol: visit_TypeSymbol,
    NameTypeSymbol: visit_TypeSymbol,
    Block: lambda b: {'statements': [JsonVisitor.visit(stmt) for stmt in cast(Block, b).stmts]},
    EdePrimitive: lambda p: {"type": cast(EdePrimitive, p).get_type().name},
    EdeArray: lambda a: {'inner_type': JsonVisitor.visit(cast(EdeArray, a).get_inner_type())},
    EdeTuple: lambda t: {'inner_types': [JsonVisitor.visit(type) for type in cast(EdeTuple, t).get_inner_types()]},
    EdeObject: lambda o: {'members': {id: JsonVisitor.visit(type) for id, type in cast(EdeObject, o).get_members().items()}},
}