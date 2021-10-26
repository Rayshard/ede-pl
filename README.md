# EDE Programming Language


## Installation
- Activate the virtual environment: ```.\venv\Scripts\activate```
- From the project's root directory, run: ```pip install .```
- Run: ```evm-build``` and check that no errors are produced (Requires that ```g++``` is installed globally)
- Run: ```ede --help``` and check that no errors are produced

## Syntax
# Variable
```
// Variable Declaration
var myVar : int = 10;
```

# Objects
```
def Person = object {
    name: string,
    age: int
}
```

# Enums
```
def RainbowColor = enum {
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    INDIGO,
    VIOLET
}
```
# Function
```
def myfunc = function(a: int, b: string) -> bool {
    return a == int(b);
}
```