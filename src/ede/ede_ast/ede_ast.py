from abc import ABC, abstractmethod
from enum import Enum, auto
import json
from typing import Any, Dict, Optional
from ede_utils import Position, Result, Success
from interpreter import ExecContext, ExecValue
from .ede_typesystem import EdeType, Environment

class NodeType(Enum):
    '''Enumeration of AST node types'''

    EXPR = auto()
    STMT = auto()

class Node(ABC):
    '''AST node'''

    def __init__(self, pos: Position) -> None:
        '''Create an AST node'''

        self.position = pos
        self.__type : Optional[EdeType] = None

    def typecheck(self, env: Environment) -> Result[EdeType]:
        '''
        Type checks the node within the given environment and updates
        the node's stored ede type.
        '''

        result = self._typecheck(env)

        if result.is_error():
            self.__type = None
            return result
        else:
            self.__type = result.get()
            return Success(self.__type)

    def execute(self, ctx: ExecContext) -> ExecValue:
        '''
        Under the assumption of a successful type checking, simulates execution
        of the node in the given execution context.
        '''
        
        assert self.__type is not None
        return self._execute(ctx)

    def get_type(self) -> EdeType:
        '''Returns the ede type; node must have already been type checked.'''

        assert self.__type is not None, "Node has not been type checked!"
        return self.__type

    def execute_in(self, env: Environment, ctx: ExecContext) -> ExecValue:
        '''
        Typechecks the node in the given environment and under the assumption of 
        a success, simulates execution of the node in the given execution context.
        '''

        assert self.typecheck(env).is_success()
        return self.execute(ctx)

    def __str__(self) -> str:
        return json.dumps(self.to_json(), indent=4, sort_keys=False)

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        '''Protected version for self.typecheck to be overriden in child classes'''
        pass

    @abstractmethod
    def _execute(self, ctx: ExecContext) -> ExecValue:
        '''Protected version for self.execute to be overriden in child classes'''
        pass

    @abstractmethod
    def get_node_type(self) -> NodeType:
        '''Returns the NodeType'''
        pass

    @abstractmethod
    def to_json(self) -> Dict[str, Any]:
        '''Returns a json representation of the node'''
        pass        