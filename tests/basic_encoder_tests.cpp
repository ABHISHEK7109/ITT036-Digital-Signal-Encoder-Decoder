#include "encoders.h"

#include <iostream>
#include <string>

using namespace std;

int passed = 0;
int failed = 0;

void test(
    const string& name,
    const string& actual,
    const string& expected
) {
    cout << "\nTest: " << name << '\n';
    cout << "Expected: " << expected << '\n';
    cout << "Actual:   " << actual << '\n';

    if (actual == expected) {
        cout << "Result: PASS\n";
        ++passed;
    } else {
        cout << "Result: FAIL\n";
        ++failed;
    }
}

int main() {
    cout << "====================================\n";
    cout << " BASIC LINE ENCODER TESTS\n";
    cout << "====================================\n";

    // NRZ-L: 0 = high (+), 1 = low (-).
    test(
        "NRZ-L",
        levelsToString(encodeNRZL("0101")),
        "+-+-"
    );

    // NRZ-I: 1 causes a transition; 0 keeps the level.
    // Initial level is positive.
    test(
        "NRZ-I",
        levelsToString(encodeNRZI("0101")),
        "+--+"
    );

    // Manchester: 0 = -+, 1 = +-.
    test(
        "Manchester",
        levelsToString(encodeManchester("01")),
        "-++-"
    );

    // Differential Manchester:
    // 0 = transition at the start of the bit.
    // 1 = no transition at the start.
    // Every bit has a mid-bit transition.
    // Initial reference level is positive.
    test(
        "Differential Manchester",
        levelsToString(encodeDiffManchester("01")),
        "-++-"
    );

    // AMI: 0 = zero level; successive 1s alternate polarity.
    test(
        "AMI",
        levelsToString(encodeAMI("101101")),
        "+0-+0-"
    );

    cout << "\n====================================\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "====================================\n";

    return failed == 0 ? 0 : 1;
}