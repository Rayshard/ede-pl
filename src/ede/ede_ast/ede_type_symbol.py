from typing import Any, Dict, List, cast
from .ede_ast import Node, NodeType
from interpreter import ExecContext, ExecValue
from .ede_typesystem import EdeArray, EdePrimitive, EdeRecord, EdeTuple, EdeType, EnvEntryType, Environment, TypeCheckError
from ede_utils import Error, Position, Result, Success

class TypeSymbol(Node):
    '''AST type symbol node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST type symbol node'''

        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.TYPE_SYMBOL

    def _execute(self, ctx: ExecContext) -> ExecValue:
        assert False, 'Type symbols are not executable'
    
    def to_json(self) -> Dict[str, Any]:
        return {"repr": str(self)}

class PrimitiveTypeSymbol(TypeSymbol):
    '''Representation of an primitive type symbol'''

    def __init__(self, primitive: EdePrimitive, pos: Position) -> None:
        '''Create primitive type symbol'''
        
        super().__init__(pos)
        self.primitive = primitive

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        return Success(self.primitive)

    def __str__(self) -> str:
        return self.primitive.get_type().get_type_symbol()

class NameTypeSymbol(TypeSymbol):
    '''Representation of an name type symbol'''

    def __init__(self, name: str, pos: Position) -> None:
        '''Create name type symbol'''
        
        super().__init__(pos)
        self.name = name

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        get_res = env.get(self.name, self.position, True)
        if get_res.is_error():
            return cast(Error, get_res)
        elif get_res.get().type != EnvEntryType.TYPENAME:
            return TypeCheckError.UnresolvableTypeName(self.name, self.position)
            
        return Success(get_res.get().ede_type)

    def __str__(self) -> str:
        return self.name

class ArrayTypeSymbol(TypeSymbol):
    '''Representation of an array type symbol'''

    def __init__(self, inner: TypeSymbol, pos: Position) -> None:
        '''Create array type symbol'''
        
        super().__init__(pos)
        self.inner = inner

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        inner_res = self.inner.typecheck(env)
        if inner_res.is_error():
            return inner_res
            
        return Success(EdeArray(inner_res.get()))

    def __str__(self) -> str:
        return f"[{str(self.inner)}]"

class TupleTypeSymbol(TypeSymbol):
    '''Representation of a tuple type symbol'''

    def __init__(self, inners: List[TypeSymbol], pos: Position) -> None:
        '''Create tuple type symbol'''
        
        assert len(inners) > 1, 'Tuple type symbols must have at least 2 inner type symbols'
        super().__init__(pos)
        self.inners = inners

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        inners : List[EdeType] = []

        for inner in self.inners:
            inner_res = inner.typecheck(env)
            if inner_res.is_error():
                return inner_res
        
            inners.append(inner_res.get())
            
        return Success(EdeTuple(inners))

    def __str__(self) -> str:
        return f"({','.join([str(inner) for inner in self.inners])})"

class RecordTypeSymbol(TypeSymbol):
    '''Representation of a record type symbol'''

    def __init__(self, items: Dict[str, TypeSymbol], pos: Position) -> None:
        '''Create record type symbol'''
        
        assert len(items) > 0, 'Record type symbols must have at least 1 item'
        super().__init__(pos)
        self.items = items

    def _typecheck(self, env: Environment) -> Result[EdeType]:
        items : Dict[str, EdeType] = {}

        for name, item in self.items.items():
            item_res = item.typecheck(env)
            if item_res.is_error():
                return item_res
        
            items[name] = item_res.get()
            
        return Success(EdeRecord(items))

    def __str__(self) -> str:
        return "{" + ','.join([f"{name}:{str(type_symbol)}" for name, type_symbol in self.items.items()]) + "}"
