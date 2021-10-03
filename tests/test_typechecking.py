from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_binop import BinopExpr, BinopType
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_ast.ede_typesystem import EdeBool, EdeChar, EdeInt, EdeString, Environment
from ede_utils import Position, char


print(CharLiteral(Position(1, 1), char('c')))


def test_literals():
    env = Environment()

    assert IntLiteral(Position(1, 1), 10).typecheck(env).get() == EdeInt
    assert BoolLiteral(Position(1, 1), True).typecheck(env).get() == EdeBool
    assert CharLiteral(Position(1, 1), char('c')).typecheck(env).get() == EdeChar
    assert StringLiteral(Position(1, 1), 'string').typecheck(env).get() == EdeString

def test_binop():
    env = Environment()
    
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD).typecheck(env).get() == EdeInt
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB).typecheck(env).get() == EdeInt
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL).typecheck(env).get() == EdeInt
    assert BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV).typecheck(env).get() == EdeInt
    assert BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD).typecheck(env).get() == EdeString

def test_identifier():
    env = Environment()
    
    env.declare('name', EdeChar, Position())
    assert IdentifierExpr(Position(1, 1), 'name').typecheck(env).get() == EdeChar