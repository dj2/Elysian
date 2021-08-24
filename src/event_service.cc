module;

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

export module event_service;

namespace el {

export enum class EventType { kResized };

export struct Event {};

export struct ResizeEvent : public Event {};

export using EventCallback = std::function<void(const Event*)>;

export class EventService {
 public:
  auto add(EventType event, EventCallback cb) -> void {
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

    std::ranges::for_each(it->second, [data](EventCallback cb) { cb(data); });
  }

 private:
  std::unordered_map<EventType, std::vector<EventCallback>> listeners_;
};

}  // namespace el
