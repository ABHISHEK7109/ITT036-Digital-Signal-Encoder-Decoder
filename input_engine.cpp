#define _USE_MATH_DEFINES
#include "input_engine.h"
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// 1. Random Bitstream Generator
string generateRandomBits(int length, bool injectZeros) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    mt19937 gen(seed);
    uniform_int_distribution<int> bitDist(0, 1);
    
    string bitStream = "";
    for (int i = 0; i < length; ++i) {
        bitStream += to_string(bitDist(gen));
    }

    cout << "\n[Generator] Generated Random Bitstream (" << length << " bits): " << bitStream << endl;

    if (injectZeros && length >= 20) {
        bitStream.replace(2, 8, "00000000"); // 8 zeros for B8ZS test
        bitStream.replace(12, 4, "0000");     // 4 zeros for HDB3 test
        cout << "[Generator] Injected zero blocks for scrambling tests." << endl;
    }

    return bitStream;
}

// 2. Analog PCM Sampler with Console Print of Voltages & Bits
string processPCMAnalogInput(int targetLength) {
    cout << "\n[PCM Engine] Sampling Analog Wave V(t) = 5*sin(t) + 5 to generate " << targetLength << " bits..." << endl;
    
    int numSamples = (targetLength + 1) / 2; 

    // Randomize phase shift so wave values change on every execution
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    mt19937 gen(seed);
    uniform_real_distribution<double> phaseDist(0.0, M_PI);
    double phaseShift = phaseDist(gen);

    vector<double> sampledVoltages;
    for (int i = 0; i < numSamples; ++i) {
        double t = i * (2.0 * M_PI / numSamples);
        double voltage = 5.0 * sin(t + phaseShift) + 5.0; 
        sampledVoltages.push_back(voltage);
    }

    string pcmBitStream = "";
    cout << "\n--- PCM Sampled Voltages & Quantized Bit Levels ---" << endl;
    
    // Print each sample's exact voltage and corresponding 2-bit code
    for (size_t i = 0; i < sampledVoltages.size(); ++i) {
        double v = sampledVoltages[i];
        string bits;
        if (v < 2.5) bits = "00";
        else if (v < 5.0) bits = "01";
        else if (v < 7.5) bits = "10";
        else bits = "11";

        pcmBitStream += bits;
        cout << "  Sample " << (i + 1) << ": " << v << " V  -> Quantized Bits: " << bits << endl;
    }

    if ((int)pcmBitStream.length() > targetLength) {
        pcmBitStream = pcmBitStream.substr(0, targetLength);
    }

    return pcmBitStream;
}