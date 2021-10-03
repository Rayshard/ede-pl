from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_typesystem import EdeChar, EnvEntry, EnvEntryType, Environment
from interpreter import ExecContext, ExecException, ExecValue
from ede_ast.ede_binop import BinopExpr, BinopType
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_utils import Position, char

def test_literals():
    env = Environment()
    ctx = ExecContext()
    
    assert IntLiteral(Position(1, 1), 10).execute_in(env, ctx).get() == 10
    assert BoolLiteral(Position(1, 1), True).execute_in(env, ctx).get() == True
    assert CharLiteral(Position(1, 1), char('c')).execute_in(env, ctx).get() == char('c')
    assert StringLiteral(Position(1, 1), "string").execute_in(env, ctx).get() == "string"

def test_binop():
    env = Environment()
    ctx = ExecContext()
    
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD).execute_in(env, ctx).get() == 12
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB).execute_in(env, ctx).get() == -2
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL).execute_in(env, ctx).get() == 35
    assert BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV).execute_in(env, ctx).get() == 2
    assert BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 0), BinopType.DIV).execute_in(env, ctx).get() == ExecException.DivisionByZero(Position())
    assert BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD).execute_in(env, ctx).get() == "hiya"

def test_identifier():
    env = Environment()
    ctx = ExecContext()
    
    env.declare('name', EnvEntry(EnvEntryType.VARIABLE, EdeChar, Position()), True)
    ctx.set('name', ExecValue(char('r')), Position())
    assert IdentifierExpr(Position(1, 1), 'name').execute_in(env, ctx).get() == char('r')
    
def test_declarations():
    # TODO
    pass