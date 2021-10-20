from abc import abstractmethod
from enum import Enum, auto
from typing import Dict, OrderedDict
from .ede_stmt import Statement
from .ede_type_symbol import TypeSymbol
from .ede_ast import Node, NodeType
from ..ede_utils import Position, Positioned

class DefType(Enum):
    '''Enumeration of AST definition types'''

    OBJ = auto()
    ENUM = auto()
    FUNC = auto()
    
class Definition(Node):
    '''AST definition node'''

    def __init__(self, name: str, pos: Position) -> None:
        '''Creates an AST definition node'''
        super().__init__(pos)
        self.name = name

    def get_node_type(self) -> NodeType:
        return NodeType.DEF

    @abstractmethod
    def get_def_type(self) -> DefType:
        '''Returns the DefType'''
        pass
    
class ObjDef(Definition):
    '''AST object definition node'''

    def __init__(self, name: str, members: Dict[Positioned[str], TypeSymbol], pos: Position) -> None:
        '''Creates an AST object definition node'''

        super().__init__(name, pos)
        self.members = members

    def get_def_type(self) -> DefType:
        return DefType.OBJ

class FuncDef(Definition):
    '''AST function definition node'''

    def __init__(self, name: str, args: OrderedDict[Positioned[str], TypeSymbol], ret: TypeSymbol, body: Statement, pos: Position) -> None:
        '''Creates an AST function definition node'''

        super().__init__(name, pos)
        self.args = args
        self.ret = ret
        self.body = body

    def get_def_type(self) -> DefType:
        return DefType.FUNC

