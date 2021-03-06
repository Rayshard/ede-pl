# EDE Programming Language


## Installation
- Naviagte to the project's root directory: `ede-pl`
- Poetry or Pip
    - Pip
        - Create a virtual environment: ```python -m venv .venv```
        - Activate the virtual environment:
            - Windows: ```.\.venv\Scripts\activate```
            - Unix/macOS: ```source .venv/bin/activate```
        - Run: ```pip install .```
    - Poetry
        - To create the virtual environment: ```poetry env use 3.10.0```
        - To spawn the shell: ```poetry shell```
        - To install the package: ```poetry install```
- Run: ```evm-build``` and check that no errors are produced (Requires that ```g++``` is installed globally)
- Run: ```ede --help``` and check that no errors are produced

## Syntax
### Variables
```
// Variable Declaration
var myVar : int = 10;
```

### Objects
```
def Person = object {
    name: string,
    age: int
}
```

### Enums
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
### Function
```
def myfunc = function(a: int, b: string) -> bool {
    return a == int(b);
}
```