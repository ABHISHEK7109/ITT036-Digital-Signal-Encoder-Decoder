#include "input_engine.h"

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

int passed = 0;
int failed = 0;

void report(const string& name, bool success) {
    cout << "\nTest: " << name << '\n';

    if (success) {
        cout << "Result: PASS\n";
        ++passed;
    } else {
        cout << "Result: FAIL\n";
        ++failed;
    }
}

int main() {
    cout << "====================================\n";
    cout << " INPUT ENGINE TESTS\n";
    cout << "====================================\n";

    // Test 1: Random bit generation.
    string randomBits = generateRandomBits(32);

    report(
        "Random generator returns 32 valid bits",
        randomBits.size() == 32 && isValidBitString(randomBits)
    );

    // Test 2: Fixed zero-run generation.
    string fixedBits = generateBitsWithZeroRuns(32);

    bool valid = fixedBits.size() == 32 &&
                 isValidBitString(fixedBits);

    report("Fixed-run generator returns valid bits", valid);

    // Test 3: Check for eight consecutive zeros.
    report(
        "Contains eight consecutive zeros",
        fixedBits.find("00000000") != string::npos
    );

    // Test 4: Check for four consecutive zeros.
    report(
        "Contains four consecutive zeros",
        fixedBits.find("0000") != string::npos
    );

    // Test 5: Bit-string validation.
    report(
        "Rejects invalid bit strings",
        !isValidBitString("10102") &&
        !isValidBitString("") &&
        isValidBitString("10101")
    );

    // Test 6: PCM generation.
    PCMResult pcm = processPCMAnalogInput(5.0, 1.0, 16, 4);

    report(
        "PCM generates 16 samples",
        pcm.voltages.size() == 16 &&
        pcm.levels.size() == 16
    );

    // Test 7: Four bits per sample means 64 output bits.
    report(
        "PCM generates 64 bits",
        pcm.bits.size() == 64 &&
        isValidBitString(pcm.bits)
    );

    // Test 8: Quantization levels are in range [0, 15].
    bool levelsValid = all_of(
        pcm.levels.begin(),
        pcm.levels.end(),
        [](int level) {
            return level >= 0 && level <= 15;
        }
    );

    report("PCM quantization levels are valid", levelsValid);

    cout << "\n====================================\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "====================================\n";

    return failed == 0 ? 0 : 1;
}
