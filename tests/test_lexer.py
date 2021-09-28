from lexer import Reader, lex, lex_integer, lex_string
from ede_token import Token, TokenType


def test_integer():
    assert lex_integer(Reader("0")) == Token(type=TokenType.INTEGER, value=0)
    assert lex_integer(Reader("123")) == Token(type=TokenType.INTEGER, value=123)
    assert lex_integer(Reader("1a")) == Token(type=TokenType.INTEGER, value=1)
    assert lex_integer(Reader("a")) == Token(type=TokenType.INVALID, value="a")
    assert lex_integer(Reader("a1")) == Token(type=TokenType.INVALID, value="a")
    assert lex_integer(Reader(" ")) == Token(type=TokenType.INVALID, value=" ")
    assert lex_integer(Reader(" 12")) == Token(type=TokenType.INVALID, value=" ")

def test_string():
    assert lex(Reader('"a"')) == Token(type=TokenType.STRING, value="a")
    assert lex(Reader('"this is a long string"')) == Token(type=TokenType.STRING, value="this is a long string")
    assert lex(Reader('"new line\\nis here"')) == Token(type=TokenType.STRING, value="new line\nis here")
    assert lex(Reader('"new line\nis here"')) == Token(type=TokenType.STRING, value="new line\nis here")
    assert lex(Reader('"tab\\tis here"')) == Token(type=TokenType.STRING, value="tab\tis here")
    assert lex(Reader('"tab\tis here"')) == Token(type=TokenType.STRING, value="tab\tis here")
    assert lex(Reader('"backslash\\\\is here"')) == Token(type=TokenType.STRING, value="backslash\\is here")
    assert lex(Reader('"unknown escaped\\is here"')) == Token(type=TokenType.STRING, value="unknown escaped\\is here")
    assert lex(Reader('"improper close\\"')) == Token(type=TokenType.INVALID, value="improper close\"")
    assert lex(Reader('"improper close')) == Token(type=TokenType.INVALID, value="improper close")
    assert lex(Reader('"improper close\\')) == Token(type=TokenType.INVALID, value="improper close\\")