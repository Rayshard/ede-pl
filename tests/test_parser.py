from typing import cast
from ede_ast.ede_ast import Node
from ede_ast.ede_expr import DefaultExpr, IdentifierExpr, ObjInitExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, VarDeclStmt
from ede_ast.ede_type_symbol import NameTypeSymbol, ArrayTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol
from ede_ast.ede_typesystem import EdePrimitive
from ede_ast.ede_visitors.ede_json_visitor import JsonVisitor
from ede_parser import TokenReader, parse_expr, parse_stmt, parse_type_symbol
from ede_lexer import Reader, tokenize
from ede_utils import ErrorType, Position, Positioned, Result, Success, char

def get_token_reader(text: str) -> TokenReader:
    return TokenReader(tokenize(Reader(text)).get())

def check(value: str | Result[Node], expected: Node | ErrorType) -> bool:
    res = parse_stmt(get_token_reader(value)) if isinstance(value, str) else cast(Result[Node], value)
    print(res)
    if res.is_success():
        assert isinstance(expected, Node)
        print(JsonVisitor.visit(expected))
        print(JsonVisitor.visit(res.get()))
        return JsonVisitor.visit(res.get()) == JsonVisitor.visit(expected)

    assert isinstance(expected, ErrorType)
    return res.is_error(expected)

def test_expr():
    def check_expr(text: str, expected: Node | ErrorType) -> bool:
        res = parse_expr(get_token_reader(text))
        return check(Success(res.get()), expected) if res.is_success() else check(res.error(), expected)

    assert check_expr('5', IntLiteral(Position(), 5))
    assert check_expr('"Hello World"', StringLiteral(Position(), 'Hello World'))
    assert check_expr("'c'", CharLiteral(Position(), char('c')))
    assert check_expr('true', BoolLiteral(Position(), True))
    assert check_expr('false', BoolLiteral(Position(), False))
    assert check_expr('name', IdentifierExpr(Position(), 'name'))
    assert check_expr('default(int)', DefaultExpr(PrimitiveTypeSymbol(EdePrimitive.INT(), Position())))
    assert check_expr('Person { name = "Ray", age = 22 }', ObjInitExpr("Person", {
        Positioned[str]('name', Position()): StringLiteral(Position(), "Ray"),
        Positioned[str]('age', Position()): IntLiteral(Position(), 22)
    }, Position()))
    assert check_expr('default(5)', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_expr('Person { name = "Ray" age = 22 }', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_expr('Person { name = "Ray", "age" = 22 }', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_expr('Person { name = "Ray", age = 22 ', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_expr('Person { name: "Ray", age = 22 }', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_expr('Person { name = "Ray", name = 22 }', ErrorType.PARSING_DUP_MEMBER_NAME)

    # TODO: Tuple
    # TODO: Array
    # TODO: Binops (Each operator in a long long expression)

def test_decls():
    assert check('let a : int = 10;', VarDeclStmt(Position(), 'a', PrimitiveTypeSymbol(EdePrimitive.INT(), Position()), IntLiteral(Position(), 10)))
    assert check('let a = 10;', VarDeclStmt(Position(), 'a', None, IntLiteral(Position(), 10)))
    assert check('let a : int;', VarDeclStmt(Position(), 'a', PrimitiveTypeSymbol(EdePrimitive.INT(), Position()), None))
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
    def check_ts(text: str, expected: Node | ErrorType) -> bool:
        res = parse_type_symbol(get_token_reader(text))
        return check(Success(res.get()), expected) if res.is_success() else check(res.error(), expected)

    assert check_ts('unit', PrimitiveTypeSymbol(EdePrimitive.UNIT(), Position()))
    assert check_ts('int', PrimitiveTypeSymbol(EdePrimitive.INT(), Position()))
    assert check_ts('string', PrimitiveTypeSymbol(EdePrimitive.STRING(), Position()))
    assert check_ts('char', PrimitiveTypeSymbol(EdePrimitive.CHAR(), Position()))
    assert check_ts('bool', PrimitiveTypeSymbol(EdePrimitive.BOOL(), Position()))
    assert check_ts('MyType', NameTypeSymbol('MyType', Position()))
    assert check_ts('[int]', ArrayTypeSymbol(PrimitiveTypeSymbol(EdePrimitive.INT(), Position()), Position()))
    assert check_ts('[[int]]', ArrayTypeSymbol(ArrayTypeSymbol(PrimitiveTypeSymbol(EdePrimitive.INT(), Position()), Position()), Position()))
    assert check_ts('(int, char)', TupleTypeSymbol([PrimitiveTypeSymbol(EdePrimitive.INT(), Position()), PrimitiveTypeSymbol(EdePrimitive.CHAR(), Position())], Position()))
    assert check_ts('[]', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('[int}', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('(int)', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('()', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('(int,)', ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert check_ts('(int, char}', ErrorType.PARSING_UNEXPECTED_TOKEN)

def test_definition():
    # TODO: object
    # TODO: enum
    # TODO: function
    pass