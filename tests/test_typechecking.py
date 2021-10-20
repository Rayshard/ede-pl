from ede.ede_ast.ede_context import CtxEntryType
from ede.ede_ast.ede_expr import IdentifierExpr, BinopExpr, BinopType
from ede.ede_ast.ede_literal import UnitLiteral, BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede.ede_ast.ede_stmt import ReturnStmt
from ede.ede_ast.ede_typesystem import EdePrimitive, TCContext, TCCtxEntry
from ede.ede_ast.ede_visitors.ede_typecheck_visitor import TypecheckVisitor
from ede.ede_utils import Position, char

def test_literals():
    ctx = TCContext()

    assert TypecheckVisitor.visit(UnitLiteral(Position(1, 1)), ctx).get() == EdePrimitive.UNIT()
    assert TypecheckVisitor.visit(IntLiteral(Position(1, 1), 10), ctx).get() == EdePrimitive.INT()
    assert TypecheckVisitor.visit(BoolLiteral(Position(1, 1), True), ctx).get()  == EdePrimitive.BOOL()
    assert TypecheckVisitor.visit(CharLiteral(Position(1, 1), char('c')), ctx).get() == EdePrimitive.CHAR()
    assert TypecheckVisitor.visit(StringLiteral(Position(1, 1), 'string'), ctx).get() == EdePrimitive.STRING()

def test_binop():
    ctx = TCContext()
    
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD), ctx).get() == EdePrimitive.INT()
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB), ctx).get() == EdePrimitive.INT()
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL), ctx).get() == EdePrimitive.INT()
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV), ctx).get() == EdePrimitive.INT()
    assert TypecheckVisitor.visit(BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD), ctx).get() == EdePrimitive.STRING()

def test_identifier():
    ctx = TCContext()
    
    ctx.add('name', TCCtxEntry(CtxEntryType.VARIABLE, EdePrimitive.CHAR(), Position()), True)
    assert TypecheckVisitor.visit(IdentifierExpr(Position(), 'name'), ctx).get() == EdePrimitive.CHAR()

def test_if_else():
    # TODO
    pass

def test_return():
    ctx = TCContext()
    ctx.ret_type = EdePrimitive.INT()
    
    assert TypecheckVisitor.visit(ReturnStmt(IntLiteral(Position(), 5), Position()), ctx).get() == EdePrimitive.UNIT()