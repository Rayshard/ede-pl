from enum import Enum, auto
from ede_utils import Position, Result, Success
from .ede_ast import ExecContext, ExecResult, TypedExecValue
from .ede_expr import Expression, ExprType
from .ede_type import EdeType, Environment
from typing import Any, Dict, Generic, NewType, TypeVar, cast

char = NewType('char', str)
T = TypeVar('T', int, str, char, bool)

class LiteralType(Enum):
    '''Enumeration of AST literal types'''

    INTEGER = (auto(), int, EdeType.INT)
    BOOL = (auto(), bool, EdeType.BOOL)
    CHAR = (auto(), char, EdeType.CHAR)
    STRING = (auto(), str, EdeType.STR)

    def get_type(self) -> Any:
        '''Returns the underlying python type'''
        return self.value[1]

    def get_ede_type(self) -> EdeType:
        '''Returns the associated ede type'''
        return cast(EdeType, self.value[2])

LIT_TYPE_DICT = {item.get_type(): item for item in LiteralType}  # Map between python types and ast literal types
LIT_EDE_TYPE_DICT = {item: item.get_ede_type() for item in LiteralType}  # Map between python types and ast literal types

class Literal(Expression, Generic[T]):
    '''AST Literal node'''

    def __init__(self, pos: Position, value: T) -> None:
        '''Creates an AST Literal node'''

        super().__init__(pos)
        self.value : T = value

    def get_expr_type(self) -> ExprType:
        return ExprType.LITERAL

    def _execute(self, ctx: ExecContext) -> ExecResult:
        return TypedExecValue(self.get_type(), self.value)

    def to_json(self) -> Dict[str, Any]:
        return {
            "_type_": str(self.get_lit_type()),
            "value": self.value
        }

    def get_lit_type(self) -> LiteralType:
        '''Returns the LiteralType'''
        return LIT_TYPE_DICT[type(self.value)]

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(LIT_EDE_TYPE_DICT[self.get_lit_type()])

# Literal Class Types
IntLiteral = Literal[int]
StringLiteral = Literal[str]
CharLiteral = Literal[char]
BoolLiteral = Literal[bool]
