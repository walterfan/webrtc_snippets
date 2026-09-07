#pragma once

#include <iosfwd>
#include <string_view>
#include <vector>

namespace practice {

int run_cli(const std::vector<std::string_view> &args, std::istream &in,
            std::ostream &out, std::ostream &err);
int run_tui(std::istream &in, std::ostream &out, std::ostream &err);

} // namespace practice
