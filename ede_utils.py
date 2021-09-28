from typing import Generic, NamedTuple, NoReturn, Optional, TypeVar, Union
from enum import Enum, auto
from abc import ABC, abstractmethod

class Position(NamedTuple):
    line: int
    column: int

    def __str__(self) -> str:
        return str((self.line, self.column))

T = TypeVar('T')

class ErrorType(Enum):
    DEFAULT = auto()
    INVALID_INT_LIT = auto()
    INVALID_STR_LIT = auto()
    UNEXPECTED_EOF = auto()

class Success(Generic[T]):
    def __init__(self, value: T) -> None:
        self.value = value

    def is_success(self) -> bool:
        return True

    def is_error(self, type: ErrorType = ErrorType.DEFAULT) -> bool:
        return False

    def get(self) -> T:
        return self.value

class Error(NamedTuple):
    type: ErrorType
    position: Position = Position(1, 1)
    msg: Optional[str] = None

    def is_success(self) -> bool:
        return False

    def is_error(self, type: ErrorType = ErrorType.DEFAULT) -> bool:
        return True and (type == ErrorType.DEFAULT or self.type == type)

    def get(self) -> NoReturn:
        raise Exception("Called 'get' on an Error")

    def __str__(self) -> str:
        return f"{self.position} {self.type.name}:  {self.msg}"    

Result = Union[Success[T], Error]
