#pragma once

#include "json.h"
#include "user.h"
#include <spdlog/spdlog.h>
#include <string_view>

// This class should only contains POD (plain old data).
struct Packet {
  using Sender = User_info;
  static constexpr std::string_view this_protocol_name{"sbp"};

  Packet() = default;

  Packet(Sender sender, std::string payload)
      : sender{std::move(sender)}, payload{std::move(payload)}
  {
  }

  std::string protocol{this_protocol_name};
  Sender sender{"Undefined username", "Undefined password"};
  std::string payload{"Undefined body"};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Packet, protocol, sender, payload)
};

template <> struct fmt::formatter<Packet> : fmt::formatter<std::string> {
  static auto format(Packet const &packet, format_context &ctx)
      -> decltype(ctx.out())
  {
    return fmt::format_to(ctx.out(), "protocol={},sender={},payload={}.",
                          packet.protocol, packet.sender.username(),
                          packet.payload);
  }
};
