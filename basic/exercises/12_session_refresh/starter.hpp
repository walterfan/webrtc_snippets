/*
Requirements:
- Negotiate Session-Expires against Min-SE; omit refresher → UAC.
- too_small (422-shaped) vs invalid_interval for non-positive offers.
- refresh_due at half interval; UPDATE early OK; re-INVITE confirmed only.
- Classify 491 as glare retry. No sleep or wall-clock reads.
*/
#pragma once

#include "practice/starter_support.hpp"

#include <chrono>
#include <optional>
#include <variant>

namespace practice::day12 {

enum class Refresher { uac, uas };
enum class RefreshMethod { update, reinvite };

struct SessionTimerOffer {
  std::chrono::seconds interval;
  std::optional<Refresher> refresher;
};

struct SessionTimer {
  std::chrono::seconds interval;
  Refresher refresher;
};

enum class SessionTimerError { too_small, invalid_interval };

struct SessionTimerFailure {
  SessionTimerError kind;
  std::chrono::seconds required_min;
};

using SessionTimerResult =
    std::variant<SessionTimer, SessionTimerFailure>;

[[nodiscard]] SessionTimerResult
negotiate_session_timer(const SessionTimerOffer &offer,
                        std::chrono::seconds min_se);

[[nodiscard]] bool
refresh_due(const SessionTimer &timer,
            std::chrono::seconds elapsed);

[[nodiscard]] bool
refresh_method_allowed(RefreshMethod method, bool early_dialog);

[[nodiscard]] bool
is_glare_retry(unsigned short response_code);

class Solution {
public:
  int run(std::ostream &out);
};

} // namespace practice::day12
