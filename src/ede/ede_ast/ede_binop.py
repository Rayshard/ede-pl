from enum import Enum, auto
from typing import Any, Callable, Dict, Optional, Tuple, cast
from ede_utils import Position, Result, Success, Error, ErrorType
from .ede_typesystem import EdeInt, EdeString, EdeType, Environment, TypeCheckError
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

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        # typecheck LHS; return if fails
        left_type = self.left.typecheck(env)
        if left_type.is_error():
            return left_type

        # typecheck RHS; return if fails
        right_type = self.right.typecheck(env)
        if right_type.is_error():
            return right_type

        # set pattern and associated type; fail if pattern is not defined above
        # TODO: when we implement user defined types and inheritance and eq operator overloading, we'll have to update this
        self.type_pattern = (left_type.get(), right_type.get(), self.op)
        
        if self.op == BinopType.ASSIGN:
            return left_type if left_type.get() == right_type.get() else TypeCheckError.InvalidAssignment(left_type.get(), right_type.get(), self.position)
        elif self.type_pattern in BINOP_EDE_TYPE_DICT:
            return Success(BINOP_EDE_TYPE_DICT[self.type_pattern])
        else:
            return TypeCheckError_InvalidBinop(self.op, left_type.get(), right_type.get(), self.position)

    def _execute(self, ctx: ExecContext) -> Optional[ExecValue]:
        if self.op == BinopType.ASSIGN:
            id = cast(IdentifierExpr, self.left).id

            # execute RHS; return if exception
            right_res = cast(ExecValue, self.right.execute(ctx))
            if right_res.is_exception():
                return right_res

            # set the value of the id to the value of the expression
            ctx.set(id, right_res, self.position)
            return right_res
        else:
            # execute LHS; return if exception
            left_res = cast(ExecValue, self.left.execute(ctx))
            if left_res.is_exception():
                return left_res

            # execute RHS; return if exception
            right_res = cast(ExecValue, self.right.execute(ctx))
            if right_res.is_exception():
                return right_res

            # execute function associated with pattern
            return BINOP_EXEC_FUNCS[cast(Tuple[EdeType, EdeType, BinopType], self.type_pattern)](left_res, right_res, self.position, ctx)

    def _to_json(self) -> Dict[str, Any]:
        return {
            "left": self.left.to_json(),
            "right": self.right.to_json(),
            "op": str(self.op),
        }