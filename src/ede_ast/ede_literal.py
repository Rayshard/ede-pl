from abc import abstractmethod
from enum import Enum, auto
from ede_utils import Position, Result, Success
from .ede_ast import ExecutionResult
from .ede_expr import Expression, ExprType
from .ede_type import EdeType, Environment
from typing import Generic, TypeVar

class LiteralType(Enum):
    INTEGER = auto()
    STRING = auto()
    CHAR = auto()
    BOOL = auto()

T = TypeVar('T', int, str, bool)

class Literal(Expression, Generic[T]):
    def __init__(self, pos: Position, value: T) -> None:
        super().__init__(pos)
        self.value : T = value

    def get_expr_type(self) -> ExprType:
        return ExprType.LITERAL

    @abstractmethod
    def get_lit_type(self) -> LiteralType:
        pass

    @abstractmethod
    def execute(self) -> ExecutionResult:
        pass

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        pass

class IntLiteral(Literal[int]):
    def __init__(self, pos: Position, value: int) -> None:
        super().__init__(pos, value)

    def get_lit_type(self) -> LiteralType:
        return LiteralType.INTEGER

    def execute(self) -> ExecutionResult:
        return self.value

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.INT)

class StringLiteral(Literal[str]):
    def __init__(self, pos: Position, value: str) -> None:
        super().__init__(pos, value)

    def get_lit_type(self) -> LiteralType:
        return LiteralType.STRING

    def execute(self) -> ExecutionResult:
        return self.value

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.STR)

class CharLiteral(Literal[str]):
    def __init__(self, pos: Position, value: str) -> None:
        super().__init__(pos, value)

    def get_lit_type(self) -> LiteralType:
        return LiteralType.CHAR

    def execute(self) -> ExecutionResult:
        return self.value

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.CHAR)

class BoolLiteral(Literal[bool]):
    def __init__(self, pos: Position, value: bool) -> None:
        super().__init__(pos, value)

    def get_lit_type(self) -> LiteralType:
        return LiteralType.BOOL

    def execute(self) -> ExecutionResult:
        return self.value

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(EdeType.BOOL)