#pragma once
#include <string>
#include <vector>

// A signal is a vector of voltage levels: +1, 0, -1.
// NRZ-L, NRZ-I, AMI, B8ZS, HDB3      -> 1 level per bit
// Manchester, Differential Manchester -> 2 levels per bit (first half, second half)
using Levels  = std::vector<int>;
using Encoder = Levels (*)(const std::string&);

// Result of a scrambling scheme: plain AMI vs scrambled signal.
// substituted[i] == 1 where level i belongs to a substituted block (for highlighting).
struct ScrambleResult {
    Levels ami;
    Levels scrambled;
    std::vector<int> substituted;
};
using Scrambler = ScrambleResult (*)(const std::string&);

// Line coding
Levels encodeNRZL(const std::string& bits);
Levels encodeNRZI(const std::string& bits);
Levels encodeManchester(const std::string& bits);
Levels encodeDiffManchester(const std::string& bits);
Levels encodeAMI(const std::string& bits);

// Scrambling (both O(n), single pass)
ScrambleResult encodeB8ZS(const std::string& bits);
ScrambleResult encodeHDB3(const std::string& bits);

// Display helper: +1 -> '+', 0 -> '0', -1 -> '-'
std::string levelsToString(const Levels& levels);