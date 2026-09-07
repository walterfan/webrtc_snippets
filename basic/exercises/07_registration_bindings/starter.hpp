/*
Requirements:
- Maintain Contact bindings keyed by AOR, with URI as the per-AOR binding key.
- Positive expires create/replace; expires==0 removes one Contact; wildcard
  clears the AOR. Lookups return owned bindings in deterministic q/URI order.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace practice::day07 {

struct ContactBinding {
  std::string uri;
  unsigned expires{};
  int q_value{};
};

struct RegisterUpdate {
  std::string aor;
  std::vector<ContactBinding> contacts;
  bool wildcard_remove{};
};

class RegistrationStore {
public:
  bool apply(const RegisterUpdate &update);
  [[nodiscard]] std::vector<ContactBinding>
  lookup(std::string_view aor) const;
};

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day07
