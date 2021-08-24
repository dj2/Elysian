#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace el {

enum class EventType { kResized };

struct Event {};

struct ResizeEvent : public Event {};

using EventCallback = std::function<void(const Event*)>;

class EventService {
 public:
  auto add(EventType event, const EventCallback& cb) -> void {
    auto it = listeners_.find(event);
    if (it == listeners_.end()) {
      std::vector<EventCallback> vec = {cb};
      listeners_.insert(std::make_pair(event, vec));
      return;
    }

    it->second.push_back(cb);
  }

  auto emit(EventType event, const Event* data) -> void {
    auto it = listeners_.find(event);
    if (it == listeners_.end()) {
      return;
    }

    auto& vec = it->second;
    std::for_each(std::begin(vec), std::end(vec),
                  [data](const EventCallback& cb) { cb(data); });
  }

 private:
  std::unordered_map<EventType, std::vector<EventCallback>> listeners_;
};

}  // namespace el
