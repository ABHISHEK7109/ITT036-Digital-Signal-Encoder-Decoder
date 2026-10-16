#pragma once
#include <string>

// Declarations for bitstream generation and PCM sampling
std::string generateRandomBits(int length,bool injectZeros = false);
std::string processPCMAnalogInput(int targetLength);