from typing import Union, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, VarDeclStmt
from ede_ast.ede_type_symbol import NameTypeSymbol, ArrayTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol
from ede_ast.ede_typesystem import EdeChar, EdeInt
from ede_ast.ede_visitors.ede_json_visitor import JsonVisitor
from ede_parser import TokenReader, parse_expr, parse_stmt, parse_type_symbol
from ede_lexer import Reader, tokenize
from ede_utils import ErrorType, Position, Result, Success, char

def get_token_reader(text: str) -> TokenReader:
    return TokenReader(tokenize(Reader(text)).get())

def check(value: Union[str, Result[Node]], expected: Union[Node, ErrorType]) -> bool:
    res = parse_stmt(get_token_reader(value)) if isinstance(value, str) else cast(Result[Node], value)

    if res.is_success():
        assert isinstance(expected, Node)
        print(JsonVisitor.visit(expected))
        print(JsonVisitor.visit(res.get()))
        return JsonVisitor.visit(res.get()) == JsonVisitor.visit(expected)

    assert isinstance(expected, ErrorType)
    return res.is_error(expected)

def test_expr():
    def check_expr(text: str, expected: Union[Node, ErrorType]) -> bool:
        res = parse_expr(get_token_reader(text))
        return check(Success(res.get()), expected) if res.is_success() else check(res.error(), expected)

    assert check_expr('5', IntLiteral(Position(), 5))
    assert check_expr('"Hello World"', StringLiteral(Position(), 'Hello World'))
    assert check_expr("'c'", CharLiteral(Position(), char('c')))
    assert check_expr('true', BoolLiteral(Position(), True))
    assert check_expr('false', BoolLiteral(Position(), False))
    assert check_expr('name', IdentifierExpr(Position(), 'name'))

def test_decls():
    assert check('let a : int = 10;', VarDeclStmt(Position(), 'a', PrimitiveTypeSymbol(EdeInt(), Position()), IntLiteral(Position(), 10)))
    assert check('let a = 10;', VarDeclStmt(Position(), 'a', None, IntLiteral(Position(), 10)))
    assert check('let a : int;', VarDeclStmt(Position(), 'a', PrimitiveTypeSymbol(EdeInt(), Position()), None))
    assert check('let', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check('let a', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check('let a : = 10', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check('let a : int =', ErrorType.PARSING_UNEXPECTED_TOKEN)

def test_block():
    assert check('{ }', Block([], Position()))
    assert check('{ let a = 5; let b = 10; }', Block([
        VarDeclStmt(Position(), 'a', None, IntLiteral(Position(), 5)),
        VarDeclStmt(Position(), 'b', None, IntLiteral(Position(), 10))
    ], Position()))
    assert check('{ let a = 5; let b = 10;', ErrorType.PARSING_UNEXPECTED_TOKEN)

def test_if_else():
    assert check('if true { 5; } else { 6; }', IfElseStmt(
        BoolLiteral(Position(), True),
        Block([ExprStmt(IntLiteral(Position(), 5))], Position()),
        Block([ExprStmt(IntLiteral(Position(), 6))], Position()),
        Position()))
    assert check('if true { 5; }', IfElseStmt(
        BoolLiteral(Position(), True),
        Block([ExprStmt(IntLiteral(Position(), 5))], Position()),
        None,
        Position()))
    assert check('if', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check('if true', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check('if true else', ErrorType.PARSING_UNEXPECTED_TOKEN)

def test_type_symbols():
    def check_ts(text: str, expected: Union[Node, ErrorType]) -> bool:
        res = parse_type_symbol(get_token_reader(text))
        return check(Success(res.get()), expected) if res.is_success() else check(res.error(), expected)

    assert check_ts('int', PrimitiveTypeSymbol(EdeInt(), Position()))
    assert check_ts('MyType', NameTypeSymbol('MyType', Position()))
    assert check_ts('[int]', ArrayTypeSymbol(PrimitiveTypeSymbol(EdeInt(), Position()), Position()))
    assert check_ts('(int, char)', TupleTypeSymbol([PrimitiveTypeSymbol(EdeInt(), Position()), PrimitiveTypeSymbol(EdeChar(), Position())], Position()))
    assert check_ts('[]', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('[int}', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('(int)', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('()', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('(int,)', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('(int, char}', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('{}', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('{name}', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('{name:}', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('{name:string', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('{name:string,', ErrorType.PARSING_UNEXPECTED_TOKEN)

