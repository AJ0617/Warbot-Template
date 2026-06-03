#pragma once
// ── Lightweight pub/sub snapshot protocol (WPILib-style table feed) ───────────
// One snapshot = a group of newline-delimited lines terminated by a line "~":
//   P <x> <y> <angle>        robot pose (inches / degrees)
//   M <modeName>             resolved display mode (READY / AUTONOMOUS / ...)
//   S <selectedAutonIdx>
//   A <count>                followed by <count> auton-name lines (may have spaces)
//   <name 1> ...
//   T <count> <x1> <y1> ...  trail points, all on one line
//   ~
// State flows host → viewer only. No JSON dependency — fixed schema parsed by hand.

#include "renderer.hpp"   // RenderState (shared plain struct, no pros deps)
#include <string>
#include <sstream>
#include <cstdio>

namespace simnet {

inline std::string serialize(const RenderState& s) {
    std::ostringstream os;
    os << "P " << s.robotX << ' ' << s.robotY << ' ' << s.robotAngle << '\n';
    os << "M " << s.modeName << '\n';
    os << "S " << s.autonSelectedIdx << '\n';
    os << "A " << s.autonList.size() << '\n';
    for (const auto& n : s.autonList) os << n << '\n';
    os << "T " << s.trail.size();
    for (const auto& p : s.trail) os << ' ' << p.first << ' ' << p.second;
    os << "\n~\n";
    return os.str();
}

// Parse one complete frame (text up to, not including, the terminating '~').
inline bool parse(const std::string& frame, RenderState& out) {
    std::istringstream is(frame);
    std::string line;
    RenderState s;
    while (std::getline(is, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        switch (line[0]) {
            case 'P':
                std::sscanf(line.c_str(), "P %lf %lf %lf",
                            &s.robotX, &s.robotY, &s.robotAngle);
                break;
            case 'M':
                s.modeName = line.size() > 2 ? line.substr(2) : "";
                break;
            case 'S':
                std::sscanf(line.c_str(), "S %d", &s.autonSelectedIdx);
                break;
            case 'A': {
                int n = 0;
                std::sscanf(line.c_str(), "A %d", &n);
                for (int i = 0; i < n && std::getline(is, line); ++i) {
                    if (!line.empty() && line.back() == '\r') line.pop_back();
                    s.autonList.push_back(line);
                }
                break;
            }
            case 'T': {
                std::istringstream ts(line.substr(1));
                int n = 0;
                ts >> n;
                for (int i = 0; i < n; ++i) {
                    double x = 0, y = 0;
                    ts >> x >> y;
                    s.trail.emplace_back(x, y);
                }
                break;
            }
            default:
                break;
        }
    }
    out = std::move(s);
    return true;
}

} // namespace simnet
