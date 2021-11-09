#include "instructions.h"
#include <iostream>
#include "thread.h"
#include "vm.h"

namespace Instructions
{
    namespace Error
    {
        std::runtime_error CompilationNotImplemented(const vm_byte* _instr) { return std::runtime_error("Instruction compilation not implemented: " + ToString(_instr)); }
    }

    ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];
    ExecutionFunc SysCallExecutionFuncs[(size_t)SysCallCode::_COUNT];

    void NOOP(Thread* _thread) { return; }

    void CONVERT(Thread* _thread)
    {
        DataType from = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE), to = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE + DATA_TYPE_SIZE);
        auto value = _thread->PopStack();

        switch (from)
        {
        case DataType::I8: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_i8)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i8)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_i8)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i8)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_i8)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i8)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_i8)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i8)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_i8)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_i8)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::UI8: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_ui8)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui8)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_ui8)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui8)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_ui8)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui8)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_ui8)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui8)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_ui8)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_ui8)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::I16: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_i16)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i16)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_i16)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i16)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_i16)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i16)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_i16)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i16)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_i16)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_i16)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::UI16: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_ui16)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui16)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_ui16)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui16)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_ui16)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui16)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_ui16)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui16)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_ui16)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_ui16)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::I32: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_i32)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i32)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_i32)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i32)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_i32)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i32)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_i32)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i32)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_i32)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_i32)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::UI32: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_ui32)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui32)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_ui32)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui32)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_ui32)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui32)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_ui32)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui32)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_ui32)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_ui32)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::I64: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_i64)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i64)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_i64)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i64)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_i64)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i64)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_i64)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i64)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_i64)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_i64)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::UI64: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_ui64)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui64)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_ui64)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui64)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_ui64)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui64)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_ui64)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui64)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_ui64)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_ui64)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::F32: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_f32)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_f32)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_f32)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_f32)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_f32)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_f32)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_f32)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_f32)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_f32)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_f32)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        case DataType::F64: {
            switch (to)
            {
            case DataType::I8: _thread->PushStack(vm_i8(value.as_f64)); break;
            case DataType::UI8: _thread->PushStack(vm_ui8(value.as_f64)); break;
            case DataType::I16: _thread->PushStack(vm_i16(value.as_f64)); break;
            case DataType::UI16: _thread->PushStack(vm_ui16(value.as_f64)); break;
            case DataType::I32: _thread->PushStack(vm_i32(value.as_f64)); break;
            case DataType::UI32: _thread->PushStack(vm_ui32(value.as_f64)); break;
            case DataType::I64: _thread->PushStack(vm_i64(value.as_f64)); break;
            case DataType::UI64: _thread->PushStack(vm_ui64(value.as_f64)); break;
            case DataType::F32: _thread->PushStack(vm_f32(value.as_f64)); break;
            case DataType::F64: _thread->PushStack(vm_f64(value.as_f64)); break;
            default: assert(false && "Case not handled");
            }
        } break;
        default: assert(false && "Case not handled");
        }
    }

