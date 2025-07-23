#include "RendererAPI.h"
#include "NRI/NRIRenderer.h"

namespace Lumina
{
    RendererAPI* RendererAPI::Create()
    {
        return new NRIRenderer();
    }
}