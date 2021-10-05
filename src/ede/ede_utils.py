from typing import Generic, NamedTuple, NoReturn, Optional, TypeVar, Union
from enum import IntEnum, auto

# TODO: Comment File

class char(str):
    '''Char data type'''

    def __new__(cls, value: str):
        assert len(value) == 1, "char must be of length 1"

        obj = str.__new__(cls, value)
        return obj

class unit:
    '''Unit data type'''
    
    def __str__(self) -> str:
        return "()"

class Position(NamedTuple):
    line: int = 1
    column: int = 1

    def __str__(self) -> str:
        return str((self.line, self.column))

T = TypeVar('T')

class Positioned(Generic[T]):
    def __init__(self, value: T, pos: Position) -> None:
        self.value = value
        self.position = pos

class ErrorType(IntEnum):
    DEFAULT = auto()
    INVALID_INT_LIT = auto()
    INVALID_STR_LIT = auto()
    INVALID_CHAR_LIT = auto()
    UNEXPECTED_EOF = auto()
    INVALID_ID = auto()

    TYPECHECKING_INVALID_BINOP = auto()
    TYPECHECKING_UNKNOWN_ID = auto()
    TYPECHECKING_UNKNOWN_VAR = auto()
    TYPECHECKING_INVALID_ASSIGN = auto()
    TYPECHECKING_ID_CONFLICT = auto()
    TYPECHECKING_UNRESOLVABLE_TYPENAME = auto()
    TYPECHECKING_INCOMPATIBLE_IF_ELSE_CLAUSES = auto()
    TYPECHECKING_REINIT = auto()
    TYPECHECKING_UNEXPECTED_TYPE = auto()
    TYPECHECKING_UNDEF_OBJ = auto()
    TYPECHECKING_UNEXPECTED_INIT = auto()

    PARSING_UNEXPECTED_TOKEN = auto()
    PARSING_INVALID_OPERATOR = auto()
    PARSING_DUP_MEMBER_NAME = auto()

    CONTEXT_UNKNOWN_ID = auto()
    CONTEXT_ID_CONFLICT = auto()

class Success(Generic[T]):
    def __init__(self, value: T) -> None:
        self.value = value

    def is_success(self) -> bool:
        return True

    def is_error(self, type: ErrorType = ErrorType.DEFAULT) -> bool:
        return False

    def get(self) -> T:
        return self.value

    def error(self) -> 'Error':
        raise Exception('Called error on a success')

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

    def error(self) -> 'Error':
        return self

    def get_output_msg(self, file_path: str) -> str:
        return f"{file_path}:{self.position.line}:{self.position.column} {self.type.name}: {self.msg}"    

    def convert_to(self, type: ErrorType) -> 'Error':
        return Error(type, self.position, self.msg)

Result = Union[Success[T], Error]
