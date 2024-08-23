#pragma once

#include <memory>
#include <string_view>
#include <unordered_set>

class Message_receiver {
public:
  virtual ~Message_receiver() = 0;

  virtual void receive(std::string_view message) = 0;
};

class Chat_room {
public:
  void register_player(std::shared_ptr<Message_receiver> const &receiver)
  {
    _receivers.insert(receiver);
  }

  void deregister_player(std::shared_ptr<Message_receiver> const &receiver)
  {
    _receivers.extract(receiver);
  }

  void deliver_message(std::string_view const &message)
  {
    std::erase_if(_receivers,
                  [](auto const &player) { return player.expired(); });

    for (auto const &receiver : _receivers) {
      receiver.lock()->receive(message);
    }
  }

private:
  std::unordered_set<std::weak_ptr<Message_receiver>> _receivers;
};
