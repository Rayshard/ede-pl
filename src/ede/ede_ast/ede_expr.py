from abc import abstractmethod
from enum import Enum, auto
from typing import Dict, List, Optional, Tuple
from .ede_type_symbol import TypeSymbol

from .ede_typesystem import EdeType
from .ede_ast import Node, NodeType
from ..ede_utils import Error, ErrorType, Position, Positioned

class ExprType(Enum):
    '''Enumeration of AST expression types'''

    LITERAL = auto()
    BINOP = auto()
    UNOP = auto()
    ID = auto()
    ARRAY_INIT = auto()
    TUPLE_INIT = auto()
    OBJ_INIT = auto()
    DEFAULT = auto()
    FUNC_CALL = auto()

class Expression(Node):
    '''AST expression node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST expression node'''

        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.EXPR

    def __str__(self) -> str:
        return f"{super().__str__()}|{self.get_expr_type().name}"

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

def TypeCheckError_InvalidBinop(op: 'BinopType', ltype: EdeType, rtype: EdeType, pos: Position) -> Error:
    return Error(ErrorType.TYPECHECKING_INVALID_BINOP, pos, f"Cannot perform {op} on {ltype} and {rtype}")

class BinopType(Enum):
    '''Enumeration of AST binop types'''

    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()
    ASSIGN = auto()

class BinopExpr(Expression):
    '''AST binop expression node'''

    def __init__(self, pos: Position, left: Expression, right: Expression, op: BinopType) -> None:
        '''Creates an AST binop expression node'''

        assert op != BinopType.ASSIGN or isinstance(left, IdentifierExpr)  # ensure that identifiers are the LHS of assignments

        super().__init__(pos)
        self.left = left
        self.right = right
        self.op = op
        self.type_pattern : Optional[Tuple[EdeType, EdeType, BinopType]] = None

    def get_expr_type(self) -> ExprType:
        return ExprType.BINOP

class ArrayInitExpr(Expression):
    '''AST array initializer expression node'''

    def __init__(self, exprs: List[Expression], pos: Position) -> None:
        '''Creates an AST array initializer expression node'''

        super().__init__(pos)
        self.exprs = exprs

    def get_expr_type(self) -> ExprType:
        return ExprType.ARRAY_INIT

class TupleInitExpr(Expression):
    '''AST tuple initializer expression node'''

    def __init__(self, exprs: List[Expression], pos: Position) -> None:
        '''Creates an AST tuple initializer expression node'''

        super().__init__(pos)
        self.exprs = exprs

    def get_expr_type(self) -> ExprType:
        return ExprType.TUPLE_INIT

class DefaultExpr(Expression):
    '''AST default expression node'''

    def __init__(self, type_symbol: TypeSymbol) -> None:
        '''Creates an AST default expression node'''

        super().__init__(type_symbol.position)
        self.type_symbol = type_symbol

    def get_expr_type(self) -> ExprType:
        return ExprType.DEFAULT

class FuncCallExpr(Expression):
    '''AST function call expression node'''

    def __init__(self, name: str, args: List[Expression], pos: Position) -> None:
        '''Creates an AST function call expression node'''

        super().__init__(pos)
        self.name = name
        self.args = args

    def get_expr_type(self) -> ExprType:
        return ExprType.FUNC_CALL

class ObjInitExpr(Expression):
    '''AST object initializer expression node'''

    def __init__(self, name: str, items: Dict[Positioned[str], Expression], pos: Position) -> None:
        '''Creates an AST object initializer expression node'''

        super().__init__(pos)
        self.name = name
        self.items = items

    def get_expr_type(self) -> ExprType:
        return ExprType.OBJ_INIT