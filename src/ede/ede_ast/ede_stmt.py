from abc import abstractmethod
from enum import Enum, auto
from typing import Any, Dict
from ede_ast.ede_ast import Node, NodeType
from ede_ast.ede_expr import Expression
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
    
class ExprStmt(Statement):
    '''AST expression statement node'''

    def __init__(self, expr: Expression) -> None:
        '''Creates an AST expression statement node'''

        super().__init__(expr.position)
        self.expr = expr

    def get_stmt_type(self) -> StmtType:
        return StmtType.EXPR

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return self.expr.typecheck(env)

    def _execute(self, ctx: ExecContext) -> ExecValue:
        return self.expr.execute(ctx)

    def to_json(self) -> Dict[str, Any]:
        return {
            "_type_": "Expression Statement",
            "expr": self.expr.to_json()
        }
