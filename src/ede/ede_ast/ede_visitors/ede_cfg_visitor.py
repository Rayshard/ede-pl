import pydot # type: ignore
from typing import Any, Callable, Dict, List, NamedTuple, Optional, Type
from ede_ast.ede_ast import Node
from ede_ast.ede_stmt import Block, IfElseStmt, ReturnStmt
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

    def to_dot(self, graph_name: str = "CFG") -> pydot.Graph:
        graph = pydot.Dot(graph_name, graph_type='digraph', bgcolor='white')

        # Add all the ast nodes to the graph
        for node in self.__nodes:
            ast_node = node.ast_node
            graph.add_node(pydot.Node(str(ast_node.get_cfg_id()), label=f"{ast_node} : {ast_node.position}")) # type: ignore

        graph.add_node(pydot.Node("-1", label="START")) # type: ignore
        graph.add_node(pydot.Node(str(len(self.__nodes)), label="END")) # type: ignore

        # Add the edges between the nodes
        to_visit = [0]
        visited = []

        while len(to_visit) != 0:
            from_id = to_visit.pop()
            visited.append(from_id)
            
            for next in self.__nodes[from_id].nexts:
                to_id = next.get_cfg_id()
                graph.add_edge(pydot.Edge(str(from_id), str(to_id), color="black")) # type: ignore

                if to_id not in visited:
                    to_visit.append(to_id)

        graph.add_edge(pydot.Edge("-1", "0", color="black")) # type: ignore
        graph.add_edge(pydot.Edge(str(len(self.__nodes) - 1), str(len(self.__nodes)), color="black")) # type: ignore
        return graph

    def get_terminals(self) -> List[Node]:
        return [node.ast_node for node in self.__nodes if len(node.nexts) == 0]

    def __getitem__(self, id: int) -> CFGNode:
        return self.__nodes[id]

class CFGVisitor:
    '''Constructs a control flow graph.'''

    @staticmethod
    def visit(node: Node, cfg: CFG, next: Optional[Node]) -> None:
        if node.get_cfg_id() == -1:
            node.set_cfg_id(cfg.append(CFGNode([], node)))

        if type(node) not in VISITORS:
            visit_fall_through_node(node, cfg, next)
        else:
            VISITORS[type(node)](node, cfg, next)

def visit_fall_through_node(node: Node, cfg: CFG, next: Optional[Node]) -> None:
    if next is not None:
        cfg[node.get_cfg_id()].nexts.append(next)

def visit_Return(r: ReturnStmt, cfg: CFG, next: Optional[Node]) -> None:
    if next is not None:
        raise Exception("Cannot have node following a return statement!")

def visit_Block(b: Block, cfg: CFG, next: Optional[Node]) -> None:
    cfg_node = cfg[b.get_cfg_id()]
    
    if len(b.stmts) != 0:
        cfg_node.nexts.append(b.stmts[0])

        for i in range(len(b.stmts) - 1):
            CFGVisitor.visit(b.stmts[i], cfg, b.stmts[i + 1])

        CFGVisitor.visit(b.stmts[-1], cfg, next)
    elif next is not None:
        cfg_node.nexts.append(next)

def visit_Module(m: Module, cfg: CFG, next: Optional[Node]) -> None:
    cfg_node = cfg[m.get_cfg_id()]
    
    if len(m.stmts) != 0:
        cfg_node.nexts.append(m.stmts[0])

        for i in range(len(m.stmts) - 1):
            CFGVisitor.visit(m.stmts[i], cfg, m.stmts[i + 1])

        CFGVisitor.visit(m.stmts[-1], cfg, next)
    elif next is not None:
        cfg_node.nexts.append(next)

def visit_IfElseStmt(ie: IfElseStmt, cfg: CFG, next: Optional[Node]) -> None:
    cfg_node = cfg[ie.get_cfg_id()]
    cfg_node.nexts.append(ie.condition)

    CFGVisitor.visit(ie.condition, cfg, ie.thenClause)
    CFGVisitor.visit(ie.thenClause, cfg, next)

    if ie.elseClause is not None:
        CFGVisitor.visit(ie.condition, cfg, ie.elseClause)
        CFGVisitor.visit(ie.elseClause, cfg, next)
    else:
        CFGVisitor.visit(ie.condition, cfg, next)

VISITORS : Dict[Type[Any], Callable[[Any, CFG, Optional[Node]], None]] = {
    Module: visit_Module,
    Block: visit_Block,
    IfElseStmt: visit_IfElseStmt,
    ReturnStmt: visit_Return,
}