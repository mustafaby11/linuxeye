// SPDX-License-Identifier: GPL-3.0-or-later
// linuxeye-helper – privileged data collector
// Invoked via pkexec; writes JSON to stdout and exits.
// Schema:
// {
//   "board_serial":   "...",
//   "chassis_serial": "...",
//   "product_serial": "...",
//   "msr_tsc":        12345678   (future)
// }

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

static std::string read_file(const char *path)
{
    std::ifstream f(path);
    if (!f.is_open()) return {};
    std::ostringstream oss;
    oss << f.rdbuf();
    std::string s = oss.str();
    // trim trailing newline
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
        s.pop_back();
    return s;
}

static std::string json_escape(const std::string &s)
{
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else           out += c;
    }
    return out;
}

int main()
{
    const std::string board_serial   = read_file("/sys/class/dmi/id/board_serial");
    const std::string chassis_serial = read_file("/sys/class/dmi/id/chassis_serial");
    const std::string product_serial = read_file("/sys/class/dmi/id/product_serial");
    const std::string product_uuid   = read_file("/sys/class/dmi/id/product_uuid");

    std::printf("{\n"
                "  \"board_serial\":   \"%s\",\n"
                "  \"chassis_serial\": \"%s\",\n"
                "  \"product_serial\": \"%s\",\n"
                "  \"product_uuid\":   \"%s\"\n"
                "}\n",
                json_escape(board_serial).c_str(),
                json_escape(chassis_serial).c_str(),
                json_escape(product_serial).c_str(),
                json_escape(product_uuid).c_str());

    return 0;
}
