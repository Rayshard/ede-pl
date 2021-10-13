from typing import Any, Callable, Dict, List, NamedTuple, Optional, Type
from ede_ast.ede_ast import Node
from ede_ast.ede_stmt import Block, VarDeclStmt
from ede_ast.ede_module import Module

class CFGNode(NamedTuple):
    '''Control flow graph node'''

    nexts: List[Node]
    ast_node: Node

class CFG:
    '''Control flow gragh'''

    def __init__(self) -> None:
        '''Creates a empty CFG'''

        self.__nodes: List[CFGNode] = []

    def append(self, node: CFGNode) -> int:
        '''
        Adds the node to the CFG and return the unique id
        for the node in the CFG.
        '''

        id = len(self.__nodes)
        self.__nodes.append(node)

        return id

    def to_dot(self) -> str:
        output = "digraph graphname {\n"
        to_visit = [0]
        visited = []

        while len(to_visit) != 0:
            cur_id = to_visit.pop()
            visited.append(cur_id)

            for next in self.__nodes[cur_id].nexts:
                next_id = next.get_cfg_id()
                output += f"\t{cur_id} -> {next_id}\n"

                if next_id not in visited:
                    to_visit.append(next_id)

        return output + "}"

    def __getitem__(self, id: int) -> CFGNode:
        return self.__nodes[id]

class CFGVisitor:
    '''Constructs a control flow graph.'''

    @staticmethod
    def visit(node: Node, cfg: CFG, next: Optional[Node]) -> None:
        node.set_cfg_id(cfg.append(CFGNode([], node)))
        VISITORS[type(node)](node, cfg, next)

def visit_fall_through_node(node: Node, cfg: CFG, next: Optional[Node]) -> None:
    if next is not None:
        cfg[node.get_cfg_id()].nexts.append(next)

def visit_Block(b: Block, cfg: CFG, next: Optional[Node]) -> None:
    if len(b.stmts) == 0:
        return
    
    cfg[b.get_cfg_id()].nexts.append(b.stmts[0])

    for i in range(len(b.stmts) - 1):
        CFGVisitor.visit(b.stmts[i], cfg, b.stmts[i + 1])

    CFGVisitor.visit(b.stmts[-1], cfg, None)  # Visit the last statement, making sure that it's next is None

def visit_Module(m: Module, cfg: CFG, next: Optional[Node]) -> None:
    if len(m.stmts) == 0:
        return
    
    cfg[m.get_cfg_id()].nexts.append(m.stmts[0])

    for i in range(len(m.stmts) - 1):
        CFGVisitor.visit(m.stmts[i], cfg, m.stmts[i + 1])

    CFGVisitor.visit(m.stmts[-1], cfg, None)  # Visit the last statement, making sure that it's next is None

VISITORS : Dict[Type[Any], Callable[[Any, CFG, Optional[Node]], None]] = {
    Module: visit_Module,
    VarDeclStmt: visit_fall_through_node,
}