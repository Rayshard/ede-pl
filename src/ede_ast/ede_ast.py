from abc import ABC, abstractmethod
from enum import Enum, auto
from typing import Optional, Union
from ede_utils import Position, Result, Success
from .ede_type import EdeType, Environment

ExecutionResult = Union[int, str, bool]

class NodeType(Enum):
    EXPR = auto()
    STMT = auto()

class Node(ABC):
    def __init__(self, pos: Position) -> None:
        self.position = pos
        self.__type : Optional[EdeType] = None

    def typecheck(self, env: Environment) -> Result[EdeType]:
        if self.__type is None:
            result = self._typecheck(env)
            if result.is_error():
                return result
            else:
                self.__type = result.get()

        return Success(self.__type)

    def get_type(self) -> EdeType:
        assert self.__type is not None, "Node has not been type checked!"
        return self.__type

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        pass

    @abstractmethod
    def get_node_type(self) -> NodeType:
        pass

    @abstractmethod
    def execute(self) -> ExecutionResult:
        pass

        