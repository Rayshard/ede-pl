from typing import Any, NamedTuple
from enum import Enum, auto


class Position(NamedTuple):
    line: int
    column: int

class TokenType(Enum):
    EOF = auto()
    INVALID = auto()
    INTEGER = auto()
    STRING = auto()
    SYM_PLUS = auto()
    SYM_DASH = auto()
    SYM_ASTERISK = auto()
    SYM_FWD_SLASH = auto()
    SYM_SEMICOLON = auto()

SYMBOL_DICT = {
    '+': TokenType.SYM_PLUS,
    '-': TokenType.SYM_DASH,
    '*': TokenType.SYM_ASTERISK,
    '/': TokenType.SYM_FWD_SLASH,
    ';': TokenType.SYM_SEMICOLON,
}

def is_symbol(sym: str) -> bool:
    return sym in SYMBOL_DICT

class Token(NamedTuple):
    type: TokenType
    position: Position = Position(1, 1)
    value: Any = None

    def EOF(pos: Position):
        return Token(TokenType.EOF, pos)

    def Invalid(pos: Position, value: str):
        return Token(TokenType.INVALID, pos, value)

    def Integer(pos: Position, value: int):
        return Token(TokenType.INTEGER, pos, value)

    def String(pos: Position, value: str):
        return Token(TokenType.STRING, pos, value)

    def Symbol(pos: Position, sym: str):
        return Token(SYMBOL_DICT[sym], pos)

    def __str__(self):
        string = f"{self.position.line, self.position.column}: {self.type.name}"
        return string + f"({self.value})" if self.value is not None else string