# EVM Specification

## Globals

## Command Line Arguments
Command line arguments are passed to the program as a pointer to a length-prefixed array in the heap. The array is prefixed with its size as an 8-byte unsigned integer. After that, each item is a pointer to a length-prefixed array of bytes that represent the string of each command line argument. These arrays are also prefixed with its size as an 8-byte unsigned integer. The initial pointer to the command line arguments is pushed onto the stack of the main thread as the second entry (right after the pointer to the globals). In this was, it can be used as the first argument passed to a main function, and therefore can be access using the instruction `PLOAD 0`.

## Instruction Set
- Arithmetic Instructions:
  - Integer: [IADD](), [ISUB](), [IMUL](), [IDIV]()
  - Floating Point:

### PUSH
- Syntax: `PUSH [value]`
  - `value` : an 8 byte hexidecimal
- Function: `STACK[SP:SP+8) = value; SP += 8;`
- Description: TODO
- Variants
  - `PUSHI [value]` where `value` is an 64-bit signed integer 
  - `PUSHD [value]` where `value` is an 64-bit floating point number

### MLOAD
- Syntax: `MLOAD [offset]`
  - `offset` : a 32-bit signed integer
- Function: `addr = POP(); PUSH(HEAP[addr + offset]);`
- Description: TODO

### MSTORE
- Syntax: `MSTORE [offset]`
  - `offset` : a 32-bit signed integer
- Function: `addr = POP(); value = POP(); HEAP[addr + offset:addr + offset + 8) = value;`
- Description: TODO