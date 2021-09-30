from src.ede_ast.ede_ast import ExecContext, ExecException, TypedExecValue
from src.ede_ast.ede_binop import BinopExpr, BinopType
from src.ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from src.ede_ast.ede_type import EdeType, Environment
from src.ede_utils import Position

def test_literals():
    env = Environment()
    ctx = ExecContext()
    
    assert IntLiteral(Position(1, 1), 10).execute_in(env, ctx) == TypedExecValue(EdeType.INT, 10)
    assert BoolLiteral(Position(1, 1), True).execute_in(env, ctx) == TypedExecValue(EdeType.BOOL, True)
    assert CharLiteral(Position(1, 1), 'c').execute_in(env, ctx) == TypedExecValue(EdeType.CHAR, 'c')
    assert StringLiteral(Position(1, 1), "string").execute_in(env, ctx) == TypedExecValue(EdeType.STR, "string")

def test_binop():
    env = Environment()
    ctx = ExecContext()
    
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD).execute_in(env, ctx) == TypedExecValue(EdeType.INT, 12)
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB).execute_in(env, ctx) == TypedExecValue(EdeType.INT, -2)
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL).execute_in(env, ctx) == TypedExecValue(EdeType.INT, 35)
    assert BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV).execute_in(env, ctx) == TypedExecValue(EdeType.INT, 2)
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 0), BinopType.DIV).execute_in(env, ctx) == ExecException.DivisionByZero(Position())
    assert BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD).execute_in(env, ctx) == TypedExecValue(EdeType.STR, "hiya")
    