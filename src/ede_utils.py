from typing import Generic, NamedTuple, NoReturn, Optional, TypeVar, Union
from enum import IntEnum, auto

class Position(NamedTuple):
    line: int = 1
    column: int = 1

    def __str__(self) -> str:
        return str((self.line, self.column))

T = TypeVar('T')

class ErrorType(IntEnum):
    DEFAULT = auto()
    INVALID_INT_LIT = auto()
    INVALID_STR_LIT = auto()
    INVALID_CHAR_LIT = auto()
    UNEXPECTED_EOF = auto()
    INVALID_ID = auto()

    TYPECHECKING_INVALID_BINOP = auto()
    TYPECHECKING_UNKNOWN_ID = auto()
    TYPECHECKING_INVALID_ASSIGN = auto()

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
        return f"{self.position} {self.type.name}: {self.msg}"    

Result = Union[Success[T], Error]