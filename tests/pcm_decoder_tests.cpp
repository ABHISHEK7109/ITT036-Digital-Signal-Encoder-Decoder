
#include "input_engine.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

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
    cout << " PCM DECODER TESTS\n";
    cout << "====================================\n";

    // Four-bit codewords: levels 0, 1, and 15.
    vector<double> decoded = decodePCM("000000011111", 4, 5.0);

    report(
        "Decodes three four-bit codewords",
        decoded.size() == 3
    );

    report(
        "Reconstructs level zero correctly",
        decoded.size() == 3 &&
        abs(decoded[0] - (-4.6875)) < 1e-9
    );

    report(
        "Reconstructs level one correctly",
        decoded.size() == 3 &&
        abs(decoded[1] - (-4.0625)) < 1e-9
    );

    report(
        "Reconstructs maximum level correctly",
        decoded.size() == 3 &&
        abs(decoded[2] - 4.6875) < 1e-9
    );

    report(
        "Rejects an incomplete codeword",
        decodePCM("101", 4, 5.0).empty()
    );

    report(
        "Rejects invalid binary characters",
        decodePCM("10x0", 4, 5.0).empty()
    );

    report(
        "Rejects invalid bits-per-sample",
        decodePCM("0000", 0, 5.0).empty()
    );

    report(
        "Rejects non-positive amplitude",
        decodePCM("0000", 4, 0.0).empty()
    );

    cout << "\n====================================\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "====================================\n";

    return failed == 0 ? 0 : 1;
}
