from src.ede_ast.ede_binop import BinopExpr, BinopType
from src.ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from src.ede_ast.ede_type import EdeType, Environment
from src.ede_utils import Position, char


print(CharLiteral(Position(1, 1), char('c')))


def test_literals():
    env = Environment()

    assert IntLiteral(Position(1, 1), 10).typecheck(env).get() == EdeType.INT
    assert BoolLiteral(Position(1, 1), True).typecheck(env).get() == EdeType.BOOL
    assert CharLiteral(Position(1, 1), char('c')).typecheck(env).get() == EdeType.CHAR
    assert StringLiteral(Position(1, 1), 'string').typecheck(env).get() == EdeType.STR

def test_binop():
    env = Environment()
    
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD).typecheck(env).get() == EdeType.INT
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB).typecheck(env).get() == EdeType.INT
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL).typecheck(env).get() == EdeType.INT
    assert BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV).typecheck(env).get() == EdeType.INT
    assert BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD).typecheck(env).get() == EdeType.STR
