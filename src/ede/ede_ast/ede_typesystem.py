from abc import abstractmethod
from enum import Enum, auto
from typing import Any, Dict, List, NamedTuple, Optional, Type, Union
from ede_utils import Error, ErrorType, JSONSerializable, Position, Result, Success

class ArrayTypeSymbol:
    '''Representation of an array type symbol'''

    def __init__(self, inner: 'TypeSymbol') -> None:
        '''Create array type symbol'''
        self.inner = inner

    def __eq__(self, o: object) -> bool:
        return isinstance(o, ArrayTypeSymbol) and o.inner == self.inner

    def __str__(self) -> str:
        return f"[{str(self.inner)}]"

class TupleTypeSymbol:
    '''Representation of a tuple type symbol'''

    def __init__(self, inners: List['TypeSymbol']) -> None:
        '''Create tuple type symbol'''
        
        assert len(inners) > 1, 'Tuple type symbols must have at least 2 inner type symbols'
        self.inners = inners

    def __eq__(self, o: object) -> bool:
        return isinstance(o, TupleTypeSymbol) and o.inners == self.inners

    def __str__(self) -> str:
        return f"({','.join([str(inner) for inner in self.inners])})"

class RecordTypeSymbol:
    '''Representation of a record type symbol'''

    def __init__(self, items: Dict[str, 'TypeSymbol']) -> None:
        '''Create record type symbol'''
        
        assert len(items) > 0, 'Record type symbols must have at least 1 item'
        self.items = items

    def __eq__(self, o: object) -> bool:
        return isinstance(o, RecordTypeSymbol) and o.items == self.items

    def __str__(self) -> str:
        return "{" + ','.join([f"{name}:{str(type_symbol)}" for name, type_symbol in self.items.items()]) + "}"

TypeSymbol = Union[str, ArrayTypeSymbol, TupleTypeSymbol, RecordTypeSymbol]

class TSType(Enum):
    '''Enumeration for type system base types'''

    PRIMITIVE = auto()
    TUPLE = auto()
    ARRAY = auto()
    OBJECT = auto()

class TSPrimitiveType(Enum):
    '''Enumeration for type system primitive types'''

    NULL = (auto(), 'null')
    INT = (auto(), 'int')
    STR = (auto(), 'string')
    CHAR = (auto(), 'char')
    BOOL = (auto(), 'bool')

    def get_type_symbol(self):
        return self.value[1]

class EdeType(JSONSerializable):
    '''Type system type'''

    def __init__(self) -> None:
        '''Create a type system type'''
        super().__init__()

    def is_type(self, type: Type['EdeType']):
        return isinstance(self, type)

    def __str__(self) -> str:
        return str(self.to_json())

    @abstractmethod
    def as_type_symbol(self) -> TypeSymbol:
        '''Converts type to its type symbol representation'''
        pass

class EdePrimitive(EdeType):
    '''Ede primitive type'''

    def __init__(self, type: TSPrimitiveType) -> None:
        '''Create Ede primitive type'''

        super().__init__()
        self.__type = type

    def get_type(self) -> TSPrimitiveType:
        '''Returns the primitive type'''
        return self.__type

    def to_json(self) -> Dict[str, Any]:
        return {
            '__type__': 'primitive',
            'type': self.__type.name
        }

    def as_type_symbol(self) -> TypeSymbol:
        return self.__type.get_type_symbol()

# Primitive Instances
EdeNull = EdePrimitive(TSPrimitiveType.NULL)
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

    def get_inner_type(self) -> EdeType:
        '''Returns the inner ede type'''
        return self.__inner_type

    def to_json(self) -> Dict[str, Any]:
        return {
            '__type__': 'array',
            'inner_type': self.__inner_type.to_json()
        }

    def as_type_symbol(self) -> TypeSymbol:
        return ArrayTypeSymbol(self.__inner_type.as_type_symbol())

class EdeTuple(EdeType):
    '''Ede tuple type'''

    def __init__(self, inner_types: List[EdeType]) -> None:
        '''Create Ede tuple type'''
        
        assert len(inner_types) > 1, 'Ede tuples must have at least 2 inner types'

        super().__init__()
        self.__inner_types = inner_types

    def get_inner_types(self) -> List[EdeType]:
        '''Returns the inner ede types'''
        return self.__inner_types

    def get_count(self) -> int:
        return len(self.__inner_types)

    def to_json(self) -> Dict[str, Any]:
        return {
            '__type__': 'tuple',
            'inner_types': [type.to_json() for type in self.__inner_types]
        }

    def as_type_symbol(self) -> TypeSymbol:
        return TupleTypeSymbol([type.as_type_symbol() for type in self.__inner_types])

class EdeRecord(EdeType):
    '''Ede record type'''

    def __init__(self, items: Dict[str, EdeType]) -> None:
        '''Create Ede record type'''
        
        super().__init__()
        self.__items = items

    def get_members(self) -> Dict[str, EdeType]:
        '''Returns the items of the record'''
        return self.__items
        
    def to_json(self) -> Dict[str, Any]:
        return {
            '__type__': 'record',
            'items': {name: type.to_json() for name, type in self.__items.items()}
        }

class EdeObject(EdeType):
    '''Ede object type'''

    def __init__(self, name: str, members: Dict[str, EdeType]) -> None:
        '''Create Ede object type'''
        
        super().__init__()
        self.__name = name
        self.__members = members

    def get_name(self) -> str:
        '''Returns the name of the object'''
        return self.__name

    def get_members(self) -> Dict[str, EdeType]:
        '''Returns the members of the object'''
        return self.__members
        
    def to_json(self) -> Dict[str, Any]:
        return {
            '__type__': 'object',
            'members': {name: type.to_json() for name, type in self.__members.items()}
        }

class EnvEntry(NamedTuple):
    '''Environment entry struct; holds the metadata for entries in a type checking environment'''
    
    type: EdeType
    pos: Position

class Environment:
    '''Type checking environment class; used as a context when type checking the ast'''

    def __init__(self, parent: Optional['Environment'] = None) -> None:
        '''Create a new type checking environment'''

        self.parent : Optional[Environment] = parent
        self.variables : Dict[str, EnvEntry] = {}

    def get(self, id: str, pos: Position) -> Result[EdeType]:
        '''
        Attempts to look up a the Ede type of the given id, first in the current environment,
        then recursively in the parent environments if they exists.
        '''

        if id in self.variables:
            return Success(self.variables[id].type)
        elif self.parent is not None:
            return self.parent.get(id, pos)
        else:
            return TypeCheckError.UnknownID(id, pos)

    def declare(self, id: str, type: EdeType, pos: Position, check_parent: bool = False) -> Optional[Error]:
        '''
        Adds the given id to the environment with the given type if it does not already exist.
        Existence in parent environments is not checked unless specified.
        '''
        
        if id in self.variables or (check_parent and self.parent is not None and self.parent.get(id, pos).is_success()):
            return TypeCheckError.MultipleDeclaration(id, self.variables[id].pos, pos)

        self.variables[id] = EnvEntry(type, pos)
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
    def MultipleDeclaration(id: str, init_decl_pos: Position, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_MULTIPLE_DECL, pos, f"Declaration of \"{id}\" already exists at {init_decl_pos}")