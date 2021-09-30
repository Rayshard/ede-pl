from abc import abstractmethod
from enum import Enum, auto
from ede_utils import Position, Result
from .ede_type import EdeType, Environment, TypeCheckError
from .ede_ast import ExecContext, ExecException, ExecResult, Node, NodeType

class ExprType(Enum):
    LITERAL = auto()
    BINOP = auto()
    UNOP = auto()
    ID = auto()
    ASSIGN = auto()

class Expression(Node):
    def __init__(self, pos: Position) -> None:
        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.EXPR

    @abstractmethod
    def get_expr_type(self) -> ExprType:
        pass

    @abstractmethod
    def execute(self, ctx: ExecContext) -> ExecResult:
        pass

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        pass

class IdentifierExpr(Expression):
    def __init__(self, pos: Position, id: str) -> None:
        super().__init__(pos)
        self.id = id

    def get_expr_type(self) -> ExprType:
        return ExprType.ID

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return env.get(self.id, self.position)

    def execute(self, ctx: ExecContext) -> ExecResult:
        return ctx.get(self.id)

class AssignExpr(Expression):
    def __init__(self, pos: Position, id: str, expr: Expression) -> None:
        super().__init__(pos)
        self.id = id
        self.expr = expr

    def get_expr_type(self) -> ExprType:
        return ExprType.ASSIGN

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        expr_type = self.expr.typecheck(env)
        if expr_type.is_error():
            return expr_type
       
        env_res = env.get(self.id, self.position)
        if env_res.is_error():
            return env_res
        
        return env_res if env_res.get() == expr_type.get() else TypeCheckError.InvalidAssignment(env_res.get(), expr_type.get(), self.position)
            
    def execute(self, ctx: ExecContext) -> ExecResult:
        expr_res = self.expr.execute(ctx)
        if isinstance(expr_res, ExecException):
            return expr_res

        ctx.set(self.id, expr_res, self.position)
        return expr_res
