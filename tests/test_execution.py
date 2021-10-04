from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_typesystem import EdeChar, EnvEntry, EnvEntryType, Environment
from ede_ast.ede_visitors.ede_execution_visitor import ExecutionVisitor
from interpreter import ExecContext, ExecException, ExecValue
from ede_ast.ede_binop import BinopExpr, BinopType
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_utils import Position, char

def test_literals():
    env = Environment()
    ctx = ExecContext()
    
    assert ExecutionVisitor.visit_in(IntLiteral(Position(1, 1), 10), env, ctx).get() == 10
    assert ExecutionVisitor.visit_in(BoolLiteral(Position(1, 1), True), env, ctx).get() == True
    assert ExecutionVisitor.visit_in(CharLiteral(Position(1, 1), char('c')), env, ctx).get() == char('c')
    assert ExecutionVisitor.visit_in(StringLiteral(Position(1, 1), "string"), env, ctx).get() == "string"

def test_binop():
    env = Environment()
    ctx = ExecContext()
    
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD), env, ctx).get() == 12
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB), env, ctx).get() == -2
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL), env, ctx).get() == 35
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV), env, ctx).get() == 2
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 0), BinopType.DIV), env, ctx).get() == ExecException.DivisionByZero(Position())
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD), env, ctx).get() == "hiya"

def test_identifier():
    env = Environment()
    ctx = ExecContext()
    
    env.declare('name', EnvEntry(EnvEntryType.VARIABLE, EdeChar, Position()), True)
    ctx.set('name', ExecValue(char('r')), Position())
    assert ExecutionVisitor.visit_in(IdentifierExpr(Position(1, 1), 'name'), env, ctx).get() == char('r')
    
def test_declarations():
    # TODO
    pass