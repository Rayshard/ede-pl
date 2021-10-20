from typing import List
from .ede_ast import Node, NodeType
from .ede_typesystem import EdePrimitive
from ..ede_utils import Position

class TypeSymbol(Node):
    '''AST type symbol node'''

    def __init__(self, pos: Position) -> None:
        '''Creates an AST type symbol node'''

        super().__init__(pos)

    def get_node_type(self) -> NodeType:
        return NodeType.TYPE_SYMBOL

class PrimitiveTypeSymbol(TypeSymbol):
    '''Representation of an primitive type symbol'''

    def __init__(self, primitive: EdePrimitive, pos: Position) -> None:
        '''Create primitive type symbol'''
        
        super().__init__(pos)
        self.primitive = primitive

    def __str__(self) -> str:
        return str(self.primitive)

class NameTypeSymbol(TypeSymbol):
    '''Representation of an name type symbol'''

    def __init__(self, name: str, pos: Position) -> None:
        '''Create name type symbol'''
        
        super().__init__(pos)
        self.name = name

    def __str__(self) -> str:
        return self.name

class ArrayTypeSymbol(TypeSymbol):
    '''Representation of an array type symbol'''

    def __init__(self, inner: TypeSymbol, pos: Position) -> None:
        '''Create array type symbol'''
        
        super().__init__(pos)
        self.inner = inner

    def __str__(self) -> str:
        return f"[{self.inner}]"

class TupleTypeSymbol(TypeSymbol):
    '''Representation of a tuple type symbol'''

    def __init__(self, inners: List[TypeSymbol], pos: Position) -> None:
        '''Create tuple type symbol'''
        
        assert len(inners) > 1, 'Tuple type symbols must have at least 2 inner type symbols'
        super().__init__(pos)
        self.inners = inners

    def __str__(self) -> str:
        return f"({','.join([str(inner) for inner in self.inners])})"
