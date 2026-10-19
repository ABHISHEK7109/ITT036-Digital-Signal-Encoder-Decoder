#include "raylib.h"
#include "encoders.h"
#include "input_engine.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

constexpr int SCREEN_WIDTH = 1200;
constexpr int SCREEN_HEIGHT = 760;
constexpr int MAX_BITS = 128;

const Color C_BG = {248, 250, 252, 255};
const Color C_PANEL = {255, 255, 255, 255};
const Color C_BORDER = {226, 232, 240, 255};
const Color C_TEXT = {30, 41, 59, 255};
const Color C_MUTED = {100, 116, 139, 255};
const Color C_BLUE = {37, 99, 235, 255};
const Color C_HOVER_BLUE = {29, 78, 216, 255};
const Color C_GREEN = {22, 163, 74, 255};
const Color C_RED = {220, 38, 38, 255};
const Color C_AMBER = {217, 119, 6, 255};
const Color C_GRID = {241, 245, 249, 255};
const Color C_PCM_PURPLE = {124, 58, 237, 255};

const char* SCHEMES[] = {
    "NRZ-L",
    "NRZ-I",
    "Manchester",
    "Diff. Manchester",
    "AMI",
    "B8ZS",
    "HDB3"
};

constexpr int SCHEME_COUNT = 7;

Font appFont{};

std::string bitInput = "10000001";
std::string encodedText;
std::string palindrome;
std::string status = "Ready. Select a scheme and encode.";

std::string amplitudeInput = "5.0";
std::string frequencyInput = "1.0";
std::string samplesInput = "16";
std::string bitsPerSampleInput = "4";

Levels encodedLevels;
std::vector<int> substituted;
PCMResult pcmResult;

int selectedScheme = 0;

bool inputFocused = false;
bool amplitudeFocused = false;
bool frequencyFocused = false;
bool samplesFocused = false;
bool bitsPerSampleFocused = false;
bool hasEncoded = false;
bool analogMode = false;
bool showPcmStaircase = false;

void DrawAppText(
    const char* text,
    float x,
    float y,
    float fontSize,
    Color textColor
) {
    DrawTextEx(
        appFont,
        text,
        {x, y},
        fontSize,
        1.0f,
        textColor
    );
}

void DrawAppText(
    const std::string& text,
    float x,
    float y,
    float fontSize,
    Color textColor
) {
    DrawAppText(text.c_str(), x, y, fontSize, textColor);
}

void DrawPanel(
    int x,
    int y,
    int w,
    int h,
    const char* title
) {
    Rectangle r = {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(w),
        static_cast<float>(h)
    };

    DrawRectangleRounded(r, 0.04f, 8, C_PANEL);
    DrawRectangleRoundedLines(r, 0.04f, 8, C_BORDER);

    if (title != nullptr && title[0] != '\0') {
        DrawAppText(title, x + 18, y + 14, 20, C_TEXT);
    }
}

bool DrawButton(
    int x,
    int y,
    int w,
    int h,
    const char* label,
    bool selected = false
) {
    Rectangle r = {
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(w),
        static_cast<float>(h)
    };

    const bool hovered =
        CheckCollisionPointRec(GetMousePosition(), r);

    const bool clicked =
        hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Color fill = selected ? C_BLUE : C_PANEL;
    const Color labelColor = selected ? RAYWHITE : C_TEXT;

    if (hovered) {
        fill = selected ? C_HOVER_BLUE : C_GRID;
    }

    DrawRectangleRounded(r, 0.12f, 8, fill);

    if (!selected) {
        DrawRectangleRoundedLines(r, 0.12f, 8, C_BORDER);
    }

    const Vector2 ts = MeasureTextEx(appFont, label, 15, 1.0f);

    DrawAppText(
        label,
        x + (w - ts.x) / 2.0f,
        y + (h - ts.y) / 2.0f,
        15,
        labelColor
    );

    return clicked;
}

