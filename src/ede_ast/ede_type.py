from enum import Enum, auto
from typing import Dict, Optional

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