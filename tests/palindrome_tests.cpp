#include "palindrome.h"

#include <iostream>
#include <string>

using namespace std;

int passed = 0;
int failed = 0;

void test(const string& input, const string& expected) {
    string actual = findLongestPalindromeManacher(input);

    cout << "\nInput:    \"" << input << "\"\n";
    cout << "Expected: \"" << expected << "\"\n";
    cout << "Actual:   \"" << actual << "\"\n";

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
    cout << " LONGEST PALINDROME TESTS\n";
    cout << "====================================\n";

    test("10000001", "10000001");
    test("10101", "10101");
    test("1111", "1111");
    test("101100", "0110");
    test("12321", "12321");
    test("abcdef", "a");
    test("0", "0");
    test("", "");

    cout << "\n====================================\n";
    cout << "Passed: " << passed << '\n';
    cout << "Failed: " << failed << '\n';
    cout << "====================================\n";

    return failed == 0 ? 0 : 1;
}