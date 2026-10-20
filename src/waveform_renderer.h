#ifndef WAVEFORM_RENDERER_H
#define WAVEFORM_RENDERER_H

#include <SFML/Graphics.hpp>
#include <string>
#include "encoders.h"

namespace sigcodec {

struct WaveformConfig {
    unsigned int width = 1000;
    unsigned int height = 500;
    float leftMargin = 70.f;
    float rightMargin = 30.f;
    float topMargin = 50.f;
    float bottomMargin = 60.f;
};

bool renderWaveform(
    const std::string& bitStream,
    const Levels& signal,
    const std::string& scheme,
    const std::string& outputImage,
    const WaveformConfig& config = {}
);

} // namespace sigcodec

#endif
