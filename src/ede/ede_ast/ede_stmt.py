from abc import abstractmethod
from enum import Enum, auto
from typing import Any, Dict, Optional, cast
from .ede_ast import Node, NodeType
from ede_utils import Error, Position, Result, Success
from interpreter import ExecContext, ExecValue
from .ede_typesystem import EdeType, EdeUnit, EnvEntry, EnvEntryType, Environment, TypeCheckError, TypeSymbol
from .ede_expr import Expression

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
    def _execute(self, ctx: ExecContext) -> Optional[ExecValue]:
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

    def _execute(self, ctx: ExecContext) -> Optional[ExecValue]:
        return self.expr.execute(ctx)

    def to_json(self) -> Dict[str, Any]:
        return self.expr.to_json()

class VarDeclStmt(Statement):
    '''AST variable declartion statement node'''

    def __init__(self, pos: Position, id: str, type_symbol: Optional[TypeSymbol], expr: Optional[Expression]) -> None:
        '''Creates an AST variable declartion statement node'''

        assert type_symbol is not None or expr is not None, 'Variable declaration must specify either the type symbol or the expression'

        super().__init__(pos)
        self.id = id
        self.type_symbol = type_symbol
        self.expr = expr

    def get_stmt_type(self) -> StmtType:
        return StmtType.VAR_DECL

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        ede_type: Optional[EdeType] = None

        if self.type_symbol is not None:
            env_res = env.resolve(self.type_symbol, self.position, True)
            if env_res.is_error():
                return cast(Error, env_res)

            ede_type = env_res.get()

        if self.expr is not None:
            expr_res = self.expr.typecheck(env)
            if expr_res.is_error():
                return expr_res

            if ede_type == None:
                ede_type = expr_res.get()
            elif expr_res.get() != ede_type:
                return TypeCheckError.InvalidAssignment(ede_type, expr_res.get(), self.position)

        decl_res = env.declare(self.id, EnvEntry(EnvEntryType.VARIABLE, cast(EdeType, ede_type), self.position), False)
        if decl_res is not None:
            return decl_res

        return Success(EdeUnit)

    def _execute(self, ctx: ExecContext) -> Optional[ExecValue]:
        ctx.set(self.id, self.expr.execute(ctx) if self.expr is not None else None, self.position)
        return None

    def to_json(self) -> Dict[str, Any]:
        return {
            "_type_": "Variable Declarartion",
            "id": self.id,
            "type_symbol": str(self.type_symbol) if self.type_symbol is not None else None,
            "expr": self.expr.to_json() if self.expr is not None else None,
        }