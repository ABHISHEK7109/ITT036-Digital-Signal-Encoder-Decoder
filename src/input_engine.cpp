
#define _USE_MATH_DEFINES
#include "input_engine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

namespace {

mt19937& rng() {
    static mt19937 gen(static_cast<unsigned>(
        chrono::system_clock::now().time_since_epoch().count()));

    return gen;
}

// Place a run of zeros and force its neighbouring bits to 1.
void placeZeroRun(string& s, int pos, int len) {
    for (int k = 0; k < len; ++k) {
        s[pos + k] = '0';
    }

    const int before = pos - 1;
    const int after = pos + len;

    if (before >= 0) {
        s[before] = '1';
    }

    if (after < static_cast<int>(s.size())) {
        s[after] = '1';
    }
}

} // namespace


// ============================================================
// DIGITAL INPUT GENERATION
// ============================================================

// Generate a random sequence of bits.
string generateRandomBits(int length) {
    if (length <= 0) {
        return "";
    }

    uniform_int_distribution<int> bitDist(0, 1);

    string s(length, '0');

    for (char& c : s) {
        c = static_cast<char>('0' + bitDist(rng()));
    }

    return s;
}


// Generate a random sequence with an eight-zero run and,
// when space permits, a separate four-zero run.
string generateBitsWithZeroRuns(int length) {
    if (length <= 0) {
        return "";
    }

    string s = generateRandomBits(length);

    if (length < 8) {
        return s;
    }

    // Place eight consecutive zeros.
    uniform_int_distribution<int> pos8(0, length - 8);

    const int p8 = pos8(rng());

    placeZeroRun(s, p8, 8);

    // Find positions for a separate four-zero run.
    // Keep at least one bit between the two runs.
    vector<int> candidates;

    for (int p = 0; p + 4 <= length; ++p) {
        if (p + 4 < p8 || p > p8 + 8) {
            candidates.push_back(p);
        }
    }

    if (!candidates.empty()) {
        uniform_int_distribution<size_t> pick(
            0, candidates.size() - 1
        );

        const int p4 = candidates[pick(rng())];

        placeZeroRun(s, p4, 4);
    }

    return s;
}


// Check whether a non-empty string contains only binary digits.
bool isValidBitString(const string& s) {
    return !s.empty() &&
           all_of(
               s.begin(),
               s.end(),
               [](char c) {
                   return c == '0' || c == '1';
               }
           );
}


// ============================================================
// PCM ENCODER
// ============================================================

// Sample a sine wave, quantize each sample, and convert
// each quantization level into a binary PCM codeword.
//
// bitsPerSample determines the number of quantization levels:
// 2^bitsPerSample.
//
// Codewords are stored most-significant bit first.
PCMResult processPCMAnalogInput(
    double amplitude,
    double frequency,
    int numSamples,
    int bitsPerSample)
{
    PCMResult r;

    // Validate parameters before calculating quantization levels.
    if (!isfinite(amplitude) ||
        !isfinite(frequency) ||
        amplitude <= 0.0 ||
        frequency <= 0.0 ||
        numSamples <= 0 ||
        bitsPerSample < 1 ||
        bitsPerSample > 16) {
        return r;
    }

    r.amplitude = amplitude;
    r.frequency = frequency;
    r.bitsPerSample = bitsPerSample;

    const int numLevels = 1 << bitsPerSample;

    // Width of each quantization interval.
    const double step = (2.0 * amplitude) / numLevels;

    for (int i = 0; i < numSamples; ++i) {

        // Sample the sine wave over a fixed one-second interval.
// Changing frequency now changes the number of cycles sampled.
const double t =
    static_cast<double>(i) / numSamples;

const double v =
    amplitude * sin(2.0 * M_PI * frequency * t);
        // Map the sample voltage to a quantization level.
        int level = static_cast<int>(
            (v + amplitude) / step
        );

        // Ensure that the level is within the valid range.
        level = min(max(level, 0), numLevels - 1);

        r.voltages.push_back(v);
        r.levels.push_back(level);

        // Convert the level to a binary codeword, MSB first.
        for (int b = bitsPerSample - 1; b >= 0; --b) {
            r.bits += static_cast<char>(
                '0' + ((level >> b) & 1)
            );
        }
    }

    return r;
}


// ============================================================
// PCM DECODER
// ============================================================

// Decode a PCM bitstream into reconstructed analog voltages.
//
// Each group of bitsPerSample bits represents one quantization
// level. The decoder reconstructs the midpoint voltage of the
// corresponding quantization interval.
//
// Returns an empty vector if the parameters or bitstream
// are invalid.
vector<double> decodePCM(
    const string& bits,
    int bitsPerSample,
    double amplitude)
{
    // Validate the parameters.
    if (bits.empty() ||
        bitsPerSample < 1 ||
        bitsPerSample > 16 ||
        !isfinite(amplitude) ||
        amplitude <= 0.0) {
        return {};
    }

    // The bitstream must contain complete codewords.
    if (bits.size() %
        static_cast<size_t>(bitsPerSample) != 0) {
        return {};
    }

    // Reject invalid characters.
    if (!isValidBitString(bits)) {
        return {};
    }

    const int numLevels = 1 << bitsPerSample;

    // Voltage width of one quantization interval.
    const double step = (2.0 * amplitude) / numLevels;

    vector<double> reconstructedSamples;

    reconstructedSamples.reserve(
        bits.size() / static_cast<size_t>(bitsPerSample)
    );

    // Decode one PCM codeword at a time.
    for (size_t i = 0;
         i < bits.size();
         i += static_cast<size_t>(bitsPerSample)) {

        int level = 0;

        // Convert the codeword from binary to an integer level.
        // Bits are read most-significant bit first.
        for (int j = 0; j < bitsPerSample; ++j) {
            level = (level << 1) + (bits[i + j] - '0');
        }

        // Reconstruct the midpoint of the quantization interval.
        const double voltage =
            -amplitude + (level + 0.5) * step;

        reconstructedSamples.push_back(voltage);
    }

    return reconstructedSamples;
}

