from ede_utils import Position, char
from typing import Any, NamedTuple
from enum import IntEnum, auto

# TODO: Comment File

class TokenType(IntEnum):
    EOF = auto()
    INVALID = auto()
    INTEGER = auto()
    STRING = auto()
    CHAR = auto()
    IDENTIFIER = auto()
    SYM_PLUS = auto()
    SYM_DASH = auto()
    SYM_ASTERISK = auto()
    SYM_FWD_SLASH = auto()
    SYM_SEMICOLON = auto()
    SYM_EQUALS = auto()
    KW_LET = auto()
    KW_IF = auto()
    KW_ELSE = auto()
    KW_WHILE = auto()
    KW_TRUE = auto()
    KW_FALSE = auto()

SYMBOL_DICT = {
    '+': TokenType.SYM_PLUS,
    '-': TokenType.SYM_DASH,
    '*': TokenType.SYM_ASTERISK,
    '/': TokenType.SYM_FWD_SLASH,
    ';': TokenType.SYM_SEMICOLON,
    '=': TokenType.SYM_EQUALS,
}

KEYWORD_DICT = {
    "let": TokenType.KW_LET,
    "if": TokenType.KW_IF,
    "else": TokenType.KW_ELSE,
    "while": TokenType.KW_WHILE,
    "true": TokenType.KW_TRUE,
    "false": TokenType.KW_FALSE,
}

OPERATOR_DICT = {
    TokenType.SYM_PLUS: 0, TokenType.SYM_DASH: 0,
    TokenType.SYM_ASTERISK: 1, TokenType.SYM_FWD_SLASH: 1,
    TokenType.SYM_EQUALS: 2
}

def is_symbol(sym: str) -> bool:
    return sym in SYMBOL_DICT

def is_keyword(id: str) -> bool:
    return id in KEYWORD_DICT

class Token(NamedTuple):
    type: TokenType
    position: Position = Position(1, 1)
    value: Any = None

    @staticmethod
    def EOF(pos: Position):
        return Token(TokenType.EOF, pos)

    @staticmethod   
    def Invalid(pos: Position, value: str):
        return Token(TokenType.INVALID, pos, value)
    
    @staticmethod
    def Integer(pos: Position, value: int):
        return Token(TokenType.INTEGER, pos, value)

    @staticmethod
    def String(pos: Position, value: str):
        return Token(TokenType.STRING, pos, value)

    @staticmethod
    def Char(pos: Position, value: char):
        return Token(TokenType.CHAR, pos, value)

    @staticmethod
    def Symbol(pos: Position, sym: str):
        return Token(SYMBOL_DICT[sym], pos)

    @staticmethod
    def Keyword(pos: Position, id: str):
        return Token(KEYWORD_DICT[id], pos)

    @staticmethod
    def Identifier(pos: Position, id: str):
        return Token(TokenType.IDENTIFIER, pos, id)

    def __str__(self):
        string = f"{self.position.line, self.position.column}: {self.type.name}"
        return string + f"({repr(self.value)})" if self.value is not None else string