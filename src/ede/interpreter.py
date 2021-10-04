from enum import Enum, auto
from typing import Dict, List, NamedTuple, Optional, Union, cast, get_args
from ede_utils import Position, char, unit

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

class ArrayValue(NamedTuple):
    '''Array value'''

    values: List['ExecValue']

    def __str__(self) -> str:
        return f"[{', '.join([str(value) for value in self.values])}]"

class TupleValue(NamedTuple):
    '''Tuple value'''

    values: List['ExecValue']

    def __str__(self) -> str:
        return f"({', '.join([str(value) for value in self.values])})"

class RecordValue(NamedTuple):
    '''Record value'''

    items: Dict[str, 'ExecValue']

    def __str__(self) -> str:
        return '{' + f"{', '.join([name + '=' + str(value) for name, value in self.items.items()])}" + '}'

ExecValueTypes = Union[int, str, bool, char, unit, ArrayValue, TupleValue, RecordValue, ExecException]
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
        if isinstance(self.value, char):
            return "'" + str(self.value) + "'"
        elif isinstance(self.value, str):
            return '"' + str(self.value) + '"'
        
        return str(self.value)

    def is_exception(self) -> bool:
        return isinstance(self.value, ExecException)

    def to_unit(self) -> unit:
        '''Returns value casted as unit'''
        assert type(self.value) == unit
        return cast(unit, self.value)

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

    def to_array(self) -> ArrayValue:
        '''Returns value casted as array value'''

        assert type(self.value) == ArrayValue
        return cast(ArrayValue, self.value)

    def to_tuple(self) -> TupleValue:
        '''Returns value casted as tuple value'''

        assert type(self.value) == TupleValue
        return cast(TupleValue, self.value)

    def to_record(self) -> RecordValue:
        '''Returns value casted as record value'''

        assert type(self.value) == RecordValue
        return cast(RecordValue, self.value)

    @staticmethod
    def UNIT() -> 'ExecValue':
        return ExecValue(unit())

class ExecContext:
    '''Execution Context: a mutale environment that holds the state of a program during AST execution'''

    def __init__(self, parent: Optional['ExecContext'] = None) -> None:
        '''Create an execution environment'''

        self.parent : Optional[ExecContext] = parent
        self.variables : Dict[str, Optional[ExecValue]] = {} 

    def get(self, id: str) -> Optional[ExecValue]:
        '''Returns the typed value of a entry. Existence of the entry is assumed.'''
        
        return self.variables[id]

    def set(self, id: str, value: Optional[ExecValue], pos: Position):
        '''Sets the typed value for the given id and creates the id if not already in the context.'''
        
        self.variables[id] = value

    def __str__(self) -> str:
        return '\n'.join([f"{id}: {value}" for id, value in self.variables.items()])