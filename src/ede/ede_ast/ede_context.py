from enum import Enum, auto
from typing import Dict, Generic, Optional, TypeVar
from ede_utils import Error, ErrorType, Position, Result, Success

class CtxEntryType(Enum):
    '''Enumeration of context entry types'''

    VARIABLE = auto()
    TYPENAME = auto()
    FUNCTION = auto()

class CtxEntry:
    '''Context entry; holds the metadata for entries in a context'''
    
    def __init__(self, type: CtxEntryType, pos: Position) -> None:
        '''Creates a context entry'''

        self.type = type
        self.pos = pos

class CtxError:
    '''Wrapper for context errors'''

    @staticmethod
    def UnknownID(id: str, pos: Position) -> Error:
        return Error(ErrorType.CONTEXT_UNKNOWN_ID, pos, f"Unknown id \"{id}\"")

    @staticmethod
    def IDConflict(id: str, init_decl_pos: Position, pos: Position) -> Error:
        return Error(ErrorType.CONTEXT_ID_CONFLICT, pos, f"\"{id}\" conflict with id at {init_decl_pos}")

T = TypeVar('T', bound=CtxEntry)
class Context(Generic[T]):
    '''Context class; used for traversing the ast'''

    def __init__(self, parent: Optional['Context[T]'] = None) -> None:
        '''Create a new context'''

        self.parent : Optional[Context[T]] = parent
        self.namespace : Dict[str, T]

    def get(self, id: str, pos: Position, check_parent: bool) -> Result[T]:
        '''
        Attempts to look up the entry of the given id, first in the current context,
        then recursively in the parent contexts if they exists and `check_parent` is True.
        '''

        if id in self.namespace:
            return Success(self.namespace[id])
        elif check_parent and self.parent is not None:
            return self.parent.get(id, pos, True)

        return CtxError.UnknownID(id, pos)

    def add(self, id: str, entry: T, check_parent: bool) -> Optional[Error]:
        '''
        Adds the given id to the context with the given entry if it does not already exist.
        Existence in parent contexts is not checked unless specified.
        '''
        
        if id in self.namespace:
            return CtxError.IDConflict(id, self.namespace[id].pos, entry.pos)
        elif check_parent and self.parent is not None:
            return self.parent.add(id, entry, True)

        self.namespace[id] = entry
        return None

    def set(self, id: str, entry: T) -> Optional[Error]:
        '''
        Sets the given id with the given entry. If the id is not in the current context, the first
        occurrence upong traversing up the parent tree is set.
        '''

        if id in self.namespace:
            self.namespace[id] = entry
            return None
        elif self.parent is not None:
            return self.parent.set(id, entry)

        raise Exception(f"ID '{id}' does not exist") # should not occur is type checking passed

    def get_entries(self, type: CtxEntryType) -> Dict[str, T]:
        return dict(filter(lambda e: e[1].type == type, self.namespace.items()))
