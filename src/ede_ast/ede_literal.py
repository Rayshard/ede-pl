from abc import abstractmethod
from enum import Enum, auto
from ede_utils import Position, Result, Success
from .ede_ast import ExecContext, ExecResult, TypedExecValue
from .ede_expr import Expression, ExprType
from .ede_type import EdeType, Environment
from typing import Generic, TypeVar

class LiteralType(Enum):
    '''Enumeration of AST literal types'''

    INTEGER = auto()
    STRING = auto()
    CHAR = auto()
    BOOL = auto()

T = TypeVar('T', int, str, bool)
class Literal(Expression, Generic[T]):
    '''AST Literal node'''

    def __init__(self, pos: Position, value: T) -> None:
        '''Creates an AST Literal node'''

        super().__init__(pos)
        self.value : T = value

    def get_expr_type(self) -> ExprType:
        return ExprType.LITERAL

    def to_string(self, indent: int) -> str:
        return f"{self.get_lit_type()}: {self.value}"

    @abstractmethod
    def get_lit_type(self) -> LiteralType:
        '''Returns the LiteralType'''
        pass

    @abstractmethod
    def _execute(self, ctx: ExecContext) -> ExecResult:
        pass

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        pass

class IntLiteral(Literal[int]):
    '''AST integer Literal node'''

    def __init__(self, pos: Position, value: int) -> None:
        '''Creates an AST integer Literal node'''

        super().__init__(pos, value)

    def get_lit_type(self) -> LiteralType:
        return LiteralType.INTEGER

    def _execute(self, ctx: ExecContext) -> ExecResult:
        return TypedExecValue(self.get_type(), self.value)

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.INT)

class StringLiteral(Literal[str]):
    '''AST string Literal node'''

    def __init__(self, pos: Position, value: str) -> None:
        '''Creates an AST string Literal node'''

        super().__init__(pos, value)

    def get_lit_type(self) -> LiteralType:
        return LiteralType.STRING

    def _execute(self, ctx: ExecContext) -> ExecResult:
        return TypedExecValue(self.get_type(), self.value)

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.STR)

class CharLiteral(Literal[str]):
    '''AST char Literal node'''

    def __init__(self, pos: Position, value: str) -> None:
        '''Creates an AST char Literal node'''

        super().__init__(pos, value)
        assert len(value) == 1, "value must be of length 1"

    def get_lit_type(self) -> LiteralType:
        return LiteralType.CHAR

    def _execute(self, ctx: ExecContext) -> ExecResult:
        return TypedExecValue(self.get_type(), self.value)

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.CHAR)

class BoolLiteral(Literal[bool]):
    '''AST bool Literal node'''

    def __init__(self, pos: Position, value: bool) -> None:
        '''Creates an AST bool Literal node'''

        super().__init__(pos, value)

    def get_lit_type(self) -> LiteralType:
        return LiteralType.BOOL

    def _execute(self, ctx: ExecContext) -> ExecResult:
        return TypedExecValue(self.get_type(), self.value)

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.BOOL)