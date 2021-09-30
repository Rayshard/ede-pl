from enum import Enum, auto
from typing import Dict, Optional
from ede_utils import Error, ErrorType, Position, Result, Success

class TypeCheckError:
    @staticmethod
    def InvalidAssignment(ltype: 'EdeType', rtype: 'EdeType', pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_INVALID_ASSIGN, pos, f"Cannot assign {rtype} to {ltype}")

    @staticmethod
    def UnknownID(id: str, pos: Position) -> Error:
        return Error(ErrorType.TYPECHECKING_UNKNOWN_ID, pos, f"Unknown id \"{id}\"")

class EdeType(Enum):
    NULL = auto()
    INT = auto()
    STR = auto()
    CHAR = auto()
    BOOL = auto()

class Environment:
    def __init__(self, parent: Optional['Environment'] = None) -> None:
        self.parent : Optional[Environment] = parent
        self.variables : Dict[str, EdeType] = {}

    def get(self, id: str, pos: Position) -> Result[EdeType]:
        return Success(self.variables[id]) if id in self.variables else TypeCheckError.UnknownID(id, pos)

    def declare(self, id: str, type: EdeType, pos: Position) -> Optional[TypeCheckError]:
        # TODO: throw error if id already exists
        self.variables[id] = type
        return None