#pragma region Binops
    void ADD(Thread* _thread)
    {
        DataType dataType = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE);
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (dataType)
        {
        case DataType::I8: _thread->PushStack(Word(vm_i8(left.as_i8 + right.as_i8))); break;
        case DataType::UI8: _thread->PushStack(Word(vm_ui8(left.as_ui8 + right.as_ui8))); break;
        case DataType::I16: _thread->PushStack(Word(vm_i16(left.as_i16 + right.as_i16))); break;
        case DataType::UI16: _thread->PushStack(Word(vm_ui16(left.as_ui16 + right.as_ui16))); break;
        case DataType::I32: _thread->PushStack(Word(left.as_i32 + right.as_i32)); break;
        case DataType::UI32: _thread->PushStack(Word(left.as_ui32 + right.as_ui32)); break;
        case DataType::I64: _thread->PushStack(Word(left.as_i64 + right.as_i64)); break;
        case DataType::UI64: _thread->PushStack(Word(left.as_ui64 + right.as_ui64)); break;
        case DataType::F32: _thread->PushStack(Word(left.as_f32 + right.as_f32)); break;
        case DataType::F64: _thread->PushStack(Word(left.as_f64 + right.as_f64)); break;
        default: assert(false && "Case not handled");
        }
    }

    void SUB(Thread* _thread)
    {
        DataType dataType = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE);
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (dataType)
        {
        case DataType::I8: _thread->PushStack(Word(vm_i8(left.as_i8 - right.as_i8))); break;
        case DataType::UI8: _thread->PushStack(Word(vm_ui8(left.as_ui8 - right.as_ui8))); break;
        case DataType::I16: _thread->PushStack(Word(vm_i16(left.as_i16 - right.as_i16))); break;
        case DataType::UI16: _thread->PushStack(Word(vm_ui16(left.as_ui16 - right.as_ui16))); break;
        case DataType::I32: _thread->PushStack(Word(left.as_i32 - right.as_i32)); break;
        case DataType::UI32: _thread->PushStack(Word(left.as_ui32 - right.as_ui32)); break;
        case DataType::I64: _thread->PushStack(Word(left.as_i64 - right.as_i64)); break;
        case DataType::UI64: _thread->PushStack(Word(left.as_ui64 - right.as_ui64)); break;
        case DataType::F32: _thread->PushStack(Word(left.as_f32 - right.as_f32)); break;
        case DataType::F64: _thread->PushStack(Word(left.as_f64 - right.as_f64)); break;
        default: assert(false && "Case not handled");
        }
    }

    void MUL(Thread* _thread)
    {
        DataType dataType = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE);
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (dataType)
        {
        case DataType::I8: _thread->PushStack(Word(vm_i8(left.as_i8 * right.as_i8))); break;
        case DataType::UI8: _thread->PushStack(Word(vm_ui8(left.as_ui8 * right.as_ui8))); break;
        case DataType::I16: _thread->PushStack(Word(vm_i16(left.as_i16 * right.as_i16))); break;
        case DataType::UI16: _thread->PushStack(Word(vm_ui16(left.as_ui16 * right.as_ui16))); break;
        case DataType::I32: _thread->PushStack(Word(left.as_i32 * right.as_i32)); break;
        case DataType::UI32: _thread->PushStack(Word(left.as_ui32 * right.as_ui32)); break;
        case DataType::I64: _thread->PushStack(Word(left.as_i64 * right.as_i64)); break;
        case DataType::UI64: _thread->PushStack(Word(left.as_ui64 * right.as_ui64)); break;
        case DataType::F32: _thread->PushStack(Word(left.as_f32 * right.as_f32)); break;
        case DataType::F64: _thread->PushStack(Word(left.as_f64 * right.as_f64)); break;
        default: assert(false && "Case not handled");
        }
    }

    void DIV(Thread* _thread)
    {
        DataType dataType = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE);
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (dataType)
        {
        case DataType::I8: {
            if (right.as_i8 == vm_i8(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(vm_i8(left.as_i8 / right.as_i8)));
        } break;
        case DataType::UI8: {
            if (right.as_ui8 == vm_ui8(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(vm_ui8(left.as_ui8 / right.as_ui8)));
        } break;
        case DataType::I16: {
            if (right.as_i16 == vm_i16(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(vm_i16(left.as_i16 / right.as_i16)));
        } break;
        case DataType::UI16: {
            if (right.as_ui16 == vm_ui16(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(vm_ui16(left.as_ui16 / right.as_ui16)));
        } break;
        case DataType::I32: {
            if (right.as_i32 == vm_i32(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(left.as_i32 / right.as_i32));
        } break;
        case DataType::UI32: {
            if (right.as_ui32 == vm_ui32(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(left.as_ui32 / right.as_ui32));
        } break;
        case DataType::I64: {
            if (right.as_i64 == vm_i64(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(left.as_i64 / right.as_i64));
        } break;
        case DataType::UI64: {
            if (right.as_ui64 == vm_ui64(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(left.as_ui64 / right.as_ui64));
        } break;
        case DataType::F32: {
            if (right.as_f32 == vm_f32(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(left.as_f32 / right.as_f32));
        } break;
        case DataType::F64: {
            if (right.as_f64 == vm_f64(0))
                throw VMError::DIV_BY_ZERO();

            _thread->PushStack(Word(left.as_f64 / right.as_f64));
        } break;
        default: assert(false && "Case not handled");
        }
    }

    void EQ(Thread* _thread)
    {
        DataType dataType = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE);
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (dataType)
        {
        case DataType::I8: _thread->PushStack(Word(left.as_i8 == right.as_i8)); break;
        case DataType::UI8: _thread->PushStack(Word(left.as_ui8 == right.as_ui8)); break;
        case DataType::I16: _thread->PushStack(Word(left.as_i16 == right.as_i16)); break;
        case DataType::UI16: _thread->PushStack(Word(left.as_ui16 == right.as_ui16)); break;
        case DataType::I32: _thread->PushStack(Word(left.as_i32 == right.as_i32)); break;
        case DataType::UI32: _thread->PushStack(Word(left.as_ui32 == right.as_ui32)); break;
        case DataType::I64: _thread->PushStack(Word(left.as_i64 == right.as_i64)); break;
        case DataType::UI64: _thread->PushStack(Word(left.as_ui64 == right.as_ui64)); break;
        case DataType::F32: _thread->PushStack(Word(left.as_f32 == right.as_f32)); break;
        case DataType::F64: _thread->PushStack(Word(left.as_f64 == right.as_f64)); break;
        default: assert(false && "Case not handled");
        }
    }

    void NEQ(Thread* _thread)
    {
        DataType dataType = *(DataType*)(_thread->instrPtr + OP_CODE_SIZE);
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (dataType)
        {
        case DataType::I8: _thread->PushStack(Word(left.as_i8 != right.as_i8)); break;
        case DataType::UI8: _thread->PushStack(Word(left.as_ui8 != right.as_ui8)); break;
        case DataType::I16: _thread->PushStack(Word(left.as_i16 != right.as_i16)); break;
        case DataType::UI16: _thread->PushStack(Word(left.as_ui16 != right.as_ui16)); break;
        case DataType::I32: _thread->PushStack(Word(left.as_i32 != right.as_i32)); break;
        case DataType::UI32: _thread->PushStack(Word(left.as_ui32 != right.as_ui32)); break;
        case DataType::I64: _thread->PushStack(Word(left.as_i64 != right.as_i64)); break;
        case DataType::UI64: _thread->PushStack(Word(left.as_ui64 != right.as_ui64)); break;
        case DataType::F32: _thread->PushStack(Word(left.as_f32 != right.as_f32)); break;
        case DataType::F64: _thread->PushStack(Word(left.as_f64 != right.as_f64)); break;
        default: assert(false && "Case not handled");
        }
    }
#pragma endregion

#pragma region Branching
    void JUMP(Thread* _thread)
    {
        vm_byte* target = ((Word*)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        _thread->instrPtr = target - GetSize(OpCode::JUMP);
    }

    void JUMPNZ(Thread* _thread)
    {
        if (!_thread->PopStack().AsBool())
            return;

        vm_byte* target = ((Word*)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        _thread->instrPtr = target - GetSize(OpCode::JUMPNZ);
    }

    void JUMPZ(Thread* _thread)
    {
        if (_thread->PopStack().AsBool())
            return;

        vm_byte* target = ((Word*)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        _thread->instrPtr = target - GetSize(OpCode::JUMPZ);
    }

    void CALL(Thread* _thread)
    {
        vm_byte* target = ((Word*)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        vm_ui32 storage = *(vm_ui32*)(_thread->instrPtr + OP_CODE_SIZE + VM_PTR_SIZE);

        _thread->PushStack(_thread->instrPtr + GetSize(OpCode::CALL)); //Push return value
        _thread->PushFrame();                                          //Push current frame pointer and set the frame pointer for new frame
        _thread->OffsetSP(storage);                                    //Allocate space of stack for function local storage
        _thread->instrPtr = target - GetSize(OpCode::CALL);            //Jump to target
    }

    void RET(Thread* _thread)
    {
        _thread->PopFrame();                                                       //Clear current frame and restore previous frame pointer
        _thread->instrPtr = _thread->PopStack().as_ptr - GetSize(OpCode::RET); //Jump to instruction after most recent call
    }

    void RETV(Thread* _thread)
    {
        Word retValue = _thread->PopStack(); //Pop off return value
        RET(_thread);                              //Execute return
        _thread->PushStack(retValue);              //Push return value
    }
#pragma endregion

#pragma region Syscalls
    namespace SYSCALL
    {
        SysCallCode GetCode(const vm_byte* _instr) { return *(SysCallCode*)(_instr + OP_CODE_SIZE); }

        void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent)
        {
            switch (GetCode(_instr))
            {
            case SysCallCode::EXIT:
            {
                _stream << _indent << "pop rdi\n";
                _stream << _indent << "mov rax, 0x02000001\n";
                _stream << _indent << "syscall";
            } break;
            default: throw Error::CompilationNotImplemented(_instr);
            }
        }

        void ExecuteEXIT(Thread* _thread) { _thread->GetVM()->Quit(_thread->PopStack().as_i64); }

        void ExecutePRINTC(Thread* _thread) { _thread->GetVM()->GetStdOut() << _thread->PopStack().as_byte; }

        void ExecuteMALLOC(Thread* _thread) { _thread->PushStack(_thread->GetVM()->GetHeap().Alloc(_thread->PopStack().as_i64)); }

        void ExecuteFREE(Thread* _thread) { _thread->GetVM()->GetHeap().Free(_thread->PopStack().as_ptr); }

        void Execute(Thread* _thread)
        {
            vm_byte code = (vm_byte)GetCode(_thread->instrPtr);
            if (code >= (vm_byte)SysCallCode::_COUNT)
                throw VMError::UNKNOWN_SYSCALL_CODE(code);

            SysCallExecutionFuncs[code](_thread);
        }
    }
#pragma endregion

#pragma region Loads and Stores
    namespace PUSH
    {
        Word GetValue(const vm_byte* _instr) { return *(Word*)(_instr + OP_CODE_SIZE); }
        void Execute(Thread* _thread) { _thread->PushStack(PUSH::GetValue(_thread->instrPtr)); }

        void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent)
        {
            _stream << _indent << "mov rax, " << Hex(Instructions::PUSH::GetValue(_instr)) << "\n";
            _stream << _indent << "push rax";
        }
    }

    void POP(Thread* _thread) { _thread->PopStack(); }

    void SLOAD(Thread* _thread)
    {
        vm_i64 offset = *(vm_i64*)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(_thread->GetSP() + offset));
    }

    void SSTORE(Thread* _thread)
    {
        vm_i64 offset = *(vm_i64*)&_thread->instrPtr[OP_CODE_SIZE];
        auto value = _thread->PopStack();
        _thread->WriteStack(_thread->GetSP() + offset, value);
    }

    void LLOAD(Thread* _thread)
    {
        vm_ui32 idx = *(vm_ui32*)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() + idx * WORD_SIZE));
    }

    void LSTORE(Thread* _thread)
    {
        vm_ui32 idx = *(vm_ui32*)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->WriteStack(_thread->GetFP() + idx * WORD_SIZE, _thread->PopStack());
    }

    void PLOAD(Thread* _thread)
    {
        vm_ui32 idx = *(vm_ui32*)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() - WORD_SIZE * 2 - (idx + 1) * WORD_SIZE));
    }

    void PSTORE(Thread* _thread)
    {
        vm_ui32 idx = *(vm_ui32*)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->WriteStack(_thread->GetFP() - WORD_SIZE * 2 - (idx + 1) * WORD_SIZE, _thread->PopStack());
    }

    void GLOAD(Thread* _thread)
    {
        vm_ui64 idx = *(vm_ui64*)&_thread->instrPtr[OP_CODE_SIZE];
        vm_byte* addr = _thread->ReadStack<vm_byte*>(0) + idx * WORD_SIZE;

        if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
            throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

        _thread->PushStack(*(Word*)addr);
    }

    void GSTORE(Thread* _thread)
    {
        vm_ui64 idx = *(vm_ui64*)&_thread->instrPtr[OP_CODE_SIZE];
        vm_byte* addr = _thread->ReadStack<vm_byte*>(0) + idx * WORD_SIZE;
        Word value = _thread->PopStack();

        if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
            throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

        std::copy((vm_byte*)&value, (vm_byte*)&value + sizeof(value), addr);
    }

    void MLOAD(Thread* _thread)
    {
        vm_i64 offset = *(vm_i64*)&_thread->instrPtr[OP_CODE_SIZE];
        vm_byte* addr = _thread->PopStack().as_ptr + offset;

        if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
            throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

        _thread->PushStack(*(Word*)addr);
    }

    void MSTORE(Thread* _thread)
    {
        vm_i64 offset = *(vm_i64*)&_thread->instrPtr[OP_CODE_SIZE];
        vm_byte* addr = _thread->PopStack().as_ptr + offset;
        Word value = _thread->PopStack();

        if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
            throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

        std::copy((vm_byte*)&value, (vm_byte*)&value + sizeof(value), addr);
    }
#pragma endregion

    void Init()
    {
        ExecutionFuncs[(size_t)OpCode::NOOP] = &NOOP;
        ExecutionFuncs[(size_t)OpCode::PUSH] = &PUSH::Execute;
        ExecutionFuncs[(size_t)OpCode::POP] = &POP;
        ExecutionFuncs[(size_t)OpCode::ADD] = &ADD;
        ExecutionFuncs[(size_t)OpCode::SUB] = &SUB;
        ExecutionFuncs[(size_t)OpCode::MUL] = &MUL;
        ExecutionFuncs[(size_t)OpCode::DIV] = &DIV;
        ExecutionFuncs[(size_t)OpCode::JUMP] = &JUMP;
        ExecutionFuncs[(size_t)OpCode::JUMPZ] = &JUMPZ;
        ExecutionFuncs[(size_t)OpCode::JUMPNZ] = &JUMPNZ;
        ExecutionFuncs[(size_t)OpCode::SYSCALL] = &SYSCALL::Execute;
        ExecutionFuncs[(size_t)OpCode::EQ] = &EQ;
        ExecutionFuncs[(size_t)OpCode::NEQ] = &NEQ;
        ExecutionFuncs[(size_t)OpCode::SLOAD] = &SLOAD;
        ExecutionFuncs[(size_t)OpCode::SSTORE] = &SSTORE;
        ExecutionFuncs[(size_t)OpCode::LLOAD] = &LLOAD;
        ExecutionFuncs[(size_t)OpCode::LSTORE] = &LSTORE;
        ExecutionFuncs[(size_t)OpCode::GLOAD] = &GLOAD;
        ExecutionFuncs[(size_t)OpCode::GSTORE] = &GSTORE;
        ExecutionFuncs[(size_t)OpCode::PLOAD] = &PLOAD;
        ExecutionFuncs[(size_t)OpCode::PSTORE] = &PSTORE;
        ExecutionFuncs[(size_t)OpCode::MLOAD] = &MLOAD;
        ExecutionFuncs[(size_t)OpCode::MSTORE] = &MSTORE;
        ExecutionFuncs[(size_t)OpCode::CONVERT] = &CONVERT;
        ExecutionFuncs[(size_t)OpCode::CALL] = &CALL;
        ExecutionFuncs[(size_t)OpCode::RET] = &RET;
        ExecutionFuncs[(size_t)OpCode::RETV] = &RETV;

        SysCallExecutionFuncs[(size_t)SysCallCode::EXIT] = &SYSCALL::ExecuteEXIT;
        SysCallExecutionFuncs[(size_t)SysCallCode::PRINTC] = &SYSCALL::ExecutePRINTC;
        SysCallExecutionFuncs[(size_t)SysCallCode::MALLOC] = &SYSCALL::ExecuteMALLOC;
        SysCallExecutionFuncs[(size_t)SysCallCode::FREE] = &SYSCALL::ExecuteFREE;
    }

    std::string ToString(DataType _dt)
    {
        switch (_dt)
        {
        case DataType::I8: return "I8";
        case DataType::UI8: return "UI8";
        case DataType::I16: return "I16";
        case DataType::UI16: return "UI16";
        case DataType::I32: return "I32";
        case DataType::UI32: return "UI32";
        case DataType::I64: return "I64";
        case DataType::UI64: return "UI64";
        case DataType::F32: return "F32";
        case DataType::F64: return "F64";
        default: assert(false && "Case not handled");
        }

        return "";
    }

    std::string ToString(const vm_byte* _instr)
    {
        switch ((OpCode)*_instr)
        {
        case OpCode::NOOP: return "NOOP";
        case OpCode::CONVERT: return "CONVERT " + ToString((DataType)_instr[OP_CODE_SIZE]) + " " + ToString((DataType)_instr[OP_CODE_SIZE + DATA_TYPE_SIZE]);
        case OpCode::POP: return "POP";
        case OpCode::ADD: return "ADD " + ToString((DataType)_instr[OP_CODE_SIZE]);
        case OpCode::SUB: return "SUB " + ToString((DataType)_instr[OP_CODE_SIZE]);
        case OpCode::MUL: return "MUL " + ToString((DataType)_instr[OP_CODE_SIZE]);
        case OpCode::DIV: return "DIV " + ToString((DataType)_instr[OP_CODE_SIZE]);
        case OpCode::EQ: return "EQ " + ToString((DataType)_instr[OP_CODE_SIZE]);
        case OpCode::NEQ: return "NEQ " + ToString((DataType)_instr[OP_CODE_SIZE]);
        case OpCode::RET: return "RET";
        case OpCode::RETV: return "RETV";
        case OpCode::PUSH: return "PUSH " + Hex(PUSH::GetValue(_instr).as_ui64);
        case OpCode::JUMP: return "JUMP " + Hex(*(vm_ui64*)&_instr[OP_CODE_SIZE]);
        case OpCode::JUMPNZ: return "JUMPNZ " + Hex(*(vm_ui64*)&_instr[OP_CODE_SIZE]);
        case OpCode::JUMPZ: return "JUMPZ " + Hex(*(vm_ui64*)&_instr[OP_CODE_SIZE]);
        case OpCode::CALL: return "CALL " + Hex(*(vm_ui64*)&_instr[OP_CODE_SIZE]) + ", " + std::to_string(*(vm_ui32*)&_instr[OP_CODE_SIZE + VM_UI64_SIZE]);
        case OpCode::SYSCALL:
        {
            switch ((SysCallCode)_instr[1])
            {
            case SysCallCode::EXIT: return "SYSCALL EXIT";
            case SysCallCode::PRINTC: return "SYSCALL PRINTC";
            case SysCallCode::MALLOC: return "SYSCALL MALLOC";
            case SysCallCode::FREE: return "SYSCALL FREE";
            default: assert(false && "Case not handled");
            }
        } break;
        case OpCode::SLOAD: return "SLOAD " + std::to_string(*(vm_i64*)&_instr[1]);
        case OpCode::SSTORE: return "SSTORE " + std::to_string(*(vm_i64*)&_instr[1]);
        case OpCode::LLOAD: return "LLOAD " + std::to_string(*(vm_ui32*)&_instr[1]);
        case OpCode::LSTORE: return "LSTORE " + std::to_string(*(vm_ui32*)&_instr[1]);
        case OpCode::GLOAD: return "GLOAD " + std::to_string(*(vm_ui64*)&_instr[1]);
        case OpCode::GSTORE: return "GSTORE " + std::to_string(*(vm_ui64*)&_instr[1]);
        case OpCode::PLOAD: return "PLOAD " + std::to_string(*(vm_ui32*)&_instr[1]);
        case OpCode::PSTORE: return "PSTORE " + std::to_string(*(vm_ui32*)&_instr[1]);
        case OpCode::MLOAD: return "MLOAD " + std::to_string(*(vm_i64*)&_instr[1]);
        case OpCode::MSTORE: return "MSTORE " + std::to_string(*(vm_i64*)&_instr[1]);
        default: assert(false && "Case not handled");
        }

        return "";
    }

    void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent)
    {
        switch ((OpCode)*_instr)
        {
        case OpCode::PUSH: Instructions::PUSH::ToNASM(_instr, _stream, "\t\t"); break;
        case OpCode::SYSCALL: Instructions::SYSCALL::ToNASM(_instr, _stream, "\t\t"); break;
        default:
        throw Error::CompilationNotImplemented(_instr);
        }
    }
}