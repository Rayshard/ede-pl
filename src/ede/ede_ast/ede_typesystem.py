from abc import abstractmethod
from collections import OrderedDict
from enum import Enum, auto
from typing import Dict, List, Optional, Type, cast
from .ede_context import Context, CtxEntry, CtxEntryType
from ede_utils import Error, ErrorType, Position, Result

class TSType(Enum):
    '''Enumeration for type system base types'''

    ANY = auto()
    PRIMITIVE = auto()
    TUPLE = auto()
    ARRAY = auto()
    OBJECT = auto()
    FUNCTION = auto()

class TSPrimitiveType(Enum):
    '''Enumeration for type system primitive types'''

    UNIT = auto()
    INT = auto()
    STR = auto()
    CHAR = auto()
    BOOL = auto()

class EdeType:
    '''Type system type'''

    def is_type(self, type: Type['EdeType']):
        return isinstance(self, type)

    @abstractmethod
    def castable_to(self, type: 'EdeType') -> bool:
        '''Determines if the caller is castable to the specified type'''
        pass

    @abstractmethod
    def get_ts_type(self) -> TSType:
        '''Returns the type system type'''
        pass

class EdePrimitive(EdeType):
    '''Ede primitive type'''
    
    def __init__(self, prim_type: TSPrimitiveType) -> None:
        self.prim_type = prim_type

    def __eq__(self, o: object) -> bool:
        return isinstance(o, EdePrimitive) and o.prim_type == self.prim_type

    def __str__(self) -> str:
        match self.prim_type:
            case TSPrimitiveType.UNIT:
                return "unit"
            case TSPrimitiveType.INT:
                return "int"
            case TSPrimitiveType.STR:
                return "string"
            case TSPrimitiveType.BOOL:
                return "bool"
            case TSPrimitiveType.CHAR:
                return "char"
            case _:
                raise Exception('Case not handled')

    def get_ts_type(self) -> TSType:
        return TSType.PRIMITIVE

    def castable_to(self, type: EdeType) -> bool:
        return self == type

    @staticmethod
    def UNIT() -> 'EdePrimitive':
        return EdePrimitive(TSPrimitiveType.UNIT)

    @staticmethod
    def INT() -> 'EdePrimitive':
        return EdePrimitive(TSPrimitiveType.INT)

    @staticmethod
    def STRING() -> 'EdePrimitive':
        return EdePrimitive(TSPrimitiveType.STR)

    @staticmethod
    def CHAR() -> 'EdePrimitive':
        return EdePrimitive(TSPrimitiveType.CHAR)

    @staticmethod
    def BOOL() -> 'EdePrimitive':
        return EdePrimitive(TSPrimitiveType.BOOL)

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

    def castable_to(self, type: EdeType) -> bool:
        return type.get_ts_type() == TSType.ARRAY and self.__inner_type.castable_to(cast(EdeArray, type).__inner_type)

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

    def castable_to(self, type: EdeType) -> bool:
        if type.get_ts_type() != TSType.TUPLE:
            return False
        
        other = cast(EdeTuple, type)
        if other.get_count() != self.get_count():
            return False

        for i, it in enumerate(other.__inner_types):
            if not self.__inner_types[i].castable_to(it):
                return False

        return True

    def __str__(self) -> str:
        return f"({', '.join([str(inner) for inner in self.__inner_types])})"

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

    def castable_to(self, type: EdeType) -> bool:
        return type == self

    def __str__(self) -> str:
        return self.__name + " { " + ', '.join([f"{id}: {type}" for id, type in self.__members.items()]) + " }"

    def __eq__(self, o: object) -> bool:
        return isinstance(o, EdeObject) and o.__name == self.__name and o.__members == self.__members
        
class EdeFunc(EdeType):
    '''Ede function type'''

    def __init__(self, args: OrderedDict[str, EdeType], ret: EdeType) -> None:
        '''Create Ede function type'''
        
        super().__init__()
        self.__args = args
        self.__ret = ret

    def get_ts_type(self) -> TSType:
        return TSType.FUNCTION

    def get_args(self) -> OrderedDict[str, EdeType]:
        '''Returns the arguments of the function'''
        return self.__args

    def get_ret(self) -> EdeType:
        '''Returns the return type of the funtion'''
        return self.__ret
        
    def castable_to(self, type: EdeType) -> bool:
        return False

    def __str__(self) -> str:
        return "(" + ', '.join([f"{id}: {type}" for id, type in self.__args.items()]) + ") -> " + str(self.__ret)

    def __eq__(self, o: object) -> bool:
        return isinstance(o, EdeFunc) and o.__args == self.__args and o.__ret == self.__ret

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
    def CannotDeduceExprType(pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_CANNOT_DEDUCE_EXPR_TYPE, pos, f"Cannot deduce expression type")

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
