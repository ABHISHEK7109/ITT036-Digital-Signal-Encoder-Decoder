#include "encoders.h"

using namespace std;

// Conventions (also stated in the report):
//  NRZ-L    : 0 = high (+), 1 = low (-)
//  NRZ-I    : 1 = transition at start of bit, 0 = none; level before first bit = high
//  Manchester: 0 = low->high (-+), 1 = high->low (+-)
//  Diff. Manchester: 0 = transition at start of bit, 1 = none; always a mid-bit
//                    transition; level before first bit = high
//  AMI      : 0 = zero level, 1 = alternating +/-; the first 1 is '+'

namespace {
inline int bitOf(char c) { return c - '0'; }
}

Levels encodeNRZL(const string& bits) {
    Levels out;
    out.reserve(bits.size());
    for (char c : bits) out.push_back(1 - 2 * bitOf(c));      // 0 -> +1, 1 -> -1
    return out;
}

Levels encodeNRZI(const string& bits) {
    Levels out;
    out.reserve(bits.size());
    int level = 1;                                             // previous level: high
    for (char c : bits) {
        level *= 1 - 2 * bitOf(c);                             // 1 flips, 0 keeps
        out.push_back(level);
    }
    return out;
}

Levels encodeManchester(const string& bits) {
    Levels out;
    out.reserve(bits.size() * 2);
    for (char c : bits) {
        int first = 2 * bitOf(c) - 1;                          // 0 -> -1, 1 -> +1
        out.push_back(first);
        out.push_back(-first);
    }
    return out;
}

Levels encodeDiffManchester(const string& bits) {
    Levels out;
    out.reserve(bits.size() * 2);
    int level = 1;                                             // level at end of previous bit
    for (char c : bits) {
        level *= 2 * bitOf(c) - 1;                             // 0 -> flip at start, 1 -> keep
        out.push_back(level);
        level = -level;                                        // mid-bit transition
        out.push_back(level);
    }
    return out;
}

Levels encodeAMI(const string& bits) {
    Levels out;
    out.reserve(bits.size());
    int last = -1;                                             // so the first 1 becomes +
    for (char c : bits) {
        int b = bitOf(c);
        last *= 1 - 2 * b;                                     // flip polarity only on a 1
        out.push_back(b * last);                               // 0 -> 0, 1 -> +/-
    }
    return out;
}

// ---------------------------------------------------------------------------
// Scrambling: one table-driven O(n) routine shared by B8ZS and HDB3.
// A rule describes the zero-run length and the substitution patterns, written
// as multipliers of the polarity of the last pulse ("last"):
//   pattern[sel][k] * last  = k-th level of the substituted block
//   lastMul[sel]            = polarity of the last pulse after the block
// sel = parity of pulses since last substitution (HDB3) or 0 (B8ZS).
// ---------------------------------------------------------------------------
namespace {
struct ScrambleRule {
    int run;
    int useParity;           // 1 -> pattern depends on pulse-count parity
    int pattern[2][8];
    int lastMul[2];
};

// B8ZS: 000VB0VB  -> with last = +:  0 0 0 + - 0 - +   (last pulse unchanged)
const ScrambleRule B8ZS_RULE = { 8, 0,
    { {0,0,0, 1,-1,0,-1, 1},
      {0,0,0, 1,-1,0,-1, 1} },
    { 1, 1 } };

// HDB3:
// Even number of nonzero pulses since the last violation -> 000V
// Odd number of nonzero pulses since the last violation  -> B00V
//
// B is opposite to the preceding pulse.
// V is a violation pulse with the same polarity as the preceding pulse.
const ScrambleRule HDB3_RULE = { 4, 1,
    { { 0, 0, 0, 1, 0, 0, 0, 0 },
      {-1, 0, 0,-1, 0, 0, 0, 0} },
    { 1, -1 } };

ScrambleResult scramble(const string& bits, const ScrambleRule& rule) {
    const size_t n = bits.size();

    // zeroRun[i] = number of consecutive '0' starting at i  (computed right-to-left, O(n))
    vector<int> zeroRun(n + 1, 0);
    for (size_t i = n; i-- > 0;)
        zeroRun[i] = (bits[i] == '0') ? zeroRun[i + 1] + 1 : 0;

    ScrambleResult res;
    res.ami = encodeAMI(bits);
    res.scrambled.assign(n, 0);
    res.substituted.assign(n, 0);

    int last = -1;            // polarity of last pulse (virtual '-' before the stream)
    int pulsesSinceSub = 0;

    size_t i = 0;
    while (i < n) {
        if (zeroRun[i] >= rule.run) {
            int sel = (pulsesSinceSub & 1) * rule.useParity;
            for (int k = 0; k < rule.run; ++k) {
                res.scrambled[i + k]   = rule.pattern[sel][k] * last;
                res.substituted[i + k] = 1;
            }
            last *= rule.lastMul[sel];
            pulsesSinceSub = 0;
            i += rule.run;
        } else {
            int b = bitOf(bits[i]);
            last *= 1 - 2 * b;
            res.scrambled[i] = b * last;
            pulsesSinceSub += b;
            ++i;
        }
    }
    return res;
}
} // namespace

ScrambleResult encodeB8ZS(const string& bits) { return scramble(bits, B8ZS_RULE); }
ScrambleResult encodeHDB3(const string& bits) { return scramble(bits, HDB3_RULE); }

string levelsToString(const Levels& levels) {
    static const char SYMBOL[] = {'-', '0', '+'};
    string s;
    s.reserve(levels.size());
    for (int v : levels) s += SYMBOL[v + 1];
    return s;
}