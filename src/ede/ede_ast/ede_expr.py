from abc import abstractmethod
from enum import Enum, auto
from typing import Any, Dict, Optional, cast
from .ede_ast import Node, NodeType
from ede_utils import Error, Position, Result, Success
from interpreter import ExecContext, ExecValue
from .ede_typesystem import EdeType, EnvEntryType, Environment, TypeCheckError

class ExprType(Enum):
    '''Enumeration of AST expression types'''

    LITERAL = auto()
    BINOP = auto()
    UNOP = auto()
    ID = auto()
    TYPE_SYMBOL = auto()

class Expression(Node):
    '''AST expression node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST expression node'''

        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.EXPR

    def to_json(self) -> Dict[str, Any]:
        result = super().to_json()
        result['expr_type'] = self.get_expr_type().name
        return result

    @abstractmethod
    def get_expr_type(self) -> ExprType:
        '''Returns the ExpressionType'''
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
        entry = env.get(self.id, self.position, True)
        if entry.is_error():
            return cast(Error, entry)
        elif entry.get().type != EnvEntryType.VARIABLE:
            return TypeCheckError.UnknownVariable(self.id, self.position)

        return Success(entry.get().ede_type)

    def _execute(self, ctx: ExecContext) -> Optional[ExecValue]:
        return ctx.get(self.id)

    def _to_json(self) -> Dict[str, Any]:
        return {
            "_type_": "Identifier Expression",
            "id": self.id
        }
