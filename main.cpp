#include <iostream>
#include <string>
#include "input_engine.h"
#include "palindrome.h"

using namespace std;

int main() {
    cout << "=====================================================" << endl;
    cout << "      ITT036 DIGITAL SIGNAL ENCODER / DECODER       " << endl;
    cout << "=====================================================" << endl;

    cout << "\nSelect Input Signal Type:" << endl;
    cout << "1. Digital Input (Random Bit Stream)" << endl;
    cout << "2. Analog Input (PCM Quantization)" << endl;
    cout << "Choice (1 or 2): ";
    
    int choice;
    cin >> choice;

    // Ask user for custom bit length between 20 and 32
    int bitLength = 20;
    cout << "Enter bitstream length (20 to 32): ";
    cin >> bitLength;
    if (bitLength < 20) bitLength = 20;
    if (bitLength > 32) bitLength = 32;

    string digitalData = "";

    if (choice == 1) {
        cout << "Inject zero blocks for scrambling tests? (1 = Yes, 0 = No): ";
        int testZeroChoice;
        cin >> testZeroChoice;

        digitalData = generateRandomBits(bitLength, testZeroChoice == 1);
    } 
    else if (choice == 2) {
        digitalData = processPCMAnalogInput(bitLength);
        cout << "\n[Output 1] PCM Digital Bit Stream: " << digitalData << endl;
    }

    // Run Manacher's Algorithm
    if (!digitalData.empty()) {
        string longestPal = findLongestPalindromeManacher(digitalData);
        cout << "\n[Output 2] Longest Palindromic Sequence: " << longestPal << endl;
        cout << "[Output 2] Length: " << longestPal.length() << " bits (Manacher's O(N))" << endl;
    }

    return 0;
}