void DrawInputField(
    const char* label,
    Rectangle rect,
    const std::string& value,
    bool focused
) {
    DrawAppText(label, rect.x, rect.y - 19, 13, C_MUTED);

    DrawRectangleRounded(
        rect,
        0.08f,
        8,
        focused ? Color{239, 246, 255, 255} : C_BG
    );

    DrawRectangleRoundedLines(
        rect,
        0.08f,
        8,
        focused ? C_BLUE : C_BORDER
    );

    DrawAppText(value, rect.x + 9, rect.y + 7, 15, C_TEXT);

    if (focused && (static_cast<int>(GetTime() * 2) % 2 == 0)) {
        const float textWidth =
            MeasureTextEx(appFont, value.c_str(), 15, 1.0f).x;

        const float cursorX = rect.x + 10 + textWidth;

        DrawLine(
            static_cast<int>(cursorX),
            static_cast<int>(rect.y + 6),
            static_cast<int>(cursorX),
            static_cast<int>(rect.y + rect.height - 6),
            C_BLUE
        );
    }
}

bool IsValidBits(const std::string& bits) {
    return !bits.empty() &&
        std::all_of(bits.begin(), bits.end(), [](char c) {
            return c == '0' || c == '1';
        });
}

bool ParsePositiveNumber(
    const std::string& input,
    double& result
) {
    if (input.empty()) {
        return false;
    }

    try {
        std::size_t used = 0;
        result = std::stod(input, &used);

        return used == input.size() &&
               std::isfinite(result) &&
               result > 0.0;
    } catch (...) {
        return false;
    }
}

bool ParsePositiveInteger(
    const std::string& input,
    int& result
) {
    if (input.empty()) {
        return false;
    }

    try {
        std::size_t used = 0;
        const int value = std::stoi(input, &used);

        if (used != input.size() || value <= 0) {
            return false;
        }

        result = value;
        return true;
    } catch (...) {
        return false;
    }
}

void HandleNumericInput(
    std::string& value,
    bool focused,
    bool allowDecimal,
    std::size_t maxLength = 12
) {
    if (!focused) {
        return;
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !value.empty()) {
        value.pop_back();
    }

    int key = GetCharPressed();

    while (key > 0) {
        const bool digit = key >= '0' && key <= '9';
        const bool decimal = allowDecimal && key == '.';

        if (value.size() < maxLength && (digit || decimal)) {
            if (!decimal || value.find('.') == std::string::npos) {
                value.push_back(static_cast<char>(key));
            }
        }

        key = GetCharPressed();
    }
}

// Manacher's algorithm: O(n).
std::string LongestPalindrome(const std::string& s) {
    if (s.empty()) {
        return "";
    }

    const int n = static_cast<int>(s.size());
    std::vector<int> odd(n, 0);
    std::vector<int> even(n, 0);

    int left = 0;
    int right = -1;
    int bestStart = 0;
    int bestLength = 1;

    for (int i = 0; i < n; ++i) {
        int k = (i > right)
            ? 1
            : std::min(odd[left + right - i], right - i + 1);

        while (i - k >= 0 &&
               i + k < n &&
               s[i - k] == s[i + k]) {
            ++k;
        }

        odd[i] = k;

        const int length = 2 * k - 1;

        if (length > bestLength) {
            bestLength = length;
            bestStart = i - k + 1;
        }

        if (i + k - 1 > right) {
            left = i - k + 1;
            right = i + k - 1;
        }
    }

    left = 0;
    right = -1;

    for (int i = 0; i < n; ++i) {
        int k = (i > right)
            ? 0
            : std::min(
                even[left + right - i + 1],
                right - i + 1
            );

        while (i - k - 1 >= 0 &&
               i + k < n &&
               s[i - k - 1] == s[i + k]) {
            ++k;
        }

        even[i] = k;

        const int length = 2 * k;

        if (length > bestLength) {
            bestLength = length;
            bestStart = i - k;
        }

        if (i + k - 1 > right) {
            left = i - k;
            right = i + k - 1;
        }
    }

    return s.substr(
        static_cast<std::size_t>(bestStart),
        static_cast<std::size_t>(bestLength)
    );
}

