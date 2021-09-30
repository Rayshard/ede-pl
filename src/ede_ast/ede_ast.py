from abc import ABC, abstractmethod
from enum import Enum, auto
from typing import Dict, NamedTuple, Optional, Union
from ede_utils import Position, Result, Success
from .ede_type import EdeType, Environment

class ExecExceptionType(Enum):
    DIV_BY_ZERO = auto()

class ExecException(NamedTuple):
    type: ExecExceptionType
    name: str
    pos: Position
    msg: str

    @staticmethod
    def DivisionByZero(pos: Position) -> 'ExecException':
        return ExecException(ExecExceptionType.DIV_BY_ZERO, 'Division By Zero', pos, "Unable to divide by 0")

ExecValue = Union[int, str, bool]
class TypedExecValue:
    def __init__(self, type: EdeType, value: ExecValue) -> None:
        self.type = type
        self.value = value

    def __eq__(self, o: object) -> bool:
        if isinstance(o, TypedExecValue):
            return o.type == self.type and o.value == self.value

        return False

    def __str__(self) -> str:
        return f"Value: {self.value}, Type: {self.type.name}"

ExecResult = Union[TypedExecValue, ExecException]

class ExecContext:
    def __init__(self, parent: Optional['ExecContext'] = None) -> None:
        self.parent : Optional[ExecContext] = parent
        self.variables : Dict[str, TypedExecValue] = {} 

    def get(self, id: str) -> TypedExecValue:
        return self.variables[id]

    def set(self, id: str, value: TypedExecValue, pos: Position):
        self.variables[id] = value

class NodeType(Enum):
    EXPR = auto()
    STMT = auto()

class Node(ABC):
    def __init__(self, pos: Position) -> None:
        self.position = pos
        self.__type : Optional[EdeType] = None

    def typecheck(self, env: Environment) -> Result[EdeType]:
        result = self._typecheck(env)

        if result.is_error():
            self.__type = None
            return result
        else:
            self.__type = result.get()
            return Success(self.__type)

    def get_type(self) -> EdeType:
        assert self.__type is not None, "Node has not been type checked!"
        return self.__type

    def execute_in(self, env: Environment, ctx: ExecContext) -> ExecResult:
        assert self.typecheck(env).is_success()
        return self.execute(ctx)

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        pass

    @abstractmethod
    def get_node_type(self) -> NodeType:
        pass

    @abstractmethod
    def execute(self, ctx: ExecContext) -> ExecResult:
        pass

        