from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_binop import BinopExpr, BinopType
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_ast.ede_typesystem import EdeBool, EdeChar, EdeInt, EdeString, EnvEntry, EnvEntryType, Environment
from ede_ast.ede_visitors.ede_typecheck_visitor import TypecheckVisitor
from ede_utils import Position, char


print(CharLiteral(Position(1, 1), char('c')))


def test_literals():
    env = Environment()

    assert TypecheckVisitor.visit(IntLiteral(Position(1, 1), 10), env).get() == EdeInt
    assert TypecheckVisitor.visit(BoolLiteral(Position(1, 1), True), env).get() == EdeBool
    assert TypecheckVisitor.visit(CharLiteral(Position(1, 1), char('c')), env).get() == EdeChar
    assert TypecheckVisitor.visit(StringLiteral(Position(1, 1), 'string'), env).get() == EdeString

def test_binop():
    env = Environment()
    
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD), env).get() == EdeInt
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB), env).get() == EdeInt
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL), env).get() == EdeInt
    assert TypecheckVisitor.visit(BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV), env).get() == EdeInt
    assert TypecheckVisitor.visit(BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD), env).get() == EdeString

def test_identifier():
    env = Environment()
    
    env.declare('name', EnvEntry(EnvEntryType.VARIABLE, EdeChar, Position()), True)
    assert TypecheckVisitor.visit(IdentifierExpr(Position(1, 1), 'name'), env).get() == EdeChar