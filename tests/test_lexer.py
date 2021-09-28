from src.lexer import Reader, lex, lex_char, lex_id_or_keyword, lex_integer, lex_string
from src.ede_utils import ErrorType
from src.ede_token import KEYWORD_DICT, SYMBOL_DICT, Token, TokenType


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
    assert lex_string(Reader('"null\\0is here"')).get() == Token(type=TokenType.STRING, value="null\0is here")
    assert lex_string(Reader('"backslash\\\\is here"')).get() == Token(type=TokenType.STRING, value="backslash\\is here")
    assert lex_string(Reader('"unknown escaped\\is here"')).get() == Token(type=TokenType.STRING, value="unknown escaped\\is here")
    assert lex_string(Reader('"improper close\\"')).is_error(ErrorType.UNEXPECTED_EOF)
    assert lex_string(Reader('"improper close')).is_error(ErrorType.UNEXPECTED_EOF)
    assert lex_string(Reader('"improper close\\')).is_error(ErrorType.UNEXPECTED_EOF)
    assert lex_string(Reader('improper open\\')).is_error(ErrorType.INVALID_STR_LIT)

def test_char():
    assert lex_char(Reader("'a'")).get() == Token(type=TokenType.CHAR, value='a')
    assert lex_char(Reader("'\\n'")).get() == Token(type=TokenType.CHAR, value='\n')
    assert lex_char(Reader("'\\t'")).get() == Token(type=TokenType.CHAR, value='\t')
    assert lex_char(Reader("'\\\\'")).get() == Token(type=TokenType.CHAR, value='\\')
    assert lex_char(Reader("'abc'")).is_error(ErrorType.INVALID_CHAR_LIT)
    assert lex_char(Reader("'\\a'")).is_error(ErrorType.INVALID_CHAR_LIT)
    assert lex_char(Reader("'i")).is_error(ErrorType.INVALID_CHAR_LIT)
    assert lex_char(Reader("''")).is_error(ErrorType.INVALID_CHAR_LIT)
    assert lex_char(Reader("'\\'")).is_error(ErrorType.INVALID_CHAR_LIT)
    assert lex_char(Reader("a'")).is_error(ErrorType.INVALID_CHAR_LIT)

def test_symbols():
    for sym, type in SYMBOL_DICT.items():
        assert lex(Reader(sym)).get().type == type

def test_keywords():
    for kw, type in KEYWORD_DICT.items():
        assert lex_id_or_keyword(Reader(kw)).get().type == type

def test_identifiers():
    assert lex_id_or_keyword(Reader("_")).get() == Token(type=TokenType.IDENTIFIER, value="_")
    assert lex_id_or_keyword(Reader("a")).get() == Token(type=TokenType.IDENTIFIER, value="a")
    assert lex_id_or_keyword(Reader("a1")).get() == Token(type=TokenType.IDENTIFIER, value="a1")
    assert lex_id_or_keyword(Reader("a_1")).get() == Token(type=TokenType.IDENTIFIER, value="a_1")
    assert lex_id_or_keyword(Reader("a_1_")).get() == Token(type=TokenType.IDENTIFIER, value="a_1_")
    assert lex_id_or_keyword(Reader("_a_1_")).get() == Token(type=TokenType.IDENTIFIER, value="_a_1_")
    assert lex_id_or_keyword(Reader("_123")).get() == Token(type=TokenType.IDENTIFIER, value="_123")
    assert lex_id_or_keyword(Reader("a bc")).get() == Token(type=TokenType.IDENTIFIER, value="a")
    assert lex_id_or_keyword(Reader("123")).is_error(ErrorType.INVALID_ID)
    assert lex_id_or_keyword(Reader(" abc")).is_error(ErrorType.INVALID_ID)