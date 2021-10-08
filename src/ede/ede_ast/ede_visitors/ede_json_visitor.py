from typing import Any, Callable, Dict, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_context import CtxEntryType
from ede_ast.ede_definition import ObjDef
from ede_ast.ede_expr import ArrayExpr, Expression, IdentifierExpr, ObjInitExpr, TupleExpr, BinopExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, Literal, StringLiteral, UnitLiteral
from ede_ast.ede_module import Module
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, Statement, VarDeclStmt
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol, TypeSymbol
from ede_ast.ede_typesystem import EdeArray, EdeObject, EdePrimitive, EdeTuple
from interpreter import ExecContext

class JsonVisitor:
    @staticmethod
    def visit(obj: Any) -> Any:
        result = VISITORS[type(obj)](obj)
        
        if isinstance(obj, Statement):
            result['stmt_type'] = obj.get_stmt_type().name
        if isinstance(obj, Expression):
            result['expr_type'] = obj.get_expr_type().name
        if isinstance(obj, Node):
            result['node_type'] = obj.get_node_type().name

        return result 

def visit_BinopExpr(expr: BinopExpr) -> Any:
    return {
        "left": JsonVisitor.visit(expr.left),
        "right": JsonVisitor.visit(expr.right),
        "op": expr.op.name,
    }

def visit_Literal(expr: Literal[Any]) -> Any:
    return {
        "type": expr.get_lit_type().name,
        "value": expr.value
    }

def visit_VarDeclStmt(stmt: VarDeclStmt) -> Any:
    return {
        "id": stmt.id,
        "type_symbol": str(stmt.type_symbol) if stmt.type_symbol is not None else None,
        "expr": JsonVisitor.visit(stmt.expr) if stmt.expr is not None else None,
    }

def visit_TypeSymbol(ts: TypeSymbol) -> Any:
    return {"repr": str(ts)}

def visit_Module(m: Module) -> Any:
    return {
        "name": m.name,
        "definitions": [JsonVisitor.visit(definition) for definition in m.defs],
        "statments": [JsonVisitor.visit(stmt) for stmt in m.stmts]
    }

def visit_IfElseStmt(stmt: IfElseStmt) -> Any:
    return {
        'condition': JsonVisitor.visit(stmt.condition),
        'then': JsonVisitor.visit(stmt.thenClause),
        'else': JsonVisitor.visit(stmt.elseClause) if stmt.elseClause is not None else None,
    }

def visit_ObjInitExpr(oi: ObjInitExpr) -> Any:
    return {
        "name": oi.name,
        "items": {id.value: JsonVisitor.visit(expr) for id, expr in oi.items.items()}
    }

def visit_ObjDef(o: ObjDef) -> Any:
    return {
        "name": o.name,
        "members": {id.value: JsonVisitor.visit(type_symbol) for id, type_symbol in o.members.items()}
    }

def visit_ExecContext(ec: ExecContext) -> Any:
    return {
        "parent": None if ec.parent is None else visit_ExecContext(cast(ExecContext, ec.parent)),
        "variables": [f"{id} : {JsonVisitor.visit(entry.ede_type)} = {entry.value} " for id, entry in ec.get_entries(CtxEntryType.VARIABLE).items()],
        "typenames": [JsonVisitor.visit(entry.ede_type) for _, entry in ec.get_entries(CtxEntryType.TYPENAME).items()],
        "functions": []
    }

VISITORS : Dict[Type[Any], Callable[[Any], Any]]= {
    ExprStmt: lambda node: JsonVisitor.visit(cast(ExprStmt, node).expr),
    IdentifierExpr: lambda i: {"id": cast(IdentifierExpr, i).id},
    Module: visit_Module,
    ExecContext: visit_ExecContext,
    ArrayExpr: lambda a: {"elements": [JsonVisitor.visit(expr) for expr in cast(ArrayExpr, a).exprs]},
    TupleExpr: lambda t: {"elements": [JsonVisitor.visit(expr) for expr in cast(TupleExpr, t).exprs]},
    ObjInitExpr: visit_ObjInitExpr,
    ObjDef: visit_ObjDef,
    IfElseStmt: visit_IfElseStmt,
    BinopExpr: visit_BinopExpr,
    UnitLiteral: visit_Literal,
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
    EdePrimitive: lambda p: str(p),
    EdeArray: lambda a: str(a),
    EdeTuple: lambda t: str(t),
    EdeObject: lambda o: str(o),
}