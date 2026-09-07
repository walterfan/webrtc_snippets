/*
Requirements:
- Constrain observers to `on_state(ConnectionState) -> void`.
- Notify valid transitions, reject invalid transitions, and clean up local state
deterministically.
*/
#pragma once
#include "practice/starter_support.hpp"
#include <concepts>
#include <string_view>
namespace practice::day30 {
enum class ConnectionState { new_state, connecting, connected, failed, closed };
template <typename Observer>
concept StateObserver = requires(Observer observer, ConnectionState state) {
  { observer.on_state(state) } -> std::same_as<void>;
};
class PeerConnectionState {
public:
  template <StateObserver Observer> void set_observer(Observer observer);
  bool transition(ConnectionState next);
};
class Solution {
public:
  int run(std::ostream &out);
};
} // namespace practice::day30
