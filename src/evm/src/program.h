#pragma once
#include "evm.h"
#include <vector>

#pragma pack(push, 1) //This pragma ensures that the structed is packed and has no padding
struct ProgramHeader
{
    vm_ui64 numGlobals = 0; //The number of globals used in the program
    vm_ui64 entryPoint = 0; //The byte offset into the instructions data at which to start execution
};
#pragma pack(pop)

class Program
{
    ProgramHeader header;
    Memory code;
public:
    Program();
    Program(Program&& _p) noexcept;

    void ValidateAndInit();

    vm_byte* GetEntryPtr();
    const ProgramHeader& GetHeader() const { return header; }
    const Memory& GetCode() const { return code; };

    template <class T>
    void Insert(T _value) { code.insert(code.end(), (vm_byte*)&_value, (vm_byte*)&_value + sizeof(_value)); }

    template <typename Arg1, typename... Rest>
    void Insert(Arg1 _arg1, Rest const &..._rest)
    {
        Insert(_arg1);
        Insert(_rest...);
    }

    Program& operator=(Program&& _p) noexcept
    {
        if (this == &_p)
            return *this;

        header = _p.header;
        code = std::move(_p.code);
        return *this;
    }

    static Program FromFile(const std::string& _filePath);
    static Program FromStream(std::istream& _stream);
    static Program FromString(const std::string& _string);

    template <typename Arg1, typename... Rest>
    static Program FromCode(Arg1 _arg1, Rest const &..._rest)
    {
        Program program = Program();
        program.Insert(_arg1, _rest...);
        program.ValidateAndInit();
        return std::move(program);
    }
};