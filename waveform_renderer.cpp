
#include "waveform_renderer.h"

#include <cmath>
#include <iostream>

namespace sigcodec {
namespace {

void addLine(
    sf::RenderTexture& canvas,
    sf::Vector2f start,
    sf::Vector2f finish,
    sf::Color color,
    float thickness = 1.f
) {
    const sf::Vector2f delta = finish - start;
    const float length =
        std::sqrt(delta.x * delta.x + delta.y * delta.y);

    if (length <= 0.f) {
        return;
    }

    sf::RectangleShape line({length, thickness});
    line.setFillColor(color);
    line.setOrigin({0.f, thickness / 2.f});
    line.setPosition(start);
    line.setRotation(sf::radians(std::atan2(delta.y, delta.x)));
    canvas.draw(line);
}

} // namespace

bool renderWaveform(
    const std::string& bitStream,
    const Levels& signal,
    const std::string& scheme,
    const std::string& outputImage,
    const WaveformConfig& config
) {
    if (bitStream.empty() ||
        signal.empty() ||
        bitStream.size() != signal.size() ||
        config.width == 0 ||
        config.height == 0 ||
        config.width <= config.leftMargin + config.rightMargin ||
        config.height <= config.topMargin + config.bottomMargin) {
        std::cerr << "Cannot render waveform: invalid input or dimensions.\n";
        return false;
    }

    sf::RenderTexture canvas;
    if (!canvas.resize({config.width, config.height})) {
        std::cerr << "Could not create waveform render texture.\n";
        return false;
    }

    const sf::Color background(250, 250, 250);
    const sf::Color gridColor(220, 220, 220);
    const sf::Color axisColor(90, 90, 90);
    const sf::Color traceColor(20, 80, 220);

    canvas.clear(background);

    const float plotLeft = config.leftMargin;
    const float plotRight =
        static_cast<float>(config.width) - config.rightMargin;
    const float plotTop = config.topMargin;
    const float plotBottom =
        static_cast<float>(config.height) - config.bottomMargin;

    const float centerY = (plotTop + plotBottom) / 2.f;
    const float amplitude = (plotBottom - plotTop) * 0.38f;
    const float bitWidth =
        (plotRight - plotLeft) / static_cast<float>(signal.size());

    // Vertical bit divisions.
    for (std::size_t i = 0; i <= signal.size(); ++i) {
        const float x =
            plotLeft + static_cast<float>(i) * bitWidth;

        addLine(canvas, {x, plotTop}, {x, plotBottom}, gridColor);
    }

    // Voltage-level grid: -1, 0, +1.
    for (int level = -1; level <= 1; ++level) {
        const float y =
            centerY - static_cast<float>(level) * amplitude;

        addLine(canvas, {plotLeft, y}, {plotRight, y}, gridColor);
    }

    // Axes.
    addLine(
        canvas, {plotLeft, plotTop}, {plotLeft, plotBottom},
        axisColor, 2.f
    );

    addLine(
        canvas, {plotLeft, centerY}, {plotRight, centerY},
        axisColor, 2.f
    );

    addLine(
        canvas, {plotLeft, plotBottom}, {plotRight, plotBottom},
        axisColor, 2.f
    );

    // Draw the signal as a step waveform.
    float previousY =
        centerY - static_cast<float>(signal[0]) * amplitude;

    addLine(
        canvas, {plotLeft, previousY},
        {plotLeft + bitWidth, previousY},
        traceColor, 3.f
    );

    for (std::size_t i = 1; i < signal.size(); ++i) {
        const float x =
            plotLeft + static_cast<float>(i) * bitWidth;

        const float currentY =
            centerY - static_cast<float>(signal[i]) * amplitude;

        if (std::abs(currentY - previousY) > 0.1f) {
            addLine(
                canvas, {x, previousY}, {x, currentY},
                traceColor, 3.f
            );
        }

        addLine(
            canvas, {x, currentY},
            {x + bitWidth, currentY},
            traceColor, 3.f
        );

        previousY = currentY;
    }

    canvas.display();

    const sf::Image image = canvas.getTexture().copyToImage();

    if (!image.saveToFile(outputImage)) {
        std::cerr << "Could not save waveform image: "
                  << outputImage << '\n';
        return false;
    }

    std::cout << "Waveform image saved to: "
              << outputImage << '\n';

    std::cout << "Scheme: " << scheme
              << " | Bits: " << bitStream.size()
              << " | Image: " << config.width
              << 'x' << config.height << '\n';

    return true;
}

} // namespace sigcodec
