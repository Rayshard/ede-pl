#pragma once
#include "evm.h"
#include <vector>

typedef std::vector<byte> Program;

namespace Instructions
{
    template <class type>
    void Insert(Program &_prog, type _value) { _prog.insert(_prog.end(), (char *)&_value, (char *)&_value + sizeof(_value)); }

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
        return program;
    }

    Program ParseEdeASM(const std::string& _filePath);
}