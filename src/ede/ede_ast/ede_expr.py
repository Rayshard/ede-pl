from abc import abstractmethod
from enum import Enum, auto
from typing import Any, Dict
from ede_ast.ede_stmt import Statement, StmtType
from ede_utils import Position, Result
from interpreter import ExecContext, ExecValue
from .ede_typesystem import EdeType, Environment

class ExprType(Enum):
    '''Enumeration of AST expression types'''

    LITERAL = auto()
    BINOP = auto()
    UNOP = auto()
    ID = auto()
    TYPE_SYMBOL = auto()

class Expression(Statement):
    '''AST expression node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST expression node'''

        super().__init__(pos)

    def get_stmt_type(self) -> StmtType:
        return StmtType.EXPR

    @abstractmethod
    def get_expr_type(self) -> ExprType:
        '''Returns the ExpressionType'''
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
    
class IdentifierExpr(Expression):
    '''AST identifier expression node'''

    def __init__(self, pos: Position, id: str) -> None:
        '''Creates an AST identifier expression node'''

        super().__init__(pos)
        self.id = id

    def get_expr_type(self) -> ExprType:
        return ExprType.ID

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return env.get(self.id, self.position)

    def _execute(self, ctx: ExecContext) -> ExecValue:
        return ctx.get(self.id)

    def to_json(self) -> Dict[str, Any]:
        return {
            "_type_": "Identifier Expression",
            "id": self.id
        }
