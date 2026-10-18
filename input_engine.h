#pragma once

#include <string>
#include <vector>

// ---------- Digital input ----------

// Mode 1: Completely random bit stream.
std::string generateRandomBits(int length);

// Mode 2: Random bit stream containing one fixed run of
// 8 zeros and one fixed run of 4 zeros, where possible.
// The runs do not overlap, and neighbouring bits are forced
// to '1' so the runs are exactly 8 and 4 bits long.
std::string generateBitsWithZeroRuns(int length);

// Returns true if the string is non-empty and contains
// only '0' and '1'.
bool isValidBitString(const std::string& s);


// ---------- Analog input (PCM) ----------

// Analog signal:
// v(t) = A * sin(2*pi*f*t)
//
// Samples the signal over one period.
// Uses uniform quantization with 2^bitsPerSample levels
// over the range [-A, +A].
struct PCMResult {
    std::vector<double> voltages;  // Sampled analog values
    std::vector<int> levels;       // Quantized level per sample
    std::string bits;               // PCM codewords, MSB first

    int bitsPerSample = 0;
    double amplitude = 0.0;
    double frequency = 0.0;
};

PCMResult processPCMAnalogInput(
    double amplitude,
    double frequency,
    int numSamples,
    int bitsPerSample
);


// ---------- PCM decoding ----------

// Converts a PCM bitstream into reconstructed analog voltage samples.
//
// bits: PCM bitstream, with each sample represented by
//       bitsPerSample bits.
// amplitude: Original signal amplitude used during encoding.
//
// Returns reconstructed voltage values, one per decoded sample.
std::vector<double> decodePCM(
    const std::string& bits,
    int bitsPerSample,
    double amplitude
);