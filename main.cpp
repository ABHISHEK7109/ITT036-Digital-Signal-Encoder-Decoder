
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <string>

#include "encoders.h"
#include "input_engine.h"
#include "palindrome.h"
#include "waveform_renderer.h"

using namespace std;

// ----------------------------------------------------------------------------
// Input helpers
// ----------------------------------------------------------------------------
static void clearLine() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static int readInt(const string& prompt, int lo, int hi) {
    for (;;) {
        cout << prompt;

        int v;
        if (cin >> v && v >= lo && v <= hi) {
            clearLine();
            return v;
        }

        if (cin.eof()) {
            exit(0);
        }

        clearLine();
        cout << "  Please enter a number between "
             << lo << " and " << hi << ".\n";
    }
}

static double readDouble(const string& prompt, double lo, double hi) {
    for (;;) {
        cout << prompt;

        double v;
        if (cin >> v && v >= lo && v <= hi) {
            clearLine();
            return v;
        }

        if (cin.eof()) {
            exit(0);
        }

        clearLine();
        cout << "  Please enter a value between "
             << lo << " and " << hi << ".\n";
    }
}

static bool readYesNo(const string& prompt) {
    for (;;) {
        cout << prompt << " (y/n): ";

        string s;
        if (!(cin >> s)) {
            exit(0);
        }

        clearLine();

        const char c = static_cast<char>(
            tolower(static_cast<unsigned char>(s[0]))
        );

        if (c == 'y' || c == 'n') {
            return c == 'y';
        }

        cout << "  Please answer y or n.\n";
    }
}

static string readBitString() {
    for (;;) {
        cout << "Enter bit stream (0s and 1s only): ";

        string s;
        if (!(cin >> s)) {
            exit(0);
        }

        clearLine();

        if (isValidBitString(s)) {
            return s;
        }

        cout << "  Invalid. Use only 0 and 1.\n";
    }
}

// ----------------------------------------------------------------------------
// Table-driven menu helper
// ----------------------------------------------------------------------------
template <typename Entry>
static int chooseFrom(
    const string& title,
    const map<int, Entry>& table,
    const function<string(const Entry&)>& label
) {
    cout << "\n" << title << "\n";

    for (const auto& kv : table) {
        cout << "  " << kv.first << ". "
             << label(kv.second) << "\n";
    }

    return readInt(
        "Choice: ",
        table.begin()->first,
        table.rbegin()->first
    );
}

// ----------------------------------------------------------------------------
// Input sources
// ----------------------------------------------------------------------------
struct InputSource {
    const char* name;
    function<string()> run;
};

static string digitalRandom() {
    return generateRandomBits(
        readInt("Number of bits (8-64): ", 8, 64)
    );
}

static string digitalWithZeroRuns() {
    return generateBitsWithZeroRuns(
        readInt("Number of bits (12-64): ", 12, 64)
    );
}

static string digitalManual() {
    return readBitString();
}

static string analogPCM() {
    const double amp = readDouble(
        "Analog amplitude A in volts (0.5-20): ",
        0.5, 20.0
    );

    const double freq = readDouble(
        "Frequency f in Hz (1-1000): ",
        1.0, 1000.0
    );

    const int samples = readInt(
        "Number of samples over one period (4-16): ",
        4, 16
    );

    const int nbits = readInt(
        "Bits per sample (1-4): ",
        1, 4
    );

    PCMResult pcm = processPCMAnalogInput(
        amp, freq, samples, nbits
    );

    cout << "\n--- PCM: sampled voltages and quantized codewords ---\n";

    for (size_t i = 0; i < pcm.voltages.size(); ++i) {
        cout << "  Sample " << i + 1
             << ": " << pcm.voltages[i] << " V -> level "
             << pcm.levels[i] << " -> "
             << pcm.bits.substr(i * nbits, nbits) << "\n";
    }

    return pcm.bits;
}

static string digitalMenu() {
    static const map<int, InputSource> modes = {
        {1, {"Random bit stream", digitalRandom}},
        {2, {"Random stream with fixed 8-zero / 4-zero runs",
             digitalWithZeroRuns}},
        {3, {"Enter bit stream manually", digitalManual}}
    };

    const int choice = chooseFrom<InputSource>(
        "Digital data generation:",
        modes,
        [](const InputSource& s) {
            return string(s.name);
        }
    );

    return modes.at(choice).run();
}

