from abc import abstractmethod
from enum import Enum, auto
from typing import Dict, List, Optional, Type
from .ede_context import Context, CtxEntry, CtxEntryType
from ede_utils import Error, ErrorType, Position, Result

class TSType(Enum):
    '''Enumeration for type system base types'''

    UNIT = auto()
    INT = auto()
    STR = auto()
    CHAR = auto()
    BOOL = auto()
    TUPLE = auto()
    ARRAY = auto()
    OBJECT = auto()

class EdeType:
    '''Type system type'''

    def __init__(self) -> None:
        '''Create a type system type'''
        super().__init__()

    def is_type(self, type: Type['EdeType']):
        return isinstance(self, type)

    @abstractmethod
    def get_ts_type(self) -> TSType:
        '''Returns the type system type'''
        pass

class EdePrimitive(EdeType):
    '''Ede primitive type'''
    
    def __eq__(self, o: object) -> bool:
        return isinstance(o, EdePrimitive) and o.get_ts_type() == self.get_ts_type()

    @abstractmethod
    def get_ts_type(self) -> TSType:
        pass

class EdeUnit(EdePrimitive):
    '''Ede unit type'''

    def get_ts_type(self) -> TSType:
        return TSType.UNIT

    def __str__(self) -> str:
        return "unit"

class EdeInt(EdePrimitive):
    '''Ede int type'''

    def get_ts_type(self) -> TSType:
        return TSType.INT

    def __str__(self) -> str:
        return "int"

class EdeString(EdePrimitive):
    '''Ede string type'''

    def get_ts_type(self) -> TSType:
        return TSType.STR

    def __str__(self) -> str:
        return "string"

class EdeChar(EdePrimitive):
    '''Ede char type'''

    def get_ts_type(self) -> TSType:
        return TSType.CHAR

    def __str__(self) -> str:
        return "char"

class EdeBool(EdePrimitive):
    '''Ede bool type'''

    def get_ts_type(self) -> TSType:
        return TSType.BOOL

    def __str__(self) -> str:
        return "bool"

class EdeArray(EdeType):
    '''Ede array type'''

    def __init__(self, inner_type: EdeType) -> None:
        '''Create Ede array type'''
        super().__init__()
        self.__inner_type = inner_type

    def get_ts_type(self) -> TSType:
        return TSType.ARRAY

    def get_inner_type(self) -> EdeType:
        '''Returns the inner ede type'''
        return self.__inner_type

    def __str__(self) -> str:
        return f"[{str(self.__inner_type)}]"

    def __eq__(self, o: object) -> bool:
        return isinstance(o, EdeArray) and o.__inner_type == self.__inner_type

class EdeTuple(EdeType):
    '''Ede tuple type'''

    def __init__(self, inner_types: List[EdeType]) -> None:
        '''Create Ede tuple type'''
        
        assert len(inner_types) > 1, 'Ede tuples must have at least 2 inner types'

        super().__init__()
        self.__inner_types = inner_types

    def get_ts_type(self) -> TSType:
        return TSType.TUPLE

    def get_inner_types(self) -> List[EdeType]:
        '''Returns the inner ede types'''
        return self.__inner_types

    def get_count(self) -> int:
        return len(self.__inner_types)

    def __str__(self) -> str:
        return f"({','.join([str(inner) for inner in self.__inner_types])})"

    def __eq__(self, o: object) -> bool:
        return isinstance(o, EdeTuple) and o.__inner_types == self.__inner_types

class EdeObject(EdeType):
    '''Ede object type'''

    def __init__(self, name: str, members: Dict[str, EdeType]) -> None:
        '''Create Ede object type'''
        
        super().__init__()
        self.__name = name
        self.__members = members

    def get_ts_type(self) -> TSType:
        return TSType.OBJECT

    def get_name(self) -> str:
        '''Returns the name of the object'''
        return self.__name

    def get_members(self) -> Dict[str, EdeType]:
        '''Returns the members of the object'''
        return self.__members

    def __str__(self) -> str:
        return self.__name + " { " + ', '.join([f"{id}: {type}" for id, type in self.__members.items()]) + " }"

    def __eq__(self, o: object) -> bool:
        return isinstance(o, EdeObject) and o.__name == self.__name and o.__members == self.__members
        
class TCCtxEntry(CtxEntry):
    '''Type checking context entry'''
    
    def __init__(self, type: CtxEntryType, ede_type: EdeType, pos: Position) -> None:
        '''Create entry'''

        super().__init__(type, pos)
        self.ede_type = ede_type

class TCContext(Context[TCCtxEntry]):
    def __init__(self, parent: Optional['Context[TCCtxEntry]'] = None) -> None:
        super().__init__(parent=parent)

    def get(self, id: str, pos: Position, check_parent: bool) -> Result[TCCtxEntry]:
        result = super().get(id, pos, check_parent)
        return result.error().convert_to(ErrorType.TYPECHECKING_UNKNOWN_ID) if result.is_error(ErrorType.CONTEXT_UNKNOWN_ID) else result 
    
    def add(self, id: str, entry: TCCtxEntry, check_parent: bool) -> Optional[Error]:
        result = super().add(id, entry, check_parent)
        return result.convert_to(ErrorType.TYPECHECKING_ID_CONFLICT) if result is not None and result.is_error(ErrorType.CONTEXT_ID_CONFLICT) else result 

class TypeCheckError:
    '''Wrapper for type checking errors'''

    @staticmethod
    def InvalidAssignment(ltype: EdeType, rtype: EdeType, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_INVALID_ASSIGN, pos, f"Cannot assign {rtype} to {ltype}")

    @staticmethod
    def IncompatibleIfElseClause(then_type: EdeType, else_type: EdeType, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_INCOMPATIBLE_IF_ELSE_CLAUSES, pos, f"Cannot convert else clause type {else_type} to then clause type {then_type}")

    @staticmethod
    def UnknownID(id: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNKNOWN_ID, pos, f"Unknown id \"{id}\"")

    @staticmethod
    def UnknownVariable(id: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNKNOWN_VAR, pos, f"Unknown variable \"{id}\"")

    @staticmethod
    def IDConflict(id: str, init_decl_pos: Position, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_ID_CONFLICT, pos, f"\"{id}\" conflict with id at {init_decl_pos}")

    @staticmethod
    def UnresolvableTypeName(typename: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNRESOLVABLE_TYPENAME, pos, f"Could not resolve \"{typename}\" to a valid type")

    @staticmethod
    def Reinitialization(id: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_REINIT, pos, f"'{id}' is already initialized")

    @staticmethod
    def Redefinition(id: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_REDEF, pos, f"'{id}' is already defined")

    @staticmethod
    def UnexpectedInitialization(name: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNEXPECTED_INIT, pos, f"Unexected initialized of '{name}'")

    @staticmethod
    def UnexpectedType(found: EdeType, expected: EdeType, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNEXPECTED_TYPE, pos, f"Found '{found}' but expected {expected}")

    @staticmethod
    def UndefinedObject(found: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNDEF_OBJ, pos, f"'{found}' is not a defined object")
