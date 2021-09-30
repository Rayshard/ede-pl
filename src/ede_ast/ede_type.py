from enum import Enum, auto
from typing import Dict, NamedTuple, Optional
from ede_utils import Error, ErrorType, Position, Result, Success

class EdeType(Enum):
    '''Enumeration of data types for Ede'''

    NULL = auto()
    INT = auto()
    STR = auto()
    CHAR = auto()
    BOOL = auto()

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