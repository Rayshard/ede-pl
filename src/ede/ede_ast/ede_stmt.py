from abc import abstractmethod
from enum import Enum, auto
from typing import Any, Dict
from ede_ast.ede_ast import Node, NodeType
from ede_utils import Position, Result
from interpreter import ExecContext, ExecValue
from .ede_typesystem import EdeType, Environment

class StmtType(Enum):
    '''Enumeration of AST statement types'''

    EXPR = auto()
    VAR_DECL = auto()
    
class Statement(Node):
    '''AST statement node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST statement node'''
        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.STMT

    @abstractmethod
    def get_stmt_type(self) -> StmtType:
        '''Returns the StmtType'''
        pass

    @abstractmethod
    def _execute(self, ctx: ExecContext) -> ExecValue:
        pass

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        pass

    @abstractmethod
    def to_json(self) -> Dict[str, Any]:
        pass
    