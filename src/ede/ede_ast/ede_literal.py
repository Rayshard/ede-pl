from abc import abstractmethod
from enum import Enum, auto
from ede_utils import Position, Result, Success, char
from .ede_ast import ExecContext, ExecValue
from .ede_expr import Expression, ExprType
from .ede_typesystem import EdeType, Environment
from typing import Any, Dict, Generic, TypeVar, cast

T = TypeVar('T', int, str, char, bool)

class LiteralType(Enum):
    '''Enumeration of AST literal types'''

    INTEGER = (auto(), EdeType.INT)
    BOOL = (auto(), EdeType.BOOL)
    CHAR = (auto(), EdeType.CHAR)
    STRING = (auto(), EdeType.STR)

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

    def _execute(self, ctx: ExecContext) -> ExecValue:
        return ExecValue(self.value)

    def to_json(self) -> Dict[str, Any]:
        return {
            "_type_": str(self.get_lit_type()),
            "value": self.value
        }

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(LIT_EDE_TYPE_DICT[self.get_lit_type()])

    @abstractmethod
    def get_lit_type(self) -> LiteralType:
        '''Returns the LiteralType'''
        pass

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