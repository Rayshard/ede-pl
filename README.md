# EDE Programming Language


## Activating the Virtual Environment (Windows)
- Ensure you're running the command line in admin mode: ```Set-ExecutionPolicy Unrestricted -Scope Process```
- Activate the virtual environment: ```.\venv\Scripts\activate```

## Syntax
# Variable
```
// Variable Declaration
var myVar : int = 10;
```

# Objects
```
def Person = object {

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