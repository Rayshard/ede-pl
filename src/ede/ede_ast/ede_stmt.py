from abc import abstractmethod
from enum import Enum, auto
from typing import List, Optional
from .ede_type_symbol import TypeSymbol
from .ede_ast import Node, NodeType
from ede_utils import Position
from .ede_expr import Expression

class StmtType(Enum):
    '''Enumeration of AST statement types'''

    EXPR = auto()
    VAR_DECL = auto()
    BLOCK = auto()
    IF_ELSE = auto()
    
class Statement(Node):
    '''AST statement node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST statement node'''
        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.STMT

    def __str__(self) -> str:
        return f"{super().__str__()}|{self.get_stmt_type().name}"

    @abstractmethod
    def get_stmt_type(self) -> StmtType:
        '''Returns the StmtType'''
        pass
    
class Block(Statement):
    '''AST block node'''

    def __init__(self, stmts: List[Statement], pos: Position) -> None:
        '''Creates an AST block node'''

        super().__init__(pos)
        self.stmts = stmts

    def get_stmt_type(self) -> StmtType:
        return StmtType.BLOCK

class ExprStmt(Statement):
    '''AST expression statement node'''

    def __init__(self, expr: Expression) -> None:
        '''Creates an AST expression statement node'''

        super().__init__(expr.position)
        self.expr = expr

    def get_stmt_type(self) -> StmtType:
        return StmtType.EXPR

    def __str__(self) -> str:
        return f"{super().__str__()}|{self.expr.get_expr_type().name}"

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

class IfElseStmt(Statement):
    '''AST ifelse statement node'''

    def __init__(self, cond: Expression, thenClause: Statement, elseClause: Optional[Statement], pos: Position) -> None:
        '''Creates an AST ifelse statement node'''

        super().__init__(pos)
        self.condition = cond
        self.thenClause = thenClause
        self.elseClause = elseClause

    def get_stmt_type(self) -> StmtType:
        return StmtType.IF_ELSE
