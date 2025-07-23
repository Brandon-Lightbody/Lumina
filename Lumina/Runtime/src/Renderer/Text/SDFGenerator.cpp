#include "SDFGenerator.h"
#include <cmath>

namespace Lumina
{
    void SDFGenerator::Generate(const uint8_t* bitmap, uint32_t width, uint32_t height, std::vector<uint8_t>& output)
    {
        output.resize(width * height);
        const int spread = 8; // SDF spread distance

        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                float minDist = FLT_MAX;

                // Find min distance to edge
                for (int dy = -spread; dy <= spread; dy++) {
                    for (int dx = -spread; dx <= spread; dx++) {
                        int nx = x + dx;
                        int ny = y + dy;

                        if (nx >= 0 && nx < (int)width && ny >= 0 && ny < (int)height) {
                            uint8_t pixel = bitmap[ny * width + nx];
                            if (pixel < 128) { // Edge found
                                float dist = sqrtf(dx * dx + dy * dy);
                                if (dist < minDist) minDist = dist;
                            }
                        }
                    }
                }

                // Normalize to [0, 255]
                float sdf = std::min(minDist / spread, 1.0f);
                output[y * width + x] = static_cast<uint8_t>(sdf * 255);
            }
        }
    }
}