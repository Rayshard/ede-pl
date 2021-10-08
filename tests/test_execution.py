from ede_ast.ede_context import CtxEntryType
from ede_ast.ede_expr import IdentifierExpr, BinopExpr, BinopType
from ede_ast.ede_typesystem import EdeChar, TCContext, TCCtxEntry
from ede_ast.ede_visitors.ede_execution_visitor import ExecutionVisitor
from interpreter import ExecContext, ExecEntry, ExecException, ExecValue
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_utils import Position, char

def test_literals():
    tc_ctx = TCContext()
    exec_ctx = ExecContext()
    
    assert ExecutionVisitor.visit_in(IntLiteral(Position(1, 1), 10), tc_ctx, exec_ctx).get() == 10
    assert ExecutionVisitor.visit_in(BoolLiteral(Position(1, 1), True), tc_ctx, exec_ctx).get() == True
    assert ExecutionVisitor.visit_in(CharLiteral(Position(1, 1), char('c')), tc_ctx, exec_ctx).get() == char('c')
    assert ExecutionVisitor.visit_in(StringLiteral(Position(1, 1), "string"), tc_ctx, exec_ctx).get() == "string"

def test_binop():
    tc_ctx = TCContext()
    exec_ctx = ExecContext()
    
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.ADD), tc_ctx, exec_ctx).get() == 12
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.SUB), tc_ctx, exec_ctx).get() == -2
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 7), BinopType.MUL), tc_ctx, exec_ctx).get() == 35
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 10), IntLiteral(Position(), 5), BinopType.DIV), tc_ctx, exec_ctx).get() == 2
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), IntLiteral(Position(), 5), IntLiteral(Position(), 0), BinopType.DIV), tc_ctx, exec_ctx).get() == ExecException.DivisionByZero(Position())
    assert ExecutionVisitor.visit_in(BinopExpr(Position(), StringLiteral(Position(), "hi"), StringLiteral(Position(), "ya"), BinopType.ADD), tc_ctx, exec_ctx).get() == "hiya"

def test_identifier():
    tc_ctx = TCContext()
    exec_ctx = ExecContext()
    
    tc_ctx.add('name', TCCtxEntry(CtxEntryType.VARIABLE, EdeChar(), Position()), True)
    exec_ctx.add('name', ExecEntry(CtxEntryType.VARIABLE, EdeChar(), ExecValue(char('r')), Position()), True)
    exec_ctx.set('name', ExecEntry(CtxEntryType.VARIABLE, EdeChar(), ExecValue(char('r')), Position()))
    assert ExecutionVisitor.visit_in(IdentifierExpr(Position(1, 1), 'name'), tc_ctx, exec_ctx).get() == char('r')
    
def test_declarations():
    # TODO
    pass