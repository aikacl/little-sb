#pragma once

#include "user.h"
#include <spdlog/spdlog.h>

using nlohmann::json;
using Packet_sender = User_info;

// This class should only contains POD (plain old data).
struct Packet {
  static constexpr std::string_view this_protocol_name{"sbp"};

  Packet() = default;

  Packet(Packet_sender sender, std::string payload = "")
      : sender{std::move(sender)}, payload{std::move(payload)}
  {
  }

  // Header
  std::string protocol{this_protocol_name};
  Packet_sender sender{"Undefined username", "Undefined password"};
  std::string payload{"Undefined body"};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Packet, protocol, sender, payload)
};

template <> struct fmt::formatter<Packet> : fmt::formatter<std::string> {
  static auto format(Packet const &packet,
                     format_context &ctx) -> decltype(ctx.out())
  {
    return fmt::format_to(ctx.out(), "protocol={},sender={},payload={}.",
                          packet.protocol, packet.sender.username(),
                          packet.payload);
  }
};
