#include "AccelerationStructure.h"

namespace Lumina
{
    AccelerationStructure::AccelerationStructure(nri::Device& device, nri::AccelerationStructureType type)
        : m_Device(device), m_Type(type) {
    }

    void AccelerationStructure::Create(nri::AccelerationStructureDesc desc)
    {
        desc.type = m_Type;
        m_Device.CreateAccelerationStructure(desc, &m_Handle);
    }

    void AccelerationStructure::Update(nri::CommandBuffer& cmd, const nri::AccelerationStructureDesc& desc)
    {
        nri::BuildAccelerationStructureDesc buildDesc = {};
        buildDesc.instanceNum = desc.instanceNum;
        buildDesc.instances = desc.instances;
        cmd.BuildAccelerationStructure(buildDesc);
    }
}