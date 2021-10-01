from ede_ast.ede_expr import IdentifierExpr
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral
from ede_parser import TokenReader, parse
from src.ede_lexer import Reader, tokenize

def get_token_reader(text: str) -> TokenReader:
    return TokenReader(tokenize(Reader(text)).get())

def test_atom():
    assert isinstance(parse(get_token_reader('5')).get(), IntLiteral)
    assert isinstance(parse(get_token_reader('"Hello World"')).get(), StringLiteral)
    assert isinstance(parse(get_token_reader('\'c\'')).get(), CharLiteral)
    assert isinstance(parse(get_token_reader('true')).get(), BoolLiteral)
    assert isinstance(parse(get_token_reader('false')).get(), BoolLiteral)
    assert isinstance(parse(get_token_reader('name')).get(), IdentifierExpr)

def test_expr():
    # TODO
    pass
