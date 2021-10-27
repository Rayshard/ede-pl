#pragma once
#include "evm.h"
#include <vector>

typedef std::vector<vm_byte> Program;

namespace Instructions
{
    void ValidateAndInitProgram(Program &_prog);
    Program ParseFile(const std::string& _filePath);
    Program ParseStream(std::istream& _stream);

    template <class T>
    void Insert(Program &_prog, T _value) { _prog.insert(_prog.end(), (vm_byte *)&_value, (vm_byte *)&_value + sizeof(_value)); }

    template <typename Arg1, typename... Rest>
    void Insert(Program &_prog, Arg1 _arg1, Rest const &..._rest)
    {
        Insert(_prog, _arg1);
        Insert(_prog, _rest...);
    }

    template <typename Arg1, typename... Rest>
    Program CreateProgram(Arg1 _arg1, Rest const &..._rest)
    {
        Program program = Program();
        Insert(program, _arg1);
        Insert(program, _rest...);
        ValidateAndInitProgram(program);
        return program;
    }
}