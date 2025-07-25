#pragma once
#include "FrameGraph.h"

namespace Lumina
{
    class PostProcessing
    {
    public:
        static void AddEffect(const std::string& name);
        static void RemoveEffect(const std::string& name);
        static void SetEffectEnabled(const std::string& name, bool enabled);

        static void ConfigureEffect(
            const std::string& name,
            const std::vector<Parameter>& params);

        static void SetupFrameGraph(FrameGraph& frameGraph);

        struct Effect {
            std::string name;
            ShaderHandle shader;
            bool enabled = true;
            std::vector<Parameter> parameters;
        };

        static const std::vector<Effect>& GetActiveEffects();
    };
}