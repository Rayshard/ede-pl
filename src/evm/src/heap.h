#pragma once
#include "evm.h"
#include <unordered_map>
#include <unordered_set>

struct Chunk
{
    vm_byte *start;
    vm_ui64 size;
    Chunk *prev, *next;
    bool allocated;

    Chunk(vm_byte *_start, vm_ui64 _size, Chunk *_prev, Chunk *_next)
        : start(_start), size(_size), prev(_prev), next(_next), allocated(false) {}
};

class Block
{
    Memory storage;
    std::unordered_map<vm_byte *, Chunk> chunks;


public:
    Block(size_t _size);

    void Alloc(vm_byte* _chunkPtr, vm_ui64 _amt);
    void Free(vm_byte *_chunkPtr);
    void Defragment();

    bool IsAllocated(vm_byte *_addr);
    bool HasAddress(vm_byte *_addr);

    size_t GetSize() { return storage.size(); }
};

class Heap
{
    std::vector<Block> blocks;
    std::unordered_set<vm_byte*> freeChunks;
    size_t size;

public:
    Heap();

    vm_byte *Alloc(vm_ui64 _amt);
    void Free(vm_byte *_addr);
    bool IsAddress(vm_byte *_addr);
    bool IsAllocated(vm_byte *_addr);

    size_t GetSize();
};