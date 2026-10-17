#pragma once
#include <string>
#include <vector>

// ---------- Digital input ----------
// Mode 1: completely random bit stream
std::string generateRandomBits(int length);
// Mode 2: random bit stream containing one fixed run of 8 zeros and one fixed run of 4 zeros
//         (random, non-overlapping positions; neighbouring bits are forced to '1'
//          so the runs are exactly 8 and 4 long)
std::string generateBitsWithZeroRuns(int length);
// true if s is non-empty and contains only '0' / '1'
bool isValidBitString(const std::string& s);

// ---------- Analog input (PCM) ----------
// Analog signal: v(t) = A * sin(2*pi*f*t), sampled N times over one period.
// Quantizer: uniform, 2^bitsPerSample levels over [-A, +A].
struct PCMResult {
    std::vector<double> voltages;   // sampled analog values
    std::vector<int>    levels;     // quantized level index per sample
    std::string         bits;       // concatenated codewords (MSB first)
    int    bitsPerSample = 0;
    double amplitude     = 0.0;
    double frequency     = 0.0;
};

PCMResult processPCMAnalogInput(double amplitude, double frequency,
                                int numSamples, int bitsPerSample);