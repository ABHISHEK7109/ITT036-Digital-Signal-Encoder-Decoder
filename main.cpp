#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include "encoders.h"
#include "input_engine.h"
#include "palindrome.h"

using namespace std;

// ----------------------------------------------------------------------------
// Input helpers (validated; no unchecked cin)
// ----------------------------------------------------------------------------
static void clearLine() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static int readInt(const string& prompt, int lo, int hi) {
    for (;;) {
        cout << prompt;
        int v;
        if (cin >> v && v >= lo && v <= hi) { clearLine(); return v; }
        if (cin.eof()) exit(0);
        clearLine();
        cout << "  Please enter a number between " << lo << " and " << hi << ".\n";
    }
}

static double readDouble(const string& prompt, double lo, double hi) {
    for (;;) {
        cout << prompt;
        double v;
        if (cin >> v && v >= lo && v <= hi) { clearLine(); return v; }
        if (cin.eof()) exit(0);
        clearLine();
        cout << "  Please enter a value between " << lo << " and " << hi << ".\n";
    }
}

static bool readYesNo(const string& prompt) {
    for (;;) {
        cout << prompt << " (y/n): ";
        string s;
        if (!(cin >> s)) exit(0);
        clearLine();
        const char c = static_cast<char>(tolower(static_cast<unsigned char>(s[0])));
        if (c == 'y' || c == 'n') return c == 'y';   // single check, not a chain
        cout << "  Please answer y or n.\n";
    }
}

static string readBitString() {
    for (;;) {
        cout << "Enter bit stream (0s and 1s only): ";
        string s;
        if (!(cin >> s)) exit(0);
        clearLine();
        if (isValidBitString(s)) return s;
        cout << "  Invalid. Use only 0 and 1.\n";
    }
}

// Print a numbered menu from a table and return the chosen key
template <typename Entry>
static int chooseFrom(const string& title, const map<int, Entry>& table,
                      const function<string(const Entry&)>& label) {
    cout << "\n" << title << "\n";
    for (const auto& kv : table) cout << "  " << kv.first << ". " << label(kv.second) << "\n";
    return readInt("Choice: ", table.begin()->first, table.rbegin()->first);
}

// ----------------------------------------------------------------------------
// Input sources (tables replace switch / if-else chains)
// ----------------------------------------------------------------------------
struct InputSource {
    const char* name;
    function<string()> run;      // returns the digital bit stream
};

static string digitalRandom() {
    return generateRandomBits(readInt("Number of bits (8-64): ", 8, 64));
}
static string digitalWithZeroRuns() {
    return generateBitsWithZeroRuns(readInt("Number of bits (12-64): ", 12, 64));
}
static string digitalManual() { return readBitString(); }

static string analogPCM() {
    const double amp  = readDouble("Analog amplitude A in volts (0.5-20): ", 0.5, 20.0);
    const double freq = readDouble("Frequency f in Hz (1-1000): ", 1.0, 1000.0);
    const int samples = readInt("Number of samples over one period (4-16): ", 4, 16);
    const int nbits   = readInt("Bits per sample (1-4): ", 1, 4);

    PCMResult pcm = processPCMAnalogInput(amp, freq, samples, nbits);
    cout << "\n--- PCM: sampled voltages and quantized codewords ---\n";
    for (size_t i = 0; i < pcm.voltages.size(); ++i)
        cout << "  Sample " << i + 1 << ": " << pcm.voltages[i] << " V -> level "
             << pcm.levels[i] << " -> " << pcm.bits.substr(i * nbits, nbits) << "\n";
    return pcm.bits;
}

static string digitalMenu() {
    static const map<int, InputSource> modes = {
        {1, {"Random bit stream",                         digitalRandom}},
        {2, {"Random stream with fixed 8-zero / 4-zero runs", digitalWithZeroRuns}},
        {3, {"Enter bit stream manually",                 digitalManual}},
    };
    const int c = chooseFrom<InputSource>("Digital data generation:", modes,
                                          [](const InputSource& s) { return string(s.name); });
    return modes.at(c).run();
}

// ----------------------------------------------------------------------------
// Encoders
// ----------------------------------------------------------------------------
struct EncoderEntry { const char* name; Encoder fn; bool isAMI; };
struct ScramblerEntry { const char* name; Scrambler fn; };

static const map<int, EncoderEntry> ENCODERS = {
    {1, {"NRZ-L",                   encodeNRZL,          false}},
    {2, {"NRZ-I",                   encodeNRZI,          false}},
    {3, {"Manchester",              encodeManchester,    false}},
    {4, {"Differential Manchester", encodeDiffManchester, false}},
    {5, {"AMI",                     encodeAMI,           true }},
};

static const map<int, ScramblerEntry> SCRAMBLERS = {
    {1, {"B8ZS", encodeB8ZS}},
    {2, {"HDB3", encodeHDB3}},
};

int main() {
    cout << "=====================================================\n"
         << "      ITT036 DIGITAL SIGNAL ENCODER / DECODER\n"
         << "=====================================================\n";

    // 1. Input type
    static const map<int, InputSource> inputTypes = {
        {1, {"Digital input", digitalMenu}},
        {2, {"Analog input (PCM)", analogPCM}},
    };
    const int type = chooseFrom<InputSource>("Select input signal type:", inputTypes,
                                             [](const InputSource& s) { return string(s.name); });
    const string data = inputTypes.at(type).run();

    // 2. Outputs: data stream + longest palindromic substring
    cout << "\n[Output 1] Digital data stream (" << data.size() << " bits): " << data << "\n";
    const string pal = findLongestPalindromeManacher(data);
    cout << "[Output 2] Longest palindromic substring: " << pal
         << "  (length " << pal.size() << ", Manacher O(n))\n";

    // 3. Line coding
    const int enc = chooseFrom<EncoderEntry>("Select line coding scheme:", ENCODERS,
                                             [](const EncoderEntry& e) { return string(e.name); });
    const EncoderEntry& chosen = ENCODERS.at(enc);

    cout << "\n--- Results ---\n";
    cout << "Scheme: " << chosen.name << "\n";

    // AMI -> optional scrambling -> type of scrambling (flags/tables, no chains)
    const bool scramble = chosen.isAMI && readYesNo("Is scrambling needed?");

    function<void()> plain = [&]() {
        const Levels sig = chosen.fn(data);
        cout << "[Output 3] Encoded signal levels: " << levelsToString(sig) << "\n";
    };
    function<void()> scrambled = [&]() {
        const int s = chooseFrom<ScramblerEntry>("Select scrambling type:", SCRAMBLERS,
                                                 [](const ScramblerEntry& e) { return string(e.name); });
        const ScramblerEntry& sc = SCRAMBLERS.at(s);
        const ScrambleResult r = sc.fn(data);
        cout << "[Output 3] Plain AMI signal:        " << levelsToString(r.ami) << "\n";
        cout << "[Output 4] " << sc.name << " scrambled signal: " << levelsToString(r.scrambled) << "\n";
        string marks;
        for (int m : r.substituted) marks += ".^"[m];
        cout << "           substituted blocks:      " << marks << "\n";
    };
    const map<bool, function<void()>> run = {{false, plain}, {true, scrambled}};
    run.at(scramble)();

    // TODO (next steps): OpenGL rendering, glReadPixels, image decoder, PCM staircase
    return 0;
}