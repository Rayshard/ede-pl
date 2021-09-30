from abc import abstractmethod
from enum import Enum, auto
from typing import Callable, Dict, Optional, Tuple, cast
from ede_utils import Position, Result, Success, Error, ErrorType
from .ede_type import EdeType, Environment
from .ede_ast import ExecutionResult, Node, NodeType

class ExprType(Enum):
    LITERAL = auto()
    BINOP = auto()
    UNOP = auto()

class Expression(Node):
    def __init__(self, pos: Position) -> None:
        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.EXPR

    @abstractmethod
    def get_expr_type(self) -> ExprType:
        pass

    @abstractmethod
    def execute(self) -> ExecutionResult:
        pass

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        pass

class BinopType(Enum):
    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()

BINOP_EDE_TYPE_DICT : Dict[Tuple[EdeType, EdeType, BinopType], EdeType] = {
    (EdeType.INT, EdeType.INT, BinopType.ADD): EdeType.INT,
    (EdeType.INT, EdeType.INT, BinopType.SUB): EdeType.INT,
    (EdeType.INT, EdeType.INT, BinopType.MUL): EdeType.INT,
    (EdeType.INT, EdeType.INT, BinopType.DIV): EdeType.INT,
    (EdeType.STR, EdeType.STR, BinopType.ADD): EdeType.STR,
}

BINOP_EXEC_FUNCS : Dict[Tuple[EdeType, EdeType, BinopType], Callable[[ExecutionResult, ExecutionResult], ExecutionResult]] = {
    (EdeType.INT, EdeType.INT, BinopType.ADD): lambda left, right: int(left) + int(right),
    (EdeType.INT, EdeType.INT, BinopType.SUB): lambda left, right: int(left) - int(right),
    (EdeType.INT, EdeType.INT, BinopType.MUL): lambda left, right: int(left) * int(right),
    (EdeType.INT, EdeType.INT, BinopType.DIV): lambda left, right: int(left) // int(right),
    (EdeType.STR, EdeType.STR, BinopType.ADD): lambda left, right: str(left) + str(right),
}

assert BINOP_EDE_TYPE_DICT.keys() == BINOP_EXEC_FUNCS.keys()

class BinopExpr(Expression):
    def __init__(self, pos: Position, left: Expression, right: Expression, op: BinopType) -> None:
        super().__init__(pos)
        self.left = left
        self.right = right
        self.op = op
        self.type_tuple : Optional[Tuple[EdeType, EdeType, BinopType]] = None

    def get_expr_type(self) -> ExprType:
        return ExprType.BINOP

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        left_type = self.left.typecheck(env)
        if left_type.is_error():
            return cast(Error, left_type)

        right_type = self.right.typecheck(env)
        if right_type.is_error():
            return cast(Error, right_type)

        self.type_tuple = (left_type.get(), right_type.get(), self.op)
        if self.type_tuple in BINOP_EDE_TYPE_DICT:
            return Success(BINOP_EDE_TYPE_DICT[self.type_tuple])
        else:
            return Error(ErrorType.TYPECHECKNG_INVALID_OP, self.position, f"Cannot perform {self.op} on {left_type.get()} and {right_type.get()}")

    def execute(self) -> ExecutionResult:
        left_res, right_res = self.left.execute(), self.right.execute()
        return BINOP_EXEC_FUNCS[cast(Tuple[EdeType, EdeType, BinopType], self.type_tuple)](left_res, right_res)
