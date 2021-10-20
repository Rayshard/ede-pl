from abc import abstractmethod
from enum import Enum, auto
from typing import Optional
from ..ede_utils import Position
from .ede_typesystem import EdeType

class NodeType(Enum):
    '''Enumeration of AST node types'''

    MODULE = auto()
    STMT = auto()
    EXPR = auto()
    TYPE_SYMBOL = auto()
    DEF = auto()
    
class Node:
    '''AST node'''

    def __init__(self, pos: Position) -> None:
        '''Create an AST node'''

        self.position = pos
        self.__ede_type : Optional[EdeType] = None
        self.__cfg_id = -1

    def get_ede_type(self) -> EdeType:
        '''Returns the ede type; node must have already been type checked.'''

        assert self.__ede_type is not None, "Node has not been type checked!"
        return self.__ede_type

    def set_ede_type(self, ede_type: Optional[EdeType]) -> None:
        '''Sets the ede type'''
        self.__ede_type = ede_type

    def get_cfg_id(self) -> int:
        '''Returns the CFG id; node must have already had the CFG id set.'''
        return self.__cfg_id

    def set_cfg_id(self, id: int) -> None:
        "Sets the CFG id for the node; node must not already have one set."

        assert self.__cfg_id == -1, "Node already has a cfg id set!"
        self.__cfg_id = id

    def __str__(self) -> str:
        return self.get_node_type().name

    @abstractmethod
    def get_node_type(self) -> NodeType:
        '''Returns the NodeType'''
        pass
    