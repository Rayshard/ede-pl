from typing import List

from ede_utils import Position
from .ede_ast import Node, NodeType
from .ede_stmt import Statement

class Module(Node):
    '''AST module node'''

    def __init__(self, name: str, stmts: List[Statement]) -> None:
        '''Creates an AST module node'''
        super().__init__(Position())

        self.name = name
        self.stmts = stmts

    def get_node_type(self) -> NodeType:
        return NodeType.MODULE
