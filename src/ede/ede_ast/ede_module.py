from typing import List
from ede_ast.ede_definition import Definition

from ede_utils import Position
from .ede_ast import Node, NodeType
from .ede_stmt import Statement

class Module(Node):
    '''AST module node'''

    def __init__(self, file_path: str, name: str, defs: List[Definition], stmts: List[Statement]) -> None:
        '''Creates an AST module node'''
        super().__init__(Position())

        self.file_path = file_path
        self.name = name
        self.defs = defs
        self.stmts = stmts

    def get_node_type(self) -> NodeType:
        return NodeType.MODULE
