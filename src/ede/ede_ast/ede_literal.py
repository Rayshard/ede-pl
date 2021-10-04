from abc import abstractmethod
from enum import Enum, auto
from ede_utils import Position, char, unit
from .ede_expr import Expression, ExprType
from .ede_typesystem import EdeBool, EdeChar, EdeInt, EdeString, EdeType, EdeUnit
from typing import Generic, TypeVar, cast

T = TypeVar('T', int, str, char, bool, unit)

class LiteralType(Enum):
    '''Enumeration of AST literal types'''

    UNIT = (auto(), EdeUnit)
    INTEGER = (auto(), EdeInt)
    BOOL = (auto(), EdeBool)
    CHAR = (auto(), EdeChar)
    STRING = (auto(), EdeString)

    def get_ede_type(self) -> EdeType:
        '''Returns the associated ede type'''
        return cast(EdeType, self.value[1])

LIT_EDE_TYPE_DICT = {item: item.get_ede_type() for item in LiteralType}  # Map between python types and ast literal types

class Literal(Expression, Generic[T]):
    '''AST Literal node'''

    def __init__(self, pos: Position, value: T) -> None:
        '''Creates an AST Literal node'''

        super().__init__(pos)
        self.value : T = value

    def get_expr_type(self) -> ExprType:
        return ExprType.LITERAL

    @abstractmethod
    def get_lit_type(self) -> LiteralType:
        '''Returns the LiteralType'''
        pass

class UnitLiteral(Literal[unit]):
    '''AST Unit Literal'''

    def __init__(self, pos: Position) -> None:
        super().__init__(pos, unit())

    def get_lit_type(self) -> LiteralType:
        return LiteralType.UNIT

class IntLiteral(Literal[int]):
    '''AST Int Literal'''

    def get_lit_type(self) -> LiteralType:
        return LiteralType.INTEGER

class StringLiteral(Literal[str]):
    '''AST String Literal'''

    def get_lit_type(self) -> LiteralType:
        return LiteralType.STRING

class BoolLiteral(Literal[bool]):
    '''AST Bool Literal'''

    def get_lit_type(self) -> LiteralType:
        return LiteralType.BOOL

class CharLiteral(Literal[char]):
    '''AST Char Literal'''

    def get_lit_type(self) -> LiteralType:
        return LiteralType.CHAR