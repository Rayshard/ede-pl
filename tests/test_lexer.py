from ede_utils import ErrorType
from lexer import Reader, lex, lex_integer, lex_string
from ede_token import Token, TokenType


def test_integer():
    assert lex_integer(Reader("0")).get() == Token(type=TokenType.INTEGER, value=0)
    assert lex_integer(Reader("123")).get() == Token(type=TokenType.INTEGER, value=123)
    assert lex_integer(Reader("1a")).get() == Token(type=TokenType.INTEGER, value=1)
    assert lex_integer(Reader("a")).is_error(ErrorType.INVALID_INT_LIT)
    assert lex_integer(Reader("a1")).is_error(ErrorType.INVALID_INT_LIT)
    assert lex_integer(Reader(" ")).is_error(ErrorType.INVALID_INT_LIT)
    assert lex_integer(Reader(" 12")).is_error(ErrorType.INVALID_INT_LIT)

def test_string():
    assert lex_string(Reader('"a"')).get() == Token(type=TokenType.STRING, value="a")
    assert lex_string(Reader('"this is a long string"')).get() == Token(type=TokenType.STRING, value="this is a long string")
    assert lex_string(Reader('"new line\\nis here"')).get() == Token(type=TokenType.STRING, value="new line\nis here")
    assert lex_string(Reader('"new line\nis here"')).get() == Token(type=TokenType.STRING, value="new line\nis here")
    assert lex_string(Reader('"tab\\tis here"')).get() == Token(type=TokenType.STRING, value="tab\tis here")
    assert lex_string(Reader('"tab\tis here"')).get() == Token(type=TokenType.STRING, value="tab\tis here")
    assert lex_string(Reader('"backslash\\\\is here"')).get() == Token(type=TokenType.STRING, value="backslash\\is here")
    assert lex_string(Reader('"unknown escaped\\is here"')).get() == Token(type=TokenType.STRING, value="unknown escaped\\is here")
    assert lex_string(Reader('"improper close\\"')).is_error(ErrorType.UNEXPECTED_EOF)
    assert lex_string(Reader('"improper close')).is_error(ErrorType.UNEXPECTED_EOF)
    assert lex_string(Reader('"improper close\\')).is_error(ErrorType.UNEXPECTED_EOF)
    assert lex_string(Reader('improper open\\')).is_error(ErrorType.INVALID_STR_LIT)