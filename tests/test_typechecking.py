from ede_ast.ede_context import CtxEntryType
from ede_ast.ede_expr import IdentifierExpr, BinopExpr, BinopType
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_ast.ede_typesystem import EdeBool, EdeChar, EdeInt, EdeString, TCContext, TCCtxEntry
from ede_ast.ede_visitors.ede_typecheck_visitor import TypecheckVisitor
from ede_utils import Position, char

def test_literals():
    ctx = TCContext()

    assert TypecheckVisitor.visit(IntLiteral(Position(1, 1), 10), ctx).get().is_type(EdeInt)
    assert TypecheckVisitor.visit(BoolLiteral(Position(1, 1), True), ctx).get().is_type(EdeBool)
    assert TypecheckVisitor.visit(CharLiteral(Position(1, 1), char('c')), ctx).get().is_type(EdeChar)
    assert TypecheckVisitor.visit(StringLiteral(Position(1, 1), 'string'), ctx).get().is_type(EdeString)

def test_binop():
    ctx = TCContext()
    
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD), ctx).get().is_type(EdeInt)
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB), ctx).get().is_type(EdeInt)
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL), ctx).get().is_type(EdeInt)
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV), ctx).get().is_type(EdeInt)
    assert TypecheckVisitor.visit(BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD), ctx).get().is_type(EdeString)

def test_identifier():
    ctx = TCContext()
    
    ctx.add('name', TCCtxEntry(CtxEntryType.VARIABLE, EdeChar(), Position()), True)
    assert TypecheckVisitor.visit(IdentifierExpr(Position(1, 1), 'name'), ctx).get().is_type(EdeChar)

def test_if_else():
    # TODO
    pass