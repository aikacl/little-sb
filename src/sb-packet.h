#pragma once

#include "user.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

using nlohmann::json;

using Sb_packet_sender = User_info;

// This class should only contains POD (plain old data).
struct Sb_packet {
  static constexpr std::string_view this_protocol_name{"sbp"};

  Sb_packet() = default;

  Sb_packet(Sb_packet_sender sender, std::string payload = "")
      : sender{std::move(sender)}, payload{std::move(payload)}
  {
  }

  // Header
  std::string_view protocol{this_protocol_name};
  Sb_packet_sender sender{"Undefined username", "Undefined password"};
  std::string payload{"Undefined body"};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Sb_packet, protocol, sender, payload)
};

template <> struct fmt::formatter<Sb_packet> : fmt::formatter<std::string> {
  static auto format(Sb_packet const &packet,
                     format_context &ctx) -> decltype(ctx.out())
  {
    return fmt::format_to(ctx.out(), "protocol={},sender={},payload={}.",
                          packet.protocol, packet.sender.username(),
                          packet.payload);
  }
};
