from abc import abstractmethod
from enum import Enum, auto
from typing import Optional
from ede_utils import Position
from .ede_typesystem import EdeType

class NodeType(Enum):
    '''Enumeration of AST node types'''

    STMT = auto()
    EXPR = auto()
    TYPE_SYMBOL = auto()
    
class Node:
    '''AST node'''

    def __init__(self, pos: Position) -> None:
        '''Create an AST node'''

        self.position = pos
        self.__ede_type : Optional[EdeType] = None

    def get_ede_type(self) -> EdeType:
        '''Returns the ede type; node must have already been type checked.'''

        assert self.__ede_type is not None, "Node has not been type checked!"
        return self.__ede_type

    def set_ede_type(self, ede_type: Optional[EdeType]) -> None:
        self.__ede_type = ede_type

    @abstractmethod
    def get_node_type(self) -> NodeType:
        '''Returns the NodeType'''
        pass
