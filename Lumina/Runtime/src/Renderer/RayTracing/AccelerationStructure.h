#pragma once
#include "Core/Base.h"
#include <NRI.h>

namespace Lumina
{
    class AccelerationStructure
    {
    public:
        AccelerationStructure(nri::Device& device, nri::AccelerationStructureType type);
        ~AccelerationStructure();

        void Create(nri::AccelerationStructureDesc desc);
        void Update(nri::CommandBuffer& cmd, const nri::AccelerationStructureDesc& desc);

        nri::AccelerationStructure* GetHandle() const { return m_Handle; }

    private:
        nri::Device& m_Device;
        nri::AccelerationStructure* m_Handle = nullptr;
        nri::AccelerationStructureType m_Type;
    };
}