// ----------------------------------------------------------------------------
// Encoders
// ----------------------------------------------------------------------------
struct EncoderEntry {
    const char* name;
    Encoder fn;
    bool isAMI;
};

struct ScramblerEntry {
    const char* name;
    Scrambler fn;
};

static const map<int, EncoderEntry> ENCODERS = {
    {1, {"NRZ-L", encodeNRZL, false}},
    {2, {"NRZ-I", encodeNRZI, false}},
    {3, {"Manchester", encodeManchester, false}},
    {4, {"Differential Manchester", encodeDiffManchester, false}},
    {5, {"AMI", encodeAMI, true}}
};

static const map<int, ScramblerEntry> SCRAMBLERS = {
    {1, {"B8ZS", encodeB8ZS}},
    {2, {"HDB3", encodeHDB3}}
};

// ----------------------------------------------------------------------------
// Main application
// ----------------------------------------------------------------------------
int main() {
    cout << "=====================================================\n"
         << "      ITT036 DIGITAL SIGNAL ENCODER / DECODER\n"
         << "=====================================================\n";

    // 1. Select input type.
    static const map<int, InputSource> inputTypes = {
        {1, {"Digital input", digitalMenu}},
        {2, {"Analog input (PCM)", analogPCM}}
    };

    const int type = chooseFrom<InputSource>(
        "Select input signal type:",
        inputTypes,
        [](const InputSource& s) {
            return string(s.name);
        }
    );

    const string data = inputTypes.at(type).run();

    // 2. Display the digital stream and longest palindrome.
    cout << "\n[Output 1] Digital data stream ("
         << data.size() << " bits): " << data << "\n";

    const string palindrome = findLongestPalindromeManacher(data);

    cout << "[Output 2] Longest palindromic substring: "
         << palindrome
         << "  (length " << palindrome.size()
         << ", Manacher O(n))\n";

    // 3. Select a line-coding scheme.
    const int enc = chooseFrom<EncoderEntry>(
        "Select line coding scheme:",
        ENCODERS,
        [](const EncoderEntry& e) {
            return string(e.name);
        }
    );

    const EncoderEntry& chosen = ENCODERS.at(enc);

    cout << "\n--- Results ---\n";
    cout << "Scheme: " << chosen.name << "\n";

    // AMI optionally uses B8ZS or HDB3 scrambling.
    const bool scramble =
        chosen.isAMI && readYesNo("Is scrambling needed?");

    // 4. Normal encoding and waveform generation.
    function<void()> plain = [&]() {
        const Levels signal = chosen.fn(data);

        cout << "[Output 3] Encoded signal levels: "
             << levelsToString(signal) << "\n";

        const bool saved = sigcodec::renderWaveform(
            data,
            signal,
            chosen.name,
            "waveform.png"
        );

        if (saved) {
            cout << "[Output 4] Waveform saved to waveform.png\n";
        } else {
            cout << "Waveform rendering failed.\n";
        }
    };

    // 5. Scrambling and waveform generation.
    function<void()> scrambled = [&]() {
        const int s = chooseFrom<ScramblerEntry>(
            "Select scrambling type:",
            SCRAMBLERS,
            [](const ScramblerEntry& e) {
                return string(e.name);
            }
        );

        const ScramblerEntry& scrambler = SCRAMBLERS.at(s);
        const ScrambleResult result = scrambler.fn(data);

        cout << "[Output 3] Plain AMI signal:        "
             << levelsToString(result.ami) << "\n";

        cout << "[Output 4] " << scrambler.name
             << " scrambled signal: "
             << levelsToString(result.scrambled) << "\n";

        string marks;
        for (int mark : result.substituted) {
            marks += ".^"[mark];
        }

        cout << "           substituted blocks:      "
             << marks << "\n";

        const bool saved = sigcodec::renderWaveform(
            data,
            result.scrambled,
            scrambler.name,
            "waveform.png"
        );

        if (saved) {
            cout << "[Output 5] Waveform saved to waveform.png\n";
        } else {
            cout << "Waveform rendering failed.\n";
        }
    };

    // Table-driven dispatch avoids a long if/else chain.
    const map<bool, function<void()>> actions = {
        {false, plain},
        {true, scrambled}
    };

    actions.at(scramble)();

    return 0;
}