void EncodeCurrentBits() {
    if (!IsValidBits(bitInput)) {
        hasEncoded = false;
        encodedLevels.clear();
        encodedText.clear();
        palindrome.clear();
        substituted.clear();
        status = "Enter a non-empty stream containing only 0 and 1.";
        return;
    }

    if (bitInput.size() > MAX_BITS) {
        hasEncoded = false;
        encodedLevels.clear();
        encodedText.clear();
        palindrome.clear();
        substituted.clear();
        status = "Maximum input length is 128 bits.";
        return;
    }

    substituted.clear();

    switch (selectedScheme) {
        case 0:
            encodedLevels = encodeNRZL(bitInput);
            break;

        case 1:
            encodedLevels = encodeNRZI(bitInput);
            break;

        case 2:
            encodedLevels = encodeManchester(bitInput);
            break;

        case 3:
            encodedLevels = encodeDiffManchester(bitInput);
            break;

        case 4:
            encodedLevels = encodeAMI(bitInput);
            break;

        case 5: {
            ScrambleResult result = encodeB8ZS(bitInput);
            encodedLevels = result.scrambled;
            substituted = result.substituted;
            break;
        }

        case 6: {
            ScrambleResult result = encodeHDB3(bitInput);
            encodedLevels = result.scrambled;
            substituted = result.substituted;
            break;
        }

        default:
            hasEncoded = false;
            status = "Invalid encoding scheme.";
            return;
    }

    encodedText = levelsToString(encodedLevels);
    palindrome = LongestPalindrome(bitInput);
    hasEncoded = true;

    status = std::string("Encoded successfully using ") +
             SCHEMES[selectedScheme] + ".";
}

void GenerateRandomBits(bool withZeroRuns) {
    bitInput = withZeroRuns
        ? generateBitsWithZeroRuns(32)
        : generateRandomBits(16);

    analogMode = false;
    inputFocused = false;
    amplitudeFocused = false;
    frequencyFocused = false;
    samplesFocused = false;
    bitsPerSampleFocused = false;
    showPcmStaircase = false;

    EncodeCurrentBits();

    status = withZeroRuns
        ? "Generated random bits with fixed zero runs where possible."
        : "Generated random digital bits.";
}

void GeneratePCM() {
    double amplitude = 0.0;
    double frequency = 0.0;
    int samples = 0;
    int bitsPerSample = 0;

    if (!ParsePositiveNumber(amplitudeInput, amplitude)) {
        status = "Amplitude must be a positive number.";
        return;
    }

    if (!ParsePositiveNumber(frequencyInput, frequency)) {
        status = "Frequency must be a positive number.";
        return;
    }

    if (!ParsePositiveInteger(samplesInput, samples) ||
        samples > 128) {
        status = "Samples must be between 1 and 128.";
        return;
    }

    if (!ParsePositiveInteger(bitsPerSampleInput, bitsPerSample) ||
        bitsPerSample > 16) {
        status = "Bits per sample must be between 1 and 16.";
        return;
    }

    if (samples * bitsPerSample > MAX_BITS) {
        status = "PCM output cannot exceed 128 bits.";
        return;
    }

    pcmResult = processPCMAnalogInput(
        amplitude,
        frequency,
        samples,
        bitsPerSample
    );

    if (pcmResult.bits.empty()) {
        status = "PCM generation failed. Check the input parameters.";
        return;
    }

    bitInput = pcmResult.bits;
    analogMode = true;

    inputFocused = false;
    amplitudeFocused = false;
    frequencyFocused = false;
    samplesFocused = false;
    bitsPerSampleFocused = false;

    EncodeCurrentBits();

    if (hasEncoded) {
        status = "PCM generated; PCM bits fed into the selected line encoder.";
    }
}

void DecodePCMCurrentBits() {
    if (!analogMode || pcmResult.bits.empty()) {
        status = "Generate PCM data first before decoding.";
        return;
    }

    const std::vector<double> reconstructedSamples = decodePCM(
        pcmResult.bits,
        pcmResult.bitsPerSample,
        pcmResult.amplitude
    );

    if (reconstructedSamples.empty()) {
        status = "PCM decoding failed. Check the input parameters.";
        return;
    }

    status = "PCM decoded successfully: " +
             std::to_string(reconstructedSamples.size()) +
             " reconstructed voltage samples.";

    showPcmStaircase = true;
}

