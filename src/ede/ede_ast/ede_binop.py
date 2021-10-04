from enum import Enum, auto
from typing import Callable, Dict, Optional, Tuple
from ede_utils import Position, Error, ErrorType
from .ede_typesystem import EdeInt, EdeString, EdeType
from .ede_expr import Expression, ExprType, IdentifierExpr
from interpreter import ExecContext, ExecException, ExecValue

def TypeCheckError_InvalidBinop(op: 'BinopType', ltype: EdeType, rtype: EdeType, pos: Position) -> Error:
    return Error(ErrorType.TYPECHECKING_INVALID_BINOP, pos, f"Cannot perform {op} on {ltype} and {rtype}")

class BinopType(Enum):
    '''Enumeration of AST binop types'''

    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()
    ASSIGN = auto()

# TODO: use match expressions in below functions instead of these dicts
# Map of binop type patterns to resulting type 
BINOP_EDE_TYPE_DICT : Dict[Tuple[EdeType, EdeType, BinopType], EdeType] = {
    (EdeInt, EdeInt, BinopType.ADD): EdeInt,
    (EdeInt, EdeInt, BinopType.SUB): EdeInt,
    (EdeInt, EdeInt, BinopType.MUL): EdeInt,
    (EdeInt, EdeInt, BinopType.DIV): EdeInt,
    (EdeString, EdeString, BinopType.ADD): EdeString,
}

# Map of binop type patterns to execution functions
BINOP_EXEC_FUNCS : Dict[Tuple[EdeType, EdeType, BinopType], Callable[[ExecValue, ExecValue, Position, ExecContext], ExecValue]] = {
    (EdeInt, EdeInt, BinopType.ADD): lambda left, right, _, __: ExecValue(left.to_int() + right.to_int()),
    (EdeInt, EdeInt, BinopType.SUB): lambda left, right, _, __: ExecValue(left.to_int() - right.to_int()),
    (EdeInt, EdeInt, BinopType.MUL): lambda left, right, _, __: ExecValue(left.to_int() * right.to_int()),
    (EdeInt, EdeInt, BinopType.DIV): lambda left, right, pos, _: ExecValue(left.to_int() // right.to_int() if right.to_int() != 0 else ExecException.DivisionByZero(pos)),
    (EdeString, EdeString, BinopType.ADD): lambda left, right, _, __: ExecValue(left.to_str() + right.to_str())
}

# Ensures there patterns exist in both maps
assert BINOP_EDE_TYPE_DICT.keys() == BINOP_EXEC_FUNCS.keys()

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
