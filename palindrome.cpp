#include "palindrome.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// Manacher's Algorithm for finding Longest Palindromic Substring in O(N) time
string findLongestPalindromeManacher(const string& s) {
    if (s.empty()) return "";

    // Transform string "101" -> "^#1#0#1#$" to handle even/odd lengths uniformly
    string T = "^";
    for (char c : s) {
        T += "#";
        T += c;
    }
    T += "#$";

    int n = T.length();
    vector<int> P(n, 0); // Palindrome radii array
    int C = 0, R = 0;     // Center and right boundary

    for (int i = 1; i < n - 1; i++) {
        int i_mirror = 2 * C - i;

        if (R > i) {
            P[i] = min(R - i, P[i_mirror]);
        }

        // Expand palindrome centered at i
        while (T[i + 1 + P[i]] == T[i - 1 - P[i]]) {
            P[i]++;
        }

        // Update center and right boundary if expansion extends past R
        if (i + P[i] > R) {
            C = i;
            R = i + P[i];
        }
    }

    // Locate maximum radius and center index
    int maxLen = 0;
    int centerIndex = 0;
    for (int i = 1; i < n - 1; i++) {
        if (P[i] > maxLen) {
            maxLen = P[i];
            centerIndex = i;
        }
    }

    int start = (centerIndex - 1 - maxLen) / 2;
    return s.substr(start, maxLen);
}