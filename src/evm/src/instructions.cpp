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

    void Execute(const NOOP* _instr, Thread* _thread) { return; }

    void Execute(const CONVERT* _instr, Thread* _thread)
    {
        auto value = _thread->PopStack();

        switch (_instr->from)
        {
        case DataType::I8: {
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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
            switch (_instr->to)
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

    void Execute(const SYSCALL* _instr, Thread* _thread)
    {
        if ((vm_byte)_instr->code >= (vm_byte)SysCallCode::_COUNT)
            throw VMError::UNKNOWN_SYSCALL_CODE((vm_byte)_instr->code);

        switch (_instr->code)
        {
        case SysCallCode::EXIT: { _thread->GetVM()->Quit(_thread->PopStack().as_i64); } break;
        case SysCallCode::PRINTC: { _thread->GetVM()->GetStdOut() << _thread->PopStack().as_byte; } break;
        case SysCallCode::MALLOC: { _thread->PushStack(_thread->GetVM()->GetHeap().Alloc(_thread->PopStack().as_ui64)); } break;
        case SysCallCode::FREE: { _thread->GetVM()->GetHeap().Free(_thread->PopStack().as_ptr); } break;        
        default: assert(false && "Case not handled");
        }
    }

    void Execute(const ADD* _instr, Thread* _thread)
    {
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (_instr->type)
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

    void Execute(const SUB* _instr, Thread* _thread)
    {
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (_instr->type)
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

    void Execute(const MUL* _instr, Thread* _thread)
    {
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (_instr->type)
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

    void Execute(const DIV* _instr, Thread* _thread)
    {
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (_instr->type)
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

    void Execute(const EQ* _instr, Thread* _thread)
    {
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (_instr->type)
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

    void Execute(const NEQ* _instr, Thread* _thread)
    {
        auto left = _thread->PopStack(), right = _thread->PopStack();

        switch (_instr->type)
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

    void Execute(const JUMP* _instr, Thread* _thread) { _thread->instrPtr = _instr->target - _instr->GetSize(); }

    void Execute(const JUMPNZ* _instr, Thread* _thread)
    {
        if (!_thread->PopStack().AsBool())
            return;

        _thread->instrPtr = _instr->target - _instr->GetSize();
    }

    void Execute(const JUMPZ* _instr, Thread* _thread)
    {
        if (_thread->PopStack().AsBool())
            return;

        _thread->instrPtr = _instr->target - _instr->GetSize();
    }

    void Execute(const CALL* _instr, Thread* _thread)
    {
        _thread->PushStack((vm_byte*)(_thread->instrPtr + _instr->GetSize())); //Push return value
        _thread->PushFrame();                                          //Push current frame pointer and set the frame pointer for new frame
        _thread->OffsetSP(_instr->storage);                            //Allocate space of stack for function local storage
        _thread->instrPtr = _instr->target - _instr->GetSize();    //Jump to target
    }

    void Execute(const RET* _instr, Thread* _thread)
    {
        _thread->PopFrame();                                                   //Clear current frame and restore previous frame pointer
        _thread->instrPtr = _thread->PopStack().as_ptr - _instr->GetSize(); //Jump to instruction after most recent call
    }

    void Execute(const RETV* _instr, Thread* _thread)
    {
        Word retValue = _thread->PopStack();                                   //Pop off return value
        _thread->PopFrame();                                                   //Clear current frame and restore previous frame pointer
        _thread->instrPtr = _thread->PopStack().as_ptr - _instr->GetSize(); //Jump to instruction after most recent call
        _thread->PushStack(retValue);                                          //Push return value
    }

    void Execute(const PUSH* _instr, Thread* _thread) { _thread->PushStack(_instr->value); }
    void Execute(const POP* _instr, Thread* _thread) { _thread->PopStack(); }
    void Execute(const LLOAD* _instr, Thread* _thread) { _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() + _instr->idx * WORD_SIZE)); }
    void Execute(const LSTORE* _instr, Thread* _thread) { _thread->WriteStack(_thread->GetFP() + _instr->idx * WORD_SIZE, _thread->PopStack()); }
    void Execute(const PLOAD* _instr, Thread* _thread) { _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() - WORD_SIZE * 2 - (_instr->idx + 1) * WORD_SIZE)); }
    void Execute(const PSTORE* _instr, Thread* _thread) { _thread->WriteStack(_thread->GetFP() - WORD_SIZE * 2 - (_instr->idx + 1) * WORD_SIZE, _thread->PopStack()); }

    void Execute(const SLOAD* _instr, Thread* _thread) { _thread->PushStack(_thread->ReadStack<Word>(_thread->GetSP() + _instr->offset)); }

    void Execute(const SSTORE* _instr, Thread* _thread)
    {
        auto value = _thread->PopStack();
        _thread->WriteStack(_thread->GetSP() + _instr->offset, value);
    }

    void Execute(const MLOAD* _instr, Thread* _thread)
    {
        vm_byte* addr = _thread->PopStack().as_ptr + _instr->offset;

        if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
            throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

        _thread->PushStack(*(Word*)addr);
    }

    void Execute(const MSTORE* _instr, Thread* _thread)
    {
        vm_byte* addr = _thread->PopStack().as_ptr + _instr->offset;
        Word value = _thread->PopStack();

        if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
            throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

        *(Word*)addr = value;
    }

    void Execute(const vm_byte* _instr, Thread* _thread)
    {
        switch ((OpCode)*_instr)
        {
        case OpCode::NOOP: Execute(NOOP::From(_instr), _thread); break;
        case OpCode::PUSH: Execute(PUSH::From(_instr), _thread); break;
        case OpCode::POP: Execute(POP::From(_instr), _thread); break;
        case OpCode::ADD: Execute(ADD::From(_instr), _thread); break;
        case OpCode::SUB: Execute(SUB::From(_instr), _thread); break;
        case OpCode::MUL: Execute(MUL::From(_instr), _thread); break;
        case OpCode::DIV: Execute(DIV::From(_instr), _thread); break;
        case OpCode::EQ: Execute(EQ::From(_instr), _thread); break;
        case OpCode::NEQ: Execute(NEQ::From(_instr), _thread); break;
        case OpCode::JUMP: Execute(JUMP::From(_instr), _thread); break;
        case OpCode::JUMPZ: Execute(JUMPZ::From(_instr), _thread); break;
        case OpCode::JUMPNZ: Execute(JUMPNZ::From(_instr), _thread); break;
        case OpCode::SYSCALL: Execute(SYSCALL::From(_instr), _thread); break;
        case OpCode::SLOAD: Execute(SLOAD::From(_instr), _thread); break;
        case OpCode::SSTORE: Execute(SSTORE::From(_instr), _thread); break;
        case OpCode::LLOAD: Execute(LLOAD::From(_instr), _thread); break;
        case OpCode::LSTORE: Execute(LSTORE::From(_instr), _thread); break;
        case OpCode::PLOAD: Execute(PLOAD::From(_instr), _thread); break;
        case OpCode::PSTORE: Execute(PSTORE::From(_instr), _thread); break;
        case OpCode::MLOAD: Execute(MLOAD::From(_instr), _thread); break;
        case OpCode::MSTORE: Execute(MSTORE::From(_instr), _thread); break;
        case OpCode::CONVERT: Execute(CONVERT::From(_instr), _thread); break;
        case OpCode::CALL: Execute(CALL::From(_instr), _thread); break;
        case OpCode::RET: Execute(RET::From(_instr), _thread); break;
        case OpCode::RETV: Execute(RETV::From(_instr), _thread); break;
        default: assert(false && "Case not handled");
        }
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
        case OpCode::POP: return "POP";
        case OpCode::RET: return "RET";
        case OpCode::RETV: return "RETV";
        case OpCode::CONVERT: return "CONVERT " + ToString(CONVERT::From(_instr)->from) + " " + ToString(CONVERT::From(_instr)->to);
        case OpCode::ADD: return "ADD " + ToString(ADD::From(_instr)->type);
        case OpCode::SUB: return "SUB " + ToString(SUB::From(_instr)->type);
        case OpCode::MUL: return "MUL " + ToString(MUL::From(_instr)->type);
        case OpCode::DIV: return "DIV " + ToString(DIV::From(_instr)->type);
        case OpCode::EQ: return "EQ " + ToString(EQ::From(_instr)->type);
        case OpCode::NEQ: return "NEQ " + ToString(NEQ::From(_instr)->type);
        case OpCode::PUSH: return "PUSH " + Hex(PUSH::From(_instr)->value);
        case OpCode::JUMP: return "JUMP " + Hex(JUMP::From(_instr)->target);
        case OpCode::JUMPNZ: return "JUMPNZ " + Hex(JUMPNZ::From(_instr)->target);
        case OpCode::JUMPZ: return "JUMPZ " + Hex(JUMPZ::From(_instr)->target);
        case OpCode::CALL: return "CALL " + Hex(CALL::From(_instr)->target) + " " + std::to_string(CALL::From(_instr)->storage);
        case OpCode::SYSCALL:
        {
            switch (SYSCALL::From(_instr)->code)
            {
            case SysCallCode::EXIT: return "SYSCALL EXIT";
            case SysCallCode::PRINTC: return "SYSCALL PRINTC";
            case SysCallCode::MALLOC: return "SYSCALL MALLOC";
            case SysCallCode::FREE: return "SYSCALL FREE";
            default: assert(false && "Case not handled");
            }
        } break;
        case OpCode::SLOAD: return "SLOAD " + std::to_string(SLOAD::From(_instr)->offset);
        case OpCode::SSTORE: return "SSTORE " + std::to_string(SSTORE::From(_instr)->offset);
        case OpCode::LLOAD: return "LLOAD " + std::to_string(LLOAD::From(_instr)->idx);
        case OpCode::LSTORE: return "LSTORE " + std::to_string(LSTORE::From(_instr)->idx);
        case OpCode::PLOAD: return "PLOAD " + std::to_string(PLOAD::From(_instr)->idx);
        case OpCode::PSTORE: return "PSTORE " + std::to_string(PSTORE::From(_instr)->idx);
        case OpCode::MLOAD: return "MLOAD " + std::to_string(MLOAD::From(_instr)->offset);
        case OpCode::MSTORE: return "MSTORE " + std::to_string(MSTORE::From(_instr)->offset);
        default: assert(false && "Case not handled");
        }

        return "";
    }

    void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent)
    {
        _stream << _indent << ToString(_instr) << "\n";

        switch ((OpCode)*_instr)
        {
        case OpCode::PUSH:
        {
            _stream << _indent << "mov rax, " << Hex(PUSH::From(_instr)->value) << "\n";
            _stream << _indent << "push rax";
        } break;
        case OpCode::NOOP: { _stream << _indent << "mov rax, rax"; } break;
        case OpCode::SYSCALL:
        {
            switch (SYSCALL::From(_instr)->code)
            {
            case SysCallCode::EXIT:
            {
                _stream << _indent << "pop rdi\n";
                _stream << _indent << "mov rax, 0x02000001\n";
                _stream << _indent << "syscall";
            } break;
            default: throw Error::CompilationNotImplemented(_instr);
            }
        } break;
        case OpCode::POP: { _stream << _indent << "pop rax"; } break;
        default:
        throw Error::CompilationNotImplemented(_instr);
        }
    }
}