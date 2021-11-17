# EVM Bytecode

### Instructions

#### Notation
- `STACK[I]`: the `I`th byte of  

| Opcode | Operands    | Description                                                                                                                                              |
| :----- | :---------- | :------------------------------------------------------------------------------------------------------------------------------------------------------- |
| NOOP   |             | No operation                                                                                                                                             |
| POP    |             | Pops one word off of the the stack                                                                                                                       |
| DUP    |             | `PUSH(STACK[SP])`                                                                                                                                        |
| ADD    | `DATA_TYPE` | 1) `left = POP() as DATA_TYPE`<br>2) `right = POP() as DATA_TYPE`<br>3) `PUSH(left + right)`                                                             |
| SUB    | `DATA_TYPE` | 1) `left = POP() as DATA_TYPE`<br>2) `right = POP() as DATA_TYPE`<br>3) `PUSH(left - right)`                                                             |
| MUL    | `DATA_TYPE` | 1) `left = POP() as DATA_TYPE`<br>2) `right = POP() as DATA_TYPE`<br>3) `PUSH(left * right)`                                                             |
| DIV    | `DATA_TYPE` | 1) `left = POP() as DATA_TYPE`<br>2) `right = POP() as DATA_TYPE`<br>3) `if right != 0 as DATA_TYPE then PUSH(left / right) else throw DIVISION_BY_ZERO` |
| EQ     | `DATA_TYPE` | 1) `left = POP() as DATA_TYPE`<br>2) `right = POP() as DATA_TYPE`<br>3) `PUSH(left == right)`                                                            |
| NEQ    | `DATA_TYPE` | 1) `left = POP() as DATA_TYPE`<br>2) `right = POP() as DATA_TYPE`<br>3) `PUSH(left != right)`                                                            |
| SLOAD  | `OFFSET`    | `PUSH(STACK[SP + OFFSET])`                                                                                                                               |
| SSTORE | `OFFSET`    | `STACK[SP + OFFSET] = POP()`                                                                                                                             |
| MLOAD  | `OFFSET`    | 1) `addr = POP()`<br>2) `PUSH(MEMORY[addr + OFFSET])`                                                                                                    |
| MSTORE | `OFFSET`    | 1) `addr = POP()`<br>2) `MEMORY[addr + OFFSET] = POP()`                                                                                                  |
| MALLOC | | 1) `size = POP() as UI64`<br>2) Allocates `size` bytes of memory and pushes the start address of that memory onto the stack