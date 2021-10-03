from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_ast.ede_typesystem import ArrayTypeSymbol, RecordTypeSymbol, TupleTypeSymbol
from ede_parser import TokenReader, parse_atom, parse_type_symbol
from ede_lexer import Reader, tokenize
from ede_utils import ErrorType

def get_token_reader(text: str) -> TokenReader:
    return TokenReader(tokenize(Reader(text)).get())

def test_atom():
    assert isinstance(parse_atom(get_token_reader('5')).get(), IntLiteral)
    assert isinstance(parse_atom(get_token_reader('"Hello World"')).get(), StringLiteral)
    assert isinstance(parse_atom(get_token_reader('\'c\'')).get(), CharLiteral)
    assert isinstance(parse_atom(get_token_reader('true')).get(), BoolLiteral)
    assert isinstance(parse_atom(get_token_reader('false')).get(), BoolLiteral)
    assert isinstance(parse_atom(get_token_reader('name')).get(), IdentifierExpr)

def test_expr():
    # TODO
    pass

def test_decls():
    # TODO
    pass

def test_type_symbols():
    assert parse_type_symbol(get_token_reader('int')).get() == 'int'
    assert parse_type_symbol(get_token_reader('[int]')).get() == ArrayTypeSymbol('int')
    assert parse_type_symbol(get_token_reader('(int, char)')).get() == TupleTypeSymbol(['int', 'char'])
    assert parse_type_symbol(get_token_reader('{name: string, age: int, children: [string]}')).get() == RecordTypeSymbol({'name': 'string', 'age': 'int', 'children': ArrayTypeSymbol('string')})
    assert parse_type_symbol(get_token_reader('[]')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('[int}')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('(int)')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('()')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('(int,)')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('(int, char}')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('{}')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('{name}')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('{name:}')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('{name:string')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('{name:string,')).is_error(ErrorType.PARSING_UNEXPECTED_TOKEN)
    assert parse_type_symbol(get_token_reader('{name:string, age:int, name:int}')).is_error(ErrorType.PARSING_DUP_RECORD_ITEM_NAME)

