#pragma once
#include "evm.h"
#include <unordered_map>

struct Chunk;
class Block;

typedef std::unordered_map<vm_ui64, std::unordered_map<Chunk *, Block *>> FreeChunksMap;

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
    Block(size_t _size, FreeChunksMap &_freeChunks);

    void Alloc(vm_byte *_chunkPtr, vm_ui64 _amt, FreeChunksMap &_freeChunks);
    void Free(vm_byte *_chunkPtr, FreeChunksMap &_freeChunks);
    void Defragment();

    bool IsAllocated(vm_byte *_addr);
    bool HasAddress(vm_byte *_addr);
    bool IsEmpty();

    size_t GetSize() { return storage.size(); }
    vm_byte *GetStart() { return &storage.front(); }
};

class Heap
{
    std::vector<Block> blocks;
    FreeChunksMap freeChunks;
    size_t size;

public:
    Heap(size_t _initialSize);

    vm_byte *Alloc(vm_ui64 _amt);
    void Free(vm_byte *_addr);
    bool IsAddress(vm_byte *_addr);
    bool IsAllocated(vm_byte *_addr);

    size_t GetSize();
};