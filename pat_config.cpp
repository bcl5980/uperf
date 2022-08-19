#include <fstream>
#include <stdio.h>

#include "uperf.h"

using std::ifstream;
using std::string;
using std::vector;

inline bool startswith(const string &toCheck, const string &prefix) {
    return std::equal(prefix.begin(), prefix.end(), toCheck.begin());
}

const string WHITESPACE = " \n\r\t\f\v";
const string PrefixArch = "Arch:";
const string PrefixWorkMode = "WorkMode:";
const string PrefixArgs = "Args:";
const string PrefixDelay = "Delay:";
const string PrefixPrologue = "Prologue:";
const string PrefixContent = "Content:";
const string PrefixEpilogue = "Epilogue:";
const string PrefixPeriod = "Period:";
const string PrefixPeriodFill = "PeriodFill:";

const static unsigned long MaxMemorySize = 0x10000000;

string trim(const string &s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }

    string::const_iterator end;
    size_t comment = s.find("//");
    if (comment == 0)
        return "";

    if (comment != string::npos)
        end = start + comment - 1;
    else
        end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return string(start, end + 1);
}

static bool parseArch(const string &line, PatConfig &config) {
    size_t prefixSize = PrefixArch.size();
    if (line.size() < prefixSize)
        return false;

    string arch = trim(line.substr(prefixSize));
    if (arch == "X86")
        config.arch = ArchType::X86_64;
    else if (arch == "AArch64")
        config.arch = ArchType::AArch64;
    else {
        printf("Only support X86 or AArch64.\n");
        return false;
    }

    return true;
}

static bool parseWorkMode(const string &line, PatConfig &config) {
    size_t prefixSize = PrefixWorkMode.size();
    if (line.size() < prefixSize)
        return false;

    string arch = trim(line.substr(prefixSize));
    if (arch == "Delay")
        config.mode = WorkMode::DelayTest;
    else if (arch == "Period")
        config.mode = WorkMode::PeriodTest;
    else {
        printf("Only support Delay or Period work mode.\n");
        return false;
    }

    return true;
}

static bool parseArgs(ifstream &is, const string &line, PatConfig &config) {
    if (line.find("null") != string::npos) {
        config.args.iArg0 = 0;
        config.args.iArg1 = 8;
        config.args.ptrArg0 = nullptr;
        config.args.ptrArg1 = nullptr;
        return true;
    }

    string newLine;
    std::streampos RollBackPos = is.tellg();
    unsigned argCount = 0;
    while (argCount < 4 && std::getline(is, newLine)) {
        string arg = trim(newLine);
        if (arg.find(':') != string::npos)
            break;
        else if (startswith(arg, "I0,")) {
            string strNum = trim(arg.substr(3));
            config.args.iArg0 = std::stoull(strNum);
            argCount++;
        } else if (startswith(arg, "I1,")) {
            string strNum = trim(arg.substr(3));
            config.args.iArg1 = std::stoull(strNum);
            argCount++;
        } else if (startswith(arg, "Ptr0,")) {
            string strNum = trim(arg.substr(5));
            unsigned long memSize = std::stoul(strNum, nullptr, 16);
            if (memSize == 0) {
                config.args.ptrArg0 = nullptr;
            } else {
                if (memSize > MaxMemorySize)
                    memSize = MaxMemorySize;
                config.args.ptrArg0 = malloc(memSize);
            }
            argCount++;
        } else if (startswith(arg, "Ptr1,")) {
            string strNum = trim(arg.substr(5));
            unsigned long memSize = std::stoul(strNum, nullptr, 16);
            if (memSize == 0) {
                config.args.ptrArg1 = nullptr;
            } else {
                if (memSize > MaxMemorySize)
                    memSize = MaxMemorySize;
                config.args.ptrArg1 = malloc(memSize);
            }
            argCount++;
        }
        RollBackPos = is.tellg();
    }

    is.seekg(RollBackPos);
    return argCount == 4;
}

