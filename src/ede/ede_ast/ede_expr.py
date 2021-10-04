from abc import abstractmethod
from enum import Enum, auto
from typing import Dict, List
from .ede_ast import Node, NodeType
from ede_utils import Position

class ExprType(Enum):
    '''Enumeration of AST expression types'''

    LITERAL = auto()
    BINOP = auto()
    UNOP = auto()
    ID = auto()
    ARRAY = auto()
    TUPLE = auto()
    RECORD = auto()

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
    
class IdentifierExpr(Expression):
    '''AST identifier expression node'''

    def __init__(self, pos: Position, id: str) -> None:
        '''Creates an AST identifier expression node'''

        super().__init__(pos)
        self.id = id

    def get_expr_type(self) -> ExprType:
        return ExprType.ID

class ArrayExpr(Expression):
    '''AST array expression node'''

    def __init__(self, exprs: List[Expression], pos: Position) -> None:
        '''Creates an AST array expression node'''

        super().__init__(pos)
        self.exprs = exprs

    def get_expr_type(self) -> ExprType:
        return ExprType.ARRAY

class TupleExpr(Expression):
    '''AST tuple expression node'''

    def __init__(self, exprs: List[Expression], pos: Position) -> None:
        '''Creates an AST tuple expression node'''

        super().__init__(pos)
        self.exprs = exprs

    def get_expr_type(self) -> ExprType:
        return ExprType.TUPLE

class RecordExpr(Expression):
    '''AST record expression node'''

    def __init__(self, items: Dict[str, Expression], pos: Position) -> None:
        '''Creates an AST record expression node'''

        super().__init__(pos)
        self.items = items

    def get_expr_type(self) -> ExprType:
        return ExprType.RECORD