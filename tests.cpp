
#include "encoders.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct TestCase {
    string name;
    string input;
    string expected;
    bool hdb3;
};

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

void testExpected(const TestCase& test) {
    ScrambleResult result = test.hdb3
        ? encodeHDB3(test.input)
        : encodeB8ZS(test.input);

    string actual = levelsToString(result.scrambled);

    cout << "\nTest: " << test.name << '\n';
    cout << "Input:    " << test.input << '\n';
    cout << "Expected: " << test.expected << '\n';
    cout << "Actual:   " << actual << '\n';

    bool success =
        actual == test.expected &&
        result.scrambled.size() == test.input.size() &&
        result.substituted.size() == test.input.size();

    report(test.name, success);
}

void testNoSubstitution(
    const string& name,
    const string& input,
    bool hdb3
) {
    ScrambleResult result = hdb3
        ? encodeHDB3(input)
        : encodeB8ZS(input);

    Levels ordinaryAMI = encodeAMI(input);

    bool success =
        result.scrambled == ordinaryAMI &&
        result.scrambled.size() == input.size() &&
        result.substituted.size() == input.size();

    cout << "\nInput: " << input << '\n';
    cout << "Signal: " << levelsToString(result.scrambled) << '\n';

    report(name, success);
}

void testMultipleRuns(
    const string& name,
    const string& input,
    bool hdb3,
    size_t expectedMarked
) {
    ScrambleResult result = hdb3
        ? encodeHDB3(input)
        : encodeB8ZS(input);

    size_t marked = 0;
    for (int value : result.substituted) {
        if (value == 1) {
            ++marked;
        }
    }

    bool success =
        result.scrambled.size() == input.size() &&
        result.substituted.size() == input.size() &&
        marked == expectedMarked;

    cout << "\nInput length: " << input.size() << '\n';
    cout << "Output length: " << result.scrambled.size() << '\n';
    cout << "Marked positions: " << marked << '\n';
    cout << "Expected marked positions: " << expectedMarked << '\n';

    report(name, success);
}

int main() {
    cout << "====================================\n";
    cout << " B8ZS / HDB3 AUTOMATED TESTS\n";
    cout << "====================================\n";

    // Previously verified reference cases.
    const vector<TestCase> referenceTests = {
       {"B8ZS: eight zeros between pulses",
 "1000000001", "+000+-0-+-", false},

        {"HDB3: zero run at beginning",
         "000010", "000-+0", true},

        {"HDB3: odd pulse count",
         "100001", "+-00-+", true},

        {"HDB3: even pulse count",
         "1100001", "+-000-+", true}
    };

    for (const auto& test : referenceTests) {
        testExpected(test);
    }

    cout << "\n--- Edge cases ---\n";

    // No eligible zero run.
    testNoSubstitution(
        "B8ZS: no eight-zero run",
        "1010101", false
    );

    testNoSubstitution(
        "HDB3: no four-zero run",
        "1010101", true
    );

    // Input shorter than the B8ZS substitution length.
    testNoSubstitution(
        "B8ZS: short input",
        "1000000", false
    );

    // Two separate eligible zero runs.
    testMultipleRuns(
        "B8ZS: two eight-zero runs",
        "1000000001000000001",
        false,
        16
    );

    testMultipleRuns(
        "HDB3: two four-zero runs",
        "10000100001",
        true,
        8
    );

    cout << "\n====================================\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "====================================\n";

    return failed == 0 ? 0 : 1;
}
