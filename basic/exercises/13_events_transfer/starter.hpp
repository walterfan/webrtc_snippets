/*
Requirements:
- Key subscriptions by DialogId + event package; own stored strings.
- subscribe registers a new key; apply_notify updates or rejects.
- matches_replaces: exact Call-ID / to-tag / from-tag equality.
- No sockets, sipfrag parsing, or transfer execution.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <optional>
#include <string>

namespace practice::day13 {

enum class SubscriptionState { pending, active, terminated };

struct DialogId {
  std::string call_id;
  std::string from_tag;
  std::string to_tag;
};

struct SubscriptionKey {
  DialogId dialog;
  std::string event;
};

class SubscriptionRegistry {
public:
  bool subscribe(const SubscriptionKey &key,
                 SubscriptionState initial);

  [[nodiscard]] std::optional<SubscriptionState>
  apply_notify(const SubscriptionKey &key,
               SubscriptionState next);
};

struct Replaces {
  std::string call_id;
  std::string to_tag;
  std::string from_tag;
};

[[nodiscard]] bool
matches_replaces(const DialogId &dialog,
                 const Replaces &target);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day13
