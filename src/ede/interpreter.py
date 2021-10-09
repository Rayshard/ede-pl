from enum import Enum, auto
from typing import Dict, List, NamedTuple, Optional, Type, cast, get_args
from ede_ast.ede_context import Context, CtxEntryType
from ede_ast.ede_typesystem import EdeArray, EdeObject, EdePrimitive, EdeTuple, EdeType, TCCtxEntry, TSPrimitiveType
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

class ObjectValue(NamedTuple):
    '''Object value'''

    name: str
    members: Dict[str, 'ExecValue']

    def __str__(self) -> str:
        return self.name + ' { ' + f"{', '.join([name + ' = ' + str(value) for name, value in self.members.items()])}" + ' }'

ExecValueTypes = int | str | bool | char | unit | ArrayValue | TupleValue | ObjectValue | ExecException

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

    def __to(self, t: Type[ExecValueTypes]) -> ExecValueTypes:
        '''Returns value casted as t'''
        assert type(self.value) == t
        return cast(t, self.value)

    def to_int(self) -> int:
        return cast(int, self.__to(int))

    def to_str(self) -> str:
        return cast(str, self.__to(str))

    def to_bool(self) -> bool:
        return cast(bool, self.__to(bool))

    def to_char(self) -> char:
        return cast(char, self.__to(char))

    def to_unit(self) -> unit:
        return cast(unit, self.__to(unit))

    def to_array(self) -> ArrayValue:
        return cast(ArrayValue, self.__to(ArrayValue))

    def to_tuple(self) -> TupleValue:
        return cast(TupleValue, self.__to(TupleValue))

    def to_object(self) -> ObjectValue:
        return cast(ObjectValue, self.__to(ObjectValue))

    def to_exception(self) -> ExecException:
        return cast(ExecException, self.__to(ExecException))
 
    @staticmethod
    def UNIT() -> 'ExecValue':
        return ExecValue(unit())

    @staticmethod
    def get_default_value(type: EdeType) -> 'ExecValue':
        match type:
            case EdePrimitive(prim_type=TSPrimitiveType.UNIT):
                return ExecValue(unit())
            case EdePrimitive(prim_type=TSPrimitiveType.INT):
                return ExecValue(0)
            case EdePrimitive(prim_type=TSPrimitiveType.STR):
                return ExecValue("")
            case EdePrimitive(prim_type=TSPrimitiveType.CHAR):
                return ExecValue('\0')
            case EdePrimitive(prim_type=TSPrimitiveType.BOOL):
                return ExecValue(False)
            case EdeTuple():
                return ExecValue(TupleValue(list(map(ExecValue.get_default_value, type.get_inner_types()))))
            case EdeArray():
                return ExecValue(ArrayValue([]))
            case EdeObject():
                return ExecValue(ObjectValue(type.get_name(), {id: ExecValue.get_default_value(mem_type) for id, mem_type in type.get_members().items()}))
            case _:
                raise Exception('Case not handled')

class ExecEntry(TCCtxEntry):
    '''Execution context entry'''
    
    def __init__(self, type: CtxEntryType, ede_type: EdeType, value: ExecValue, pos: Position) -> None:
        '''Create entry'''

        super().__init__(type, ede_type, pos)
        self.value = value

class ExecContext(Context[ExecEntry]):
    def __init__(self, parent: Optional['Context[ExecEntry]'] = None) -> None:
        super().__init__(parent=parent)
