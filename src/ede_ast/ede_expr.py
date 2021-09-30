from abc import abstractmethod
from enum import Enum, auto
from ede_utils import Position, Result
from .ede_type import EdeType, Environment, TypeCheckError
from .ede_ast import ExecContext, ExecException, ExecResult, Node, NodeType

class ExprType(Enum):
    '''Enumeration of AST expression types'''

    LITERAL = auto()
    BINOP = auto()
    UNOP = auto()
    ID = auto()
    ASSIGN = auto()

class Expression(Node):
    '''AST expression node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST expression node'''

        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.EXPR

    @abstractmethod
    def get_expr_type(self) -> ExprType:
        '''Returns the ExpressionType'''
        pass

    @abstractmethod
    def _execute(self, ctx: ExecContext) -> ExecResult:
        pass

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
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

    def _execute(self, ctx: ExecContext) -> ExecResult:
        return ctx.get(self.id)

class AssignExpr(Expression):
    '''AST assignment expression node'''

    def __init__(self, pos: Position, id: str, expr: Expression) -> None:
        '''Creates an AST assignment expression node'''

        super().__init__(pos)
        self.id = id
        self.expr = expr

    def get_expr_type(self) -> ExprType:
        return ExprType.ASSIGN

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        # typecheck expression
        expr_type = self.expr.typecheck(env)
        if expr_type.is_error():
            return expr_type
       
        # ensure id has already been declared
        env_res = env.get(self.id, self.position)
        if env_res.is_error():
            return env_res
        
        # return expression type if it matches the id's type
        return env_res if env_res.get() == expr_type.get() else TypeCheckError.InvalidAssignment(env_res.get(), expr_type.get(), self.position)
            
    def _execute(self, ctx: ExecContext) -> ExecResult:
        # execute expression
        expr_res = self.expr.execute(ctx)
        if isinstance(expr_res, ExecException):
            return expr_res

        # set the value of the id to the value of the expression
        ctx.set(self.id, expr_res, self.position)
        return expr_res
