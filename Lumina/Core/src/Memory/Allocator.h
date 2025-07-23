#pragma once
#include "Core/Base.h"

namespace Lumina::Memory
{
    class Allocator
    {
    public:
        virtual ~Allocator() = default;

        virtual void* Allocate(size_t size, size_t alignment) = 0;
        virtual void Free(void* ptr) = 0;
        virtual void Reset() = 0;
    };

    class LinearAllocator : public Allocator
    {
    public:
        LinearAllocator(size_t size);
        ~LinearAllocator();

        void* Allocate(size_t size, size_t alignment) override;
        void Free(void* ptr) override;
        void Reset() override;

    private:
        uint8_t* m_Memory = nullptr;
        size_t m_Size = 0;
        size_t m_Offset = 0;
    };
}