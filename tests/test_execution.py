from io import StringIO
import os
import sys

from ede.ede_ast.ede_context import CtxEntryType
from ede.ede_ast.ede_expr import IdentifierExpr, BinopExpr, BinopType
from ede.ede_ast.ede_typesystem import EdePrimitive, TCContext, TCCtxEntry
from ede.ede_ast.ede_visitors.ede_execution_visitor import ExecutionVisitor
from ede import ede_parser
from ede.interpreter import ExecContext, ExecEntry, ExecException, ExecValue
from ede.ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral, UnitLiteral
from ede.ede_utils import Position, char, unit

def test_literals():
    tc_ctx = TCContext()
    exec_ctx = ExecContext()
    
    assert ExecutionVisitor.visit_in(UnitLiteral(Position(1, 1)), tc_ctx, exec_ctx).get() == unit()
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
    
    tc_ctx.add('name', TCCtxEntry(CtxEntryType.VARIABLE, EdePrimitive.CHAR(), Position()), True)
    exec_ctx.add('name', ExecEntry(CtxEntryType.VARIABLE, EdePrimitive.CHAR(), ExecValue(char('r')), Position()), True)
    exec_ctx.set('name', ExecEntry(CtxEntryType.VARIABLE, EdePrimitive.CHAR(), ExecValue(char('r')), Position()))
    assert ExecutionVisitor.visit_in(IdentifierExpr(Position(1, 1), 'name'), tc_ctx, exec_ctx).get() == char('r')
    
def test_var_decl():
    # TODO
    pass

def test_block():
    # TODO
    pass

def test_default_expr():
    # TODO
    pass

def test_if_else():
    # TODO
    pass

def test_definitions():
    # TODO: object
    # TODO: enum
    # TODO: function
    pass

def test_init_exprs():
    # TODO: object init
    # TODO: array init
    # TODO: tuple init
    pass

def test_return():
    # TODO:
    pass

def test_cases():
    cases_dir = os.getcwd() + '\\tests\\cases'

    for folder in os.scandir(cases_dir):
        if not folder.is_dir():
            continue

        input_file_path = os.path.join(folder.path, 'input.ede')
        expected_file_path = os.path.join(folder.path, 'expected.txt')
        assert os.path.isfile(input_file_path) and os.path.isfile(expected_file_path)
        
        parse_result = ede_parser.parse_file(input_file_path)
        if parse_result.is_error():
            print(parse_result.error().get_output_msg(input_file_path))
            assert False

        tc_ctx = TCContext()
        exec_ctx = ExecContext()

        output_io = StringIO()
        ExecutionVisitor.Output_Stream = output_io
        exec_res = ExecutionVisitor.visit_in(parse_result.get(), tc_ctx, exec_ctx)
        ExecutionVisitor.Output_Stream = sys.stdout
        if exec_res.is_error():
            print(exec_res.error().get_output_msg(input_file_path))
            assert False

        with open(expected_file_path) as f_expected:
            assert output_io.getvalue().strip() == f_expected.read().strip()
        