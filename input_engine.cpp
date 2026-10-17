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

// Put `len` zeros at `pos` and force the neighbouring bits to '1'
void placeZeroRun(string& s, int pos, int len) {
    for (int k = 0; k < len; ++k) s[pos + k] = '0';
    const int before = pos - 1, after = pos + len;
    if (before >= 0)              s[before] = '1';
    if (after < (int)s.size())    s[after]  = '1';
}
}

string generateRandomBits(int length) {
    uniform_int_distribution<int> bitDist(0, 1);
    string s(length, '0');
    for (char& c : s) c = static_cast<char>('0' + bitDist(rng()));
    return s;
}

string generateBitsWithZeroRuns(int length) {
    string s = generateRandomBits(length);
    if (length < 8) return s;

    // 8-zero run at a random position
    uniform_int_distribution<int> pos8(0, length - 8);
    const int p8 = pos8(rng());
    placeZeroRun(s, p8, 8);

    // 4-zero run at a random position that keeps a 1-bit gap from the 8-run
    vector<int> candidates;
    for (int p = 0; p + 4 <= length; ++p)
        if (p + 4 < p8 || p > p8 + 8) candidates.push_back(p);
    if (!candidates.empty()) {
        uniform_int_distribution<size_t> pick(0, candidates.size() - 1);
        const int p4 = candidates[pick(rng())];
        placeZeroRun(s, p4, 4);   // gap >= 1 bit, so it never touches the 8-run
    }
    return s;
}

bool isValidBitString(const string& s) {
    return !s.empty() &&
           all_of(s.begin(), s.end(), [](char c) { return c == '0' || c == '1'; });
}

PCMResult processPCMAnalogInput(double amplitude, double frequency,
                                int numSamples, int bitsPerSample) {
    PCMResult r;
    r.amplitude = amplitude;
    r.frequency = frequency;
    r.bitsPerSample = bitsPerSample;

    const int numLevels = 1 << bitsPerSample;
    const double step = (2.0 * amplitude) / numLevels;          // quantization step

    for (int i = 0; i < numSamples; ++i) {
        const double t = static_cast<double>(i) / (numSamples * frequency); // 1 period
        const double v = amplitude * sin(2.0 * M_PI * frequency * t);
        int level = static_cast<int>((v + amplitude) / step);
        level = min(max(level, 0), numLevels - 1);

        r.voltages.push_back(v);
        r.levels.push_back(level);
        for (int b = bitsPerSample - 1; b >= 0; --b)             // MSB first
            r.bits += static_cast<char>('0' + ((level >> b) & 1));
    }
    return r;
}