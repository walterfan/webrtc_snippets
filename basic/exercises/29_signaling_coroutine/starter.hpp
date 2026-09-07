/*
Requirements:
- Keep coroutine-handle ownership explicit and local.
- Emit offer before answer and propagate injected local failure without an
external event loop.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <coroutine>
#include <exception>
#include <string>
namespace practice::day29 {
struct SignalTask {
  struct promise_type {
    SignalTask get_return_object();
    std::suspend_never initial_suspend() noexcept;
    std::suspend_never final_suspend() noexcept;
    void return_void() noexcept;
    void unhandled_exception();
  };
};
[[nodiscard]] SignalTask negotiate_offer_answer(std::string offer);
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day29