static bool parseInsts(ifstream &is, const string &line, vector<InstBytes> &insts) {
    insts.clear();
    if (line.find("null") != string::npos) {
        return true;
    }

    string newLine;
    std::streampos RollBackPos = is.tellg();
    while (std::getline(is, newLine)) {
        if (newLine.find(':') != string::npos)
            break;

        InstBytes inst;
        string byteCode = trim(newLine);
        size_t findS = 0, findE = 0;
        while ((findE = byteCode.find(',', findS)) != string::npos) {
            inst.push_back((unsigned char)std::stoi(byteCode.substr(findS, findE), nullptr, 16));
            findS = findE + 1;
        }
        if (findS < byteCode.size() - 1)
            inst.push_back((unsigned char)std::stoi(byteCode.substr(findS), nullptr, 16));

        RollBackPos = is.tellg();
        insts.push_back(inst);
    }

    is.seekg(RollBackPos);
    return true;
}

static bool parseDelayPattern(ifstream &is, const string &line, PatConfig &config) {
    return parseInsts(is, line, config.di.delayPat);
}

static bool parsePrologue(ifstream &is, const string &line, PatConfig &config) {
    return parseInsts(is, line, config.di.prologuePat);
}

static bool parseContent(ifstream &is, const string &line, PatConfig &config) {
    return parseInsts(is, line, config.di.contentPat);
}

static bool parseEpilogue(ifstream &is, const string &line, PatConfig &config) {
    return parseInsts(is, line, config.di.epiloguePat);
}

static bool parsePeriod(ifstream &is, const string &line, PatConfig &config) {
    return parseInsts(is, line, config.pi.periodPat);
}

static bool parsePeriodFill(ifstream &is, const string &line, PatConfig &config) {
    return parseInsts(is, line, config.pi.fillPat);
}

bool parseConfig(PatConfig &config, const string &path) {
    ifstream is(path, std::ios::in);
    if (!is.is_open()) {
        printf("config file open failed\n");
        return false;
    }

    string line;
    while (std::getline(is, line)) {
        line = trim(line);
        if (startswith(line, PrefixArch)) {
            if (!parseArch(line, config)) {
                printf("parse arch failed\n");
                return false;
            }
        } else if (startswith(line, PrefixWorkMode)) {
            if (!parseWorkMode(line, config)) {
                printf("parse work mode failed\n");
                return false;
            }
        } else if (startswith(line, PrefixArgs)) {
            if (!parseArgs(is, line, config)) {
                printf("parse arguments failed\n");
                return false;
            }
        } else if (startswith(line, PrefixDelay)) {
            if (!parseDelayPattern(is, line, config)) {
                printf("parse delay pattern failed\n");
                return false;
            }
        } else if (startswith(line, PrefixPrologue)) {
            if (!parsePrologue(is, line, config)) {
                printf("parse prologue failed\n");
                return false;
            }
        } else if (startswith(line, PrefixContent)) {
            if (!parseContent(is, line, config)) {
                printf("parse content failed\n");
                return false;
            }
        } else if (startswith(line, PrefixEpilogue)) {
            if (!parseEpilogue(is, line, config)) {
                printf("parse epilogue failed\n");
                return false;
            }
        } else if (startswith(line, PrefixPeriod)) {
            if (!parsePeriod(is, line, config)) {
                printf("parse period pattern failed\n");
                return false;
            }
        } else if (startswith(line, PrefixPeriodFill)) {
            if (!parsePeriodFill(is, line, config)) {
                printf("parse period fill pattern failed\n");
                return false;
            }
        }
    }

    if (config.mode == WorkMode::DelayTest) {
        if (config.di.contentPat.empty()) {
            printf("Delay Test must have content pattern");
            return false;
        }
    } else if (config.mode == WorkMode::PeriodTest) {
        if (config.pi.periodPat.empty()) {
            printf("Period Test must have period pattern");
            return false;
        }

        if (config.pi.fillPat.empty()) {
            printf("Period Test must have fill pattern");
            return false;
        }
    }

    return true;
}