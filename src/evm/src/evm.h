#pragma once

#include <cstdint>
#include <assert.h>
#include <sstream>
#include <iomanip>

typedef uint8_t vm_byte;
typedef int32_t vm_i32;
typedef int64_t vm_i64;
typedef uint32_t vm_ui32;
typedef uint64_t vm_ui64;
typedef float vm_f32;
typedef double vm_f64;
typedef void *vm_ptr;

#define VM_BYTE_SIZE sizeof(vm_byte)
#define VM_I32_SIZE sizeof(vm_i32)
#define VM_I64_SIZE sizeof(vm_i64)
#define VM_UI32_SIZE sizeof(vm_ui32)
#define VM_UI64_SIZE sizeof(vm_ui64)
#define VM_F32_SIZE sizeof(vm_f32)
#define VM_F64_SIZE sizeof(vm_f64)
#define VM_PTR_SIZE sizeof(vm_ptr)
#define WORD_SIZE sizeof(vm_ptr)

union Word
{
    vm_byte as_byte;
    vm_i32 as_i32;
    vm_i64 as_i64;
    vm_ui32 as_ui32;
    vm_ui64 as_ui64;
    vm_f32 as_f32;
    vm_f64 as_f64;
    vm_ptr as_ptr;
    vm_byte bytes[sizeof(vm_ptr)];

    Word() : as_ptr(0) {}
    Word(vm_byte _b) : as_byte(_b) {}
    Word(vm_i32 _i) : as_i32(_i) {}
    Word(vm_i64 _i) : as_i64(_i) {}
    Word(vm_ui32 _i) : as_ui32(_i) {}
    Word(vm_ui64 _i) : as_ui64(_i) {}
    Word(vm_f32 _f) : as_f32(_f) {}
    Word(vm_f64 _f) : as_f64(_f) {}
    Word(vm_ptr _p) : as_ptr(_p) {}

    Word(vm_byte _0, vm_byte _1, vm_byte _2, vm_byte _3, vm_byte _4, vm_byte _5, vm_byte _6, vm_byte _7)
    {
        bytes[0] = _0;
        bytes[1] = _1;
        bytes[2] = _2;
        bytes[3] = _3;
        bytes[4] = _4;
        bytes[5] = _5;
        bytes[6] = _6;
        bytes[7] = _7;
    }
};

static_assert(sizeof(Word) == WORD_SIZE, "Word is not the right size!");

template <typename T>
std::string Hex(const T &_value)
{
    std::stringstream stream("0x");
    stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << _value;
    return stream.str();
}
