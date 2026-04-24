#include "protocol/parser.hpp"
#include <algorithm>
#include <cstring>
#include <sstream>

namespace gomoku {

static std::string rstrip_cr(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
    return s;
}

static bool starts_with(const std::string& s, const char* pfx) {
    size_t n = std::strlen(pfx);
    return s.size() >= n && std::equal(pfx, pfx + n, s.begin());
}

static bool parse_coords(const std::string& s, int& x, int& y) {
    size_t comma = s.find(',');
    if (comma == std::string::npos) return false;
    try {
        x = std::stoi(s.substr(0, comma));
        y = std::stoi(s.substr(comma + 1));
    } catch (...) { return false; }
    return true;
}

Parser::Command Parser::parse(std::string line) {
    line = rstrip_cr(std::move(line));
    Command c;
    c.raw = line;
    if (line.empty()) { c.kind = UNKNOWN; return c; }

    if (starts_with(line, "START")) {
        std::istringstream ss(line);
        std::string tok; ss >> tok;
        if (!(ss >> c.size)) { c.kind = MALFORMED; return c; }
        c.kind = START; return c;
    }
    if (starts_with(line, "TURN")) {
        std::string body = line.substr(4);
        while (!body.empty() && body.front() == ' ') body.erase(body.begin());
        if (!parse_coords(body, c.x, c.y)) { c.kind = MALFORMED; return c; }
        c.kind = TURN; return c;
    }
    if (line == "BEGIN")   { c.kind = BEGIN; return c; }
    if (line == "BOARD")   { c.kind = BOARD_START; return c; }
    if (line == "DONE")    { c.kind = DONE; return c; }
    if (line == "END")     { c.kind = END; return c; }
    if (line == "ABOUT")   { c.kind = ABOUT; return c; }
    if (line == "RESTART") { c.kind = RESTART; return c; }
    if (starts_with(line, "INFO")) {
        std::istringstream ss(line);
        std::string tok; ss >> tok;
        if (!(ss >> c.info_key)) { c.kind = MALFORMED; return c; }
        std::string rest;
        std::getline(ss, rest);
        if (!rest.empty() && rest.front() == ' ') rest.erase(rest.begin());
        c.info_value = rest;
        c.kind = INFO; return c;
    }
    {
        int commas = static_cast<int>(std::count(line.begin(), line.end(), ','));
        if (commas == 2) {
            size_t c1 = line.find(',');
            size_t c2 = line.find(',', c1 + 1);
            try {
                c.x   = std::stoi(line.substr(0, c1));
                c.y   = std::stoi(line.substr(c1 + 1, c2 - c1 - 1));
                c.who = std::stoi(line.substr(c2 + 1));
                c.kind = BOARD_CELL;
                return c;
            } catch (...) { }
        }
    }
    c.kind = UNKNOWN;
    return c;
}

} // namespace gomoku
