#pragma once

#include <memory>
#include <set>
#include <string_view>

class Observer {
public:
  Observer(const Observer &) = default;
  Observer(Observer &&) = delete;
  auto operator=(const Observer &) -> Observer & = default;
  auto operator=(Observer &&) -> Observer & = delete;
  virtual ~Observer() = 0;
  virtual void update(std::string_view message) = 0;
};

class Chat_room {
public:
  void attach(std::shared_ptr<Observer> const &observer);
  void detach(std::shared_ptr<Observer> const &observer);
  void notify(std::string_view message);

private:
  std::set<std::weak_ptr<Observer>, std::owner_less<>> _receivers;
};
