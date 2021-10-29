#include "heap.h"
#include "../build.h"

Block::Block(size_t _size)
{
    assert(_size > 0 && "Block must be initialized with positive size!");
    storage = Memory(_size);

    Chunk initial(&storage.front(), storage.size(), nullptr, nullptr);
    chunks.emplace(initial.start, initial);
}

void Block::Defragment()
{
    std::vector<vm_byte *> toRemove;

    for (auto &[_, chunk] : chunks)
    {
        if (chunk.allocated)
            continue;

        Chunk *current = &chunk;

        //Merge with left adjacent unallocated blocks
        while (current->prev && !current->prev->allocated)
        {
            current->prev->next = current->next;
            current->prev->size += current->size;
            toRemove.push_back(current->start);
            current = current->prev;
        }

        //Merge with right adjacent unallocated blocks
        while (current->next && !current->next->allocated)
        {
            current->next->prev = current;
            current->size += current->next->size;
            toRemove.push_back(current->next->start);
            current->next = current->next->next;
        }

        //Ensure chunk pointers are continuous
        current->next->prev = current;
    }

    //Remove unused chunks
    for (auto &chunkPtr : toRemove)
        chunks.erase(chunkPtr);
}

bool Block::HasAddress(vm_byte *_addr) { return _addr >= &storage.front() && _addr <= &storage.back(); }

bool Block::IsAllocated(vm_byte *_addr)
{
    auto search = chunks.find(_addr);
    return search != chunks.end() && search->second.allocated;
}

void Block::Alloc(vm_byte *_chunkPtr, vm_ui64 _amt)
{
    Chunk &chunk = chunks.at(_chunkPtr);
    vm_byte *nextChunkStart = _chunkPtr + _amt;

    if (HasAddress(nextChunkStart))
    {
        Chunk next(nextChunkStart, chunk.size - _amt, &chunk, chunk.next);
        chunks.emplace(next.start, next);
        chunk.next = &next;
    }

    chunk.allocated = true;
    chunk.size = _amt;
}

void Block::Free(vm_byte *_chunkPtr)
{
    assert(false && "NOT IMPLEMENTED");
}

Heap::Heap()
    : blocks()
{
}

vm_byte *Heap::Alloc(vm_ui64 _amt)
{
    assert(false && "NOT IMPLEMENTED");
}

void Free(vm_byte *_addr)
{
    assert(false && "NOT IMPLEMENTED");
}

bool Heap::IsAddress(vm_byte *_addr)
{
    for (auto &block : blocks)
    {
        if (block.HasAddress(_addr))
            return true;
    }

    return false;
}

bool Heap::IsAllocated(vm_byte *_addr)
{
    assert(false && "NOT IMPLEMENTED");
}

size_t Heap::GetSize()
{
#ifdef BUILD_DEBUG
    size_t expected = 0;
    for (auto &block : blocks)
        expected += block.GetSize();

    assert(size == expected && ("Expected size of heap to be " + std::to_string(expected) + " but found Heap::size = " + std::to_string(size)).c_str());
#endif

    return size;
}