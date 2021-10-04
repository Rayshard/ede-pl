from abc import abstractmethod
from enum import Enum, auto
from typing import Dict, List, NamedTuple, Optional, Type
from ede_utils import Error, ErrorType, Position, Result, Success

class TSType(Enum):
    '''Enumeration for type system base types'''

    PRIMITIVE = auto()
    TUPLE = auto()
    ARRAY = auto()
    RECORD = auto()
    OBJECT = auto()

class TSPrimitiveType(Enum):
    '''Enumeration for type system primitive types'''

    UNIT = (auto(), 'unit')
    INT = (auto(), 'int')
    STR = (auto(), 'string')
    CHAR = (auto(), 'char')
    BOOL = (auto(), 'bool')

    def get_type_symbol(self):
        return self.value[1]

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

    def __init__(self, type: TSPrimitiveType) -> None:
        '''Create Ede primitive type'''

        super().__init__()
        self.__type = type

    def get_ts_type(self) -> TSType:
        return TSType.PRIMITIVE

    def get_type(self) -> TSPrimitiveType:
        '''Returns the primitive type'''
        return self.__type

# Primitive Instances
EdeUnit = EdePrimitive(TSPrimitiveType.UNIT)
EdeInt = EdePrimitive(TSPrimitiveType.INT)
EdeString = EdePrimitive(TSPrimitiveType.STR)
EdeChar = EdePrimitive(TSPrimitiveType.CHAR)
EdeBool = EdePrimitive(TSPrimitiveType.BOOL)

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

class EdeRecord(EdeType):
    '''Ede record type'''

    def __init__(self, items: Dict[str, EdeType]) -> None:
        '''Create Ede record type'''
        
        super().__init__()
        self.__items = items

    def get_ts_type(self) -> TSType:
        return TSType.RECORD

    def get_members(self) -> Dict[str, EdeType]:
        '''Returns the items of the record'''
        return self.__items

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
        
class EnvEntryType(Enum):
    '''Enumeration of environment entry types'''

    VARIABLE = auto()
    TYPENAME = auto()

class EnvEntry(NamedTuple):
    '''Environment entry struct; holds the metadata for entries in a type checking environment'''
    
    type: EnvEntryType
    ede_type: EdeType
    pos: Position

class Environment:
    '''Type checking environment class; used as a context when type checking the ast'''

    def __init__(self, parent: Optional['Environment'] = None) -> None:
        '''Create a new type checking environment'''

        self.parent : Optional[Environment] = parent
        self.namespace : Dict[EnvEntryType, Dict[str, EnvEntry]] = {type: {} for type in EnvEntryType}

    def get(self, id: str, pos: Position, check_parent: bool) -> Result[EnvEntry]:
        '''
        Attempts to look up a the Ede type of the given id, first in the current environment,
        then recursively in the parent environments if they exists and `check_parent` is True.
        '''

        for _, items in self.namespace.items():
            if id in items:
                return Success(items[id])            

        if check_parent and self.parent is not None:
            return self.parent.get(id, pos, True)

        return TypeCheckError.UnknownID(id, pos)

    def declare(self, id: str, entry: EnvEntry, check_parent: bool) -> Optional[Error]:
        '''
        Adds the given id to the environment with the given type and ede type if it does not already exist.
        Existence in parent environments is not checked unless specified.
        '''
        
        for _, items in self.namespace.items():
            if id in items:
                return TypeCheckError.MultipleDeclaration(id, items[id].pos, entry.pos)

        if check_parent and self.parent is not None:
            return self.parent.declare(id, entry, True)

        self.namespace[entry.type][id] = entry
        return None

class TypeCheckError:
    '''Wrapper for type checking errors'''

    @staticmethod
    def InvalidAssignment(ltype: EdeType, rtype: EdeType, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_INVALID_ASSIGN, pos, f"Cannot assign {rtype} to {ltype}")

    @staticmethod
    def UnknownID(id: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNKNOWN_ID, pos, f"Unknown id \"{id}\"")

    @staticmethod
    def UnknownVariable(id: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNKNOWN_VAR, pos, f"Unknown variable \"{id}\"")

    @staticmethod
    def MultipleDeclaration(id: str, init_decl_pos: Position, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_MULTIPLE_DECL, pos, f"Declaration of \"{id}\" already exists at {init_decl_pos}")

    @staticmethod
    def UnresolvableTypeName(typename: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNRESOLVABLE_TYPENAME, pos, f"Could not resolve \"{typename}\" to a valid type")