void DrawWaveform(int x, int y, int width, int height) {
    const int left = x + 48;
    const int right = x + width - 20;
    const int top = y + 58;
    const int bottom = y + height - 52;
    const int center = (top + bottom) / 2;

    DrawRectangle(
        left, top, right - left, bottom - top,
        Color{252, 253, 255, 255}
    );

    for (int gy = top; gy <= bottom; gy += 30) {
        DrawLine(left, gy, right, gy, C_GRID);
    }

    DrawLine(left, center, right, center, C_BORDER);

    DrawAppText("+1", x + 14, top + 8, 13, C_MUTED);
    DrawAppText("0", x + 22, center - 7, 13, C_MUTED);
    DrawAppText("-1", x + 14, bottom - 18, 13, C_MUTED);

    if (!hasEncoded || encodedLevels.empty()) {
        DrawAppText(
            "Enter bits and click Apply entered bits.",
            left + 12, center - 10, 16, C_MUTED
        );
        return;
    }

    const int sampleCount = static_cast<int>(encodedLevels.size());
    const bool twoSamplesPerBit =
        selectedScheme == 2 || selectedScheme == 3;

    const float cellWidth =
        static_cast<float>(right - left) / sampleCount;

    if (cellWidth < 1.0f) {
        DrawAppText(
            "Waveform is too dense to display.",
            left, center - 10, 15, C_MUTED
        );
        return;
    }

    const auto levelY = [&](int level) -> float {
        if (level > 0) return static_cast<float>(top + 22);
        if (level < 0) return static_cast<float>(bottom - 22);
        return static_cast<float>(center);
    };

    const int bitCount = static_cast<int>(bitInput.size());

    for (int i = 0; i <= bitCount; ++i) {
        const int sampleIndex = i * (twoSamplesPerBit ? 2 : 1);

        if (sampleIndex <= sampleCount) {
            const float bx = left + sampleIndex * cellWidth;
            DrawLine(
                static_cast<int>(bx), top,
                static_cast<int>(bx), bottom,
                C_BORDER
            );
        }
    }

    for (int i = 0; i < sampleCount; ++i) {
        const float x1 = left + i * cellWidth;
        const float x2 = left + (i + 1) * cellWidth;
        const float currentY = levelY(encodedLevels[i]);

        Color signalColor = C_BLUE;

        if (selectedScheme >= 5 &&
            i < static_cast<int>(substituted.size()) &&
            substituted[i] == 1) {
            signalColor = C_AMBER;
        }

        DrawLineEx(
            {x1, currentY}, {x2, currentY},
            3.0f, signalColor
        );

        if (i + 1 < sampleCount) {
            const float nextY = levelY(encodedLevels[i + 1]);

            if (nextY != currentY) {
                DrawLineEx(
                    {x2, currentY}, {x2, nextY},
                    3.0f, signalColor
                );
            }
        }
    }

    const int labelStep =
        bitCount <= 16 ? 1 : bitCount <= 32 ? 2 : 4;

    for (int i = 0; i < bitCount; i += labelStep) {
        const float bx = left + i * cellWidth *
            (twoSamplesPerBit ? 2.0f : 1.0f);

        if (bx + 12 < right) {
            DrawAppText(
                TextFormat("%d", i),
                static_cast<int>(bx) + 2,
                bottom + 5,
                10,
                C_MUTED
            );
        }
    }

    DrawAppText(
        "Bit index",
        left + (right - left) / 2 - 22,
        bottom + 20,
        11,
        C_TEXT
    );

    DrawAppText(
        "Blue: encoded signal",
        left, y + height - 23, 12, C_BLUE
    );

    if (selectedScheme >= 5) {
        DrawAppText(
            "Amber: substituted positions",
            left + 175, y + height - 23, 12, C_AMBER
        );
    }
}

