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
        if (!chunk.allocated)
        {
            Chunk *current = &chunk;
            while(current->prev && !current->prev->allocated)
            {
                current->prev->next = current->next;
                current->prev->size +=current->size;
                toRemove.push_back(current->start);
                current = current->prev;
            }

            while(current->next && !current->next->allocated)
            {
                current->next->prev = current;
                current->prev->size +=current->size;
                toRemove.push_back(current->start);
            }
        }
    }
}

bool Block::HasAddress(vm_byte *_addr) { return _addr >= &storage.front() && _addr <= &storage.back(); }

bool Block::IsAllocated(vm_byte *_addr)
{
    auto search = chunks.find(_addr);
    return search != chunks.end() && search->second.allocated;
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