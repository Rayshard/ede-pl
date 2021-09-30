from abc import ABC, abstractmethod
from enum import Enum, auto
from typing import Dict, NamedTuple, Optional, Union
from ede_utils import Position, Result, Success
from .ede_type import EdeType, Environment

class ExecExceptionType(Enum):
    '''Enumeration of execution exception types'''

    DIV_BY_ZERO = auto()

class ExecException(NamedTuple):
    '''Execution Exception'''

    type: ExecExceptionType
    name: str
    pos: Position
    msg: str

    @staticmethod
    def DivisionByZero(pos: Position) -> 'ExecException':
        return ExecException(ExecExceptionType.DIV_BY_ZERO, 'Division By Zero', pos, "Unable to divide by 0")

ExecValue = Union[int, str, bool]  # Possible data values returned on execution of an AST node
class TypedExecValue:
    '''Typed Execution Value'''

    def __init__(self, type: EdeType, value: ExecValue) -> None:
        '''Create a typed execution value'''

        self.type = type
        self.value = value

    def __eq__(self, o: object) -> bool:
        if isinstance(o, TypedExecValue):
            return o.type == self.type and o.value == self.value

        return False

    def __str__(self) -> str:
        return f"Value: {self.value}, Type: {self.type.name}"

ExecResult = Union[TypedExecValue, ExecException]  # Possible values return on execution of an AST node

class ExecContext:
    '''Execution Context: a mutale environment that holds the state of a program during AST execution'''

    def __init__(self, parent: Optional['ExecContext'] = None) -> None:
        '''Create an execution environment'''

        self.parent : Optional[ExecContext] = parent
        self.variables : Dict[str, TypedExecValue] = {} 

    def get(self, id: str) -> TypedExecValue:
        '''Returns the typed value of a entry. Existence of the entry is assumed.'''
        
        return self.variables[id]

    def set(self, id: str, value: TypedExecValue, pos: Position):
        '''Sets the typed value for the given id and creates the id if not already in the context.'''
        
        self.variables[id] = value

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

    def execute(self, ctx: ExecContext) -> ExecResult:
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

    def execute_in(self, env: Environment, ctx: ExecContext) -> ExecResult:
        '''
        Typechecks the node in the given environment and under the assumption of 
        a success, simulates execution of the node in the given execution context.
        '''

        assert self.typecheck(env).is_success()
        return self.execute(ctx)

    @abstractmethod
    def _typecheck(self, env: Environment) -> Result[EdeType]:
        '''Protected version for self.typecheck to be overriden in child classes'''
        pass

    @abstractmethod
    def _execute(self, ctx: ExecContext) -> ExecResult:
        '''Protected version for self.execute to be overriden in child classes'''
        pass

    @abstractmethod
    def get_node_type(self) -> NodeType:
        '''Returns the NodeType'''
        pass

    @abstractmethod
    def to_string(self, indent:int) -> str:
        '''Returns a string representation of the node'''
        pass        