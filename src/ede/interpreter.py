from enum import Enum, auto
from typing import Dict, NamedTuple, Optional, Union, cast, get_args
from ede_utils import Position, char

class ExecExceptionType(Enum):
    '''Enumeration of execution exception types'''

    DIV_BY_ZERO = auto()

class ExecException(NamedTuple):
    '''Execution Exception'''

    type: ExecExceptionType
    name: str
    pos: Position
    msg: str

    @staticmethod
    def DivisionByZero(pos: Position) -> 'ExecException':
        return ExecException(ExecExceptionType.DIV_BY_ZERO, 'Division By Zero', pos, "Unable to divide by 0")

ExecValueTypes = Union[int, str, bool, char, ExecException]
class ExecValue:
    '''Representation of the possible values return on execution of an AST node'''

    def __init__(self, value: ExecValueTypes) -> None:
        '''Create a typed execution value'''

        self.value = value

    def __eq__(self, o: object) -> bool:
        if isinstance(o, ExecValue):
            return type(o.value) == type(self.value) and o.value == self.value
        elif isinstance(o, get_args(ExecValueTypes)):
            return type(o) == type(self.value) and o == self.value
        
        return False 

    def __str__(self) -> str:
        return f"Value: {self.value}, Type: {type(self.value).__name__}"

    def is_exception(self) -> bool:
        return isinstance(self.value, ExecException)

    def to_int(self) -> int:
        '''Returns value casted as int'''
        assert type(self.value) == int
        return cast(int, self.value)

    def to_str(self) -> str:
        '''Returns value casted as str'''

        assert type(self.value) == str
        return cast(str, self.value)

    def to_bool(self) -> bool:
        '''Returns value casted as bool'''

        assert type(self.value) == bool
        return cast(bool, self.value)

    def to_char(self) -> char:
        '''Returns value casted as char'''

        assert type(self.value) == char
        return cast(char, self.value) 

    def to_exception(self) -> ExecException:
        '''Returns value casted as ExecException'''

        assert type(self.value) == ExecException
        return cast(ExecException, self.value)

class ExecContext:
    '''Execution Context: a mutale environment that holds the state of a program during AST execution'''

    def __init__(self, parent: Optional['ExecContext'] = None) -> None:
        '''Create an execution environment'''

        self.parent : Optional[ExecContext] = parent
        self.variables : Dict[str, ExecValue] = {} 

    def get(self, id: str) -> ExecValue:
        '''Returns the typed value of a entry. Existence of the entry is assumed.'''
        
        return self.variables[id]

    def set(self, id: str, value: ExecValue, pos: Position):
        '''Sets the typed value for the given id and creates the id if not already in the context.'''
        
        self.variables[id] = value