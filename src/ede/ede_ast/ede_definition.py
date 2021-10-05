from abc import abstractmethod
from enum import Enum, auto
from typing import Dict
from ede_ast.ede_type_symbol import TypeSymbol
from .ede_ast import Node, NodeType
from ede_utils import Position, Positioned

class DefType(Enum):
    '''Enumeration of AST definition types'''

    OBJ = auto()
    ENUM = auto()
    FUNC = auto()
    
class Definition(Node):
    '''AST definition node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST definition node'''
        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.DEF

    @abstractmethod
    def get_def_type(self) -> DefType:
        '''Returns the DefType'''
        pass
    
class ObjDef(Definition):
    '''AST object definition node'''

    def __init__(self, name: str, members: Dict[Positioned[str], TypeSymbol], pos: Position) -> None:
        '''Creates an AST block node'''

        super().__init__(pos)
        self.name = name
        self.members = members

    def get_def_type(self) -> DefType:
        return DefType.OBJ

