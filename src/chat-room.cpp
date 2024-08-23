#include "chat-room.h"

void Chat_room::attach(std::shared_ptr<Observer> const &observer)
{
  _receivers.insert(observer);
}

void Chat_room::detach(std::shared_ptr<Observer> const &observer)
{
  _receivers.extract(observer);
}

void Chat_room::notify(std::string_view message)
{
  std::erase_if(_receivers,
                [](auto const &player) { return player.expired(); });

  for (auto const &receiver : _receivers) {
    if (auto const locked{receiver.lock()}) {
      locked->update(message);
    }
  }
}