void DrawPCMStaircase(int x, int y, int width, int height) {
    const int left = x + 48;
    const int right = x + width - 20;
    const int top = y + 58;
    const int bottom = y + height - 52;
    const int center = (top + bottom) / 2;

    DrawRectangle(
        left, top, right - left, bottom - top,
        Color{252, 253, 255, 255}
    );

    for (int gy = top; gy <= bottom; gy += 30) {
        DrawLine(left, gy, right, gy, C_GRID);
    }

    DrawLine(left, center, right, center, C_BORDER);

    if (!analogMode ||
        pcmResult.levels.empty() ||
        pcmResult.voltages.empty()) {
        DrawAppText(
            "Choose Analog / PCM and generate PCM first.",
            left + 8, center - 10, 15, C_MUTED
        );
        return;
    }

    const int count = static_cast<int>(pcmResult.levels.size());
    const int quantizationLevels = 1 << pcmResult.bitsPerSample;

    if (count == 0 ||
        quantizationLevels <= 0 ||
        pcmResult.voltages.size() != pcmResult.levels.size()) {
        return;
    }

    const float plotWidth = static_cast<float>(right - left);
    const float plotHeight = static_cast<float>(bottom - top);
    const float cellWidth = plotWidth / count;
    const double amplitude = pcmResult.amplitude;

    if (amplitude <= 0.0) {
        return;
    }

    // Voltage-to-screen mapping.
    const auto voltageY = [&](double voltage) -> float {
        const double normalized =
            (voltage + amplitude) / (2.0 * amplitude);

        return static_cast<float>(
            bottom - normalized * plotHeight
        );
    };

    // Quantized level-to-screen mapping.
    const auto levelY = [&](int level) -> float {
        const float normalized =
            (static_cast<float>(level) + 0.5f) /
            static_cast<float>(quantizationLevels);

        return static_cast<float>(
            bottom - normalized * plotHeight
        );
    };

    // Grid and sample guides.
    for (int i = 0; i <= count; ++i) {
        const float px = left + i * cellWidth;

        DrawLine(
            static_cast<int>(px), top,
            static_cast<int>(px), bottom,
            C_GRID
        );
    }

    // Actual voltage labels.
    DrawAppText(
        TextFormat("+%.1f V", amplitude),
        x + 2, top + 8, 12, C_MUTED
    );

    DrawAppText(
        "0 V",
        x + 14, center - 7, 12, C_MUTED
    );

    DrawAppText(
        TextFormat("-%.1f V", amplitude),
        x + 2, bottom - 18, 12, C_MUTED
    );

    // Draw the original sampled analog signal in blue.
    for (int i = 0; i + 1 < count; ++i) {
        const float x1 = left + (i + 0.5f) * cellWidth;
        const float x2 = left + (i + 1.5f) * cellWidth;

        const float y1 = voltageY(pcmResult.voltages[i]);
        const float y2 = voltageY(pcmResult.voltages[i + 1]);

        DrawLineEx(
            {x1, y1}, {x2, y2},
            2.0f, C_BLUE
        );
    }

    // Draw the PCM quantization staircase in purple.
    for (int i = 0; i < count; ++i) {
        const float x1 = left + i * cellWidth;
        const float x2 = left + (i + 1) * cellWidth;
        const float yy = levelY(pcmResult.levels[i]);

        DrawLineEx(
            {x1, yy}, {x2, yy},
            3.0f, C_PCM_PURPLE
        );

        if (i + 1 < count) {
            const float nextY = levelY(pcmResult.levels[i + 1]);

            DrawLineEx(
                {x2, yy}, {x2, nextY},
                3.0f, C_PCM_PURPLE
            );
        }

        // Sample markers.
        DrawCircle(
            static_cast<int>(x1 + cellWidth / 2.0f),
            static_cast<int>(voltageY(pcmResult.voltages[i])),
            3.0f,
            C_BLUE
        );
    }

    // X-axis sample-index labels.
    const int labelStep =
        count <= 16 ? 1 : count <= 32 ? 2 : 4;

    for (int i = 0; i < count; i += labelStep) {
        const float px = left + (i + 0.5f) * cellWidth;

        DrawAppText(
            TextFormat("%d", i),
            static_cast<int>(px) - 5,
            bottom + 5,
            10,
            C_MUTED
        );
    }

    DrawAppText(
        "Sample index",
        left + static_cast<int>(plotWidth / 2.0f) - 35,
        bottom + 20,
        11,
        C_TEXT
    );

    DrawAppText(
        "Blue: analog samples",
        left,
        y + height - 23,
        12,
        C_BLUE
    );

    DrawAppText(
        "Purple: PCM staircase",
        left + 245,
        y + height - 23,
        12,
        C_PCM_PURPLE
    );
}

} // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        "SignalLab - ITT036 Signal Encoder"
    );

    appFont = LoadFontEx("times.ttf", 32, nullptr, 0);

    if (appFont.texture.id == 0) {
        CloseWindow();
        return 1;
    }

    SetTextureFilter(appFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTargetFPS(60);

    EncodeCurrentBits();

    while (!WindowShouldClose()) {
        const Rectangle inputRect = {45, 170, 280, 42};
        const Rectangle amplitudeRect = {45, 180, 130, 30};
        const Rectangle frequencyRect = {190, 180, 135, 30};
        const Rectangle samplesRect = {45, 230, 130, 30};
        const Rectangle bitsRect = {190, 230, 135, 30};

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const Vector2 mouse = GetMousePosition();

            inputFocused = !analogMode &&
                CheckCollisionPointRec(mouse, inputRect);

            amplitudeFocused = analogMode &&
                CheckCollisionPointRec(mouse, amplitudeRect);

            frequencyFocused = analogMode &&
                CheckCollisionPointRec(mouse, frequencyRect);

            samplesFocused = analogMode &&
                CheckCollisionPointRec(mouse, samplesRect);

            bitsPerSampleFocused = analogMode &&
                CheckCollisionPointRec(mouse, bitsRect);
        }

        // Digital bit input: accept only 0 and 1.
        if (inputFocused && !analogMode) {
            // Ctrl+A clears the current sequence for replacement.
            if ((IsKeyDown(KEY_LEFT_CONTROL) ||
                 IsKeyDown(KEY_RIGHT_CONTROL)) &&
                IsKeyPressed(KEY_A)) {
                bitInput.clear();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && !bitInput.empty()) {
                bitInput.pop_back();
            }

            if (IsKeyPressed(KEY_ENTER)) {
                inputFocused = false;
            }

            int key = GetCharPressed();

            while (key > 0) {
                if ((key == '0' || key == '1') &&
                    bitInput.size() < MAX_BITS) {
                    bitInput += static_cast<char>(key);
                }

                key = GetCharPressed();
            }
        }

        HandleNumericInput(amplitudeInput, amplitudeFocused, true);
        HandleNumericInput(frequencyInput, frequencyFocused, true);
        HandleNumericInput(samplesInput, samplesFocused, false);
        HandleNumericInput(bitsPerSampleInput, bitsPerSampleFocused, false);

        if (IsKeyPressed(KEY_ENTER)) {
            amplitudeFocused = false;
            frequencyFocused = false;
            samplesFocused = false;
            bitsPerSampleFocused = false;
        }

        BeginDrawing();
        ClearBackground(C_BG);

        // Header.
        DrawAppText("SignalLab", 28, 18, 30, C_BLUE);
        DrawAppText(
            "Digital and Analog Signal Encoder",
            28, 54, 16, C_MUTED
        );

        DrawCircle(1035, 34, 5, C_GREEN);
        DrawAppText("Raylib", 1048, 26, 15, C_TEXT);
        DrawLine(25, 85, 1175, 85, C_BORDER);

        // Left panel.
        DrawPanel(25, 100, 340, 590, "");

        if (DrawButton(45, 112, 132, 28, "Digital", !analogMode)) {
            analogMode = false;
            inputFocused = false;
            amplitudeFocused = false;
            frequencyFocused = false;
            samplesFocused = false;
            bitsPerSampleFocused = false;
            showPcmStaircase = false;
            status = "Digital input selected.";
        }

        if (DrawButton(190, 112, 135, 28, "Analog / PCM", analogMode)) {
            analogMode = true;
            inputFocused = false;
            status = "Enter PCM parameters and generate PCM.";
        }

        if (!analogMode) {
            DrawAppText("DIGITAL BITSTREAM", 45, 145, 13, C_MUTED);

            DrawRectangleRounded(
                inputRect, 0.08f, 8,
                inputFocused ? Color{239, 246, 255, 255} : C_BG
            );

            DrawRectangleRoundedLines(
                inputRect, 0.08f, 8,
                inputFocused ? C_BLUE : C_BORDER
            );

            DrawAppText(bitInput, 56, 183, 17, C_TEXT);

            DrawAppText(
                inputFocused
                    ? "Type 0 or 1; Ctrl+A clears; Enter finishes"
                    : "Click field to edit; Ctrl+A replaces all bits",
                45, 218, 12, C_MUTED
            );

            if (DrawButton(45, 245, 132, 36, "Random bits")) {
                GenerateRandomBits(false);
            }

            if (DrawButton(190, 245, 135, 36, "Fixed zero runs")) {
                GenerateRandomBits(true);
            }

            // Apply the manually entered sequence.
            if (DrawButton(45, 289, 280, 30, "Apply entered bits", true)) {
                inputFocused = false;
                EncodeCurrentBits();

                if (hasEncoded) {
                    status = "Manual bit sequence applied successfully.";
                }
            }
        } else {
            DrawAppText("ANALOG INPUT: SINE WAVE", 45, 151, 13, C_MUTED);

            DrawInputField(
                "Amplitude (V)",
                amplitudeRect,
                amplitudeInput,
                amplitudeFocused
            );

            DrawInputField(
                "Frequency (Hz)",
                frequencyRect,
                frequencyInput,
                frequencyFocused
            );

            DrawInputField(
                "Samples",
                samplesRect,
                samplesInput,
                samplesFocused
            );

            DrawInputField(
                "Bits per sample",
                bitsRect,
                bitsPerSampleInput,
                bitsPerSampleFocused
            );

            int samples = 0;
            int bitsPerSample = 0;

            if (ParsePositiveInteger(samplesInput, samples) &&
                ParsePositiveInteger(bitsPerSampleInput, bitsPerSample)) {
                DrawAppText(
                    "PCM output: " +
                        std::to_string(samples * bitsPerSample) + " bits",
                    45, 268, 12, C_PCM_PURPLE
                );
            }

            if (DrawButton(45, 289, 135, 30, "Decode PCM")) {
                DecodePCMCurrentBits();
            }

            if (DrawButton(190, 289, 135, 30, "Generate PCM", true)) {
                GeneratePCM();
            }
        }

        DrawAppText("ENCODING SCHEME", 45, 329, 13, C_MUTED);

        for (int i = 0; i < SCHEME_COUNT; ++i) {
            const int row = i / 2;
            const int col = i % 2;
            const int bx = 45 + col * 145;
            const int by = 351 + row * 34;

            if (DrawButton(
                    bx, by, 135, 30,
                    SCHEMES[i], selectedScheme == i)) {
                selectedScheme = i;
                EncodeCurrentBits();
            }
        }

        if (DrawButton(45, 479, 280, 38, "Encode signal", true)) {
            EncodeCurrentBits();
        }

        if (DrawButton(45, 525, 280, 34, "Export waveform PNG")) {
            if (hasEncoded) {
                TakeScreenshot("waveform.png");
                status = "Saved full-window screenshot as waveform.png.";
            } else {
                status = "Encode a valid bitstream first.";
            }
        }

        if (DrawButton(
                45, 567, 280, 32,
                showPcmStaircase
                    ? "Show line waveform"
                    : "Show PCM staircase")) {
            showPcmStaircase = !showPcmStaircase;
        }

        DrawAppText("STATUS", 45, 604, 13, C_MUTED);
        DrawAppText(status, 45, 624, 11, hasEncoded ? C_GREEN : C_RED);
        DrawAppText("Maximum input: 128 bits", 45, 660, 12, C_MUTED);

        // Main waveform panel.
        DrawPanel(
            385, 100, 790, 410,
            showPcmStaircase ? "PCM staircase signal" : "Encoded waveform"
        );

        if (showPcmStaircase) {
            DrawPCMStaircase(385, 100, 790, 410);
        } else {
            DrawWaveform(385, 100, 790, 410);
        }

        // Results panel.
        DrawPanel(385, 525, 790, 165, "Results");

        DrawAppText("Input mode:", 405, 558, 13, C_MUTED);
        DrawAppText(
            analogMode ? "Analog -> PCM" : "Digital",
            500, 558, 13, C_TEXT
        );

        DrawAppText("Input bits:", 405, 582, 13, C_MUTED);
        DrawAppText(bitInput, 485, 582, 13, C_TEXT);

        DrawAppText("Longest palindrome:", 405, 606, 13, C_MUTED);
        DrawAppText(
            hasEncoded ? palindrome : "--",
            555, 606, 13, C_GREEN
        );

        DrawAppText("Scheme:", 405, 630, 13, C_MUTED);
        DrawAppText(SCHEMES[selectedScheme], 465, 630, 13, C_TEXT);

        DrawAppText("Encoded levels:", 405, 654, 13, C_MUTED);
        DrawAppText(
            hasEncoded ? encodedText : "--",
            510, 654, 13, C_BLUE
        );

        EndDrawing();
    }

    UnloadFont(appFont);
    CloseWindow();
    return 0;
}