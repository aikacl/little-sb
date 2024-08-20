#pragma once

#include <algorithm>
#include <array>
#include <asio.hpp>
#include <print>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string_view>

static constexpr auto field_length_limit{1024};
template <std::size_t Max_length>
  requires requires { Max_length <= field_length_limit; }
class Field {
public:
  Field() : Field{""} {}

  Field(std::string_view const data) : _length{data.size()}
  {
    if (_length >= Max_length) {
      throw std::runtime_error{"Buffer overflow"};
    }

    std::ranges::copy(data, _data.begin());
  }

  Field(char const *const data) : _length{std::strlen(data)}
  {
    if (_length >= Max_length) {
      throw std::runtime_error{"Buffer overflow"};
    }

    std::ranges::copy(std::string_view{data, _length}, _data.begin());
  }

  Field(std::string const &data) : _length{data.size()}
  {
    if (_length >= Max_length) {
      throw std::runtime_error{"Buffer overflow"};
    }

    std::ranges::copy(data, _data.begin());
  }

  [[nodiscard]] auto to_string() const -> std::string
  {
    return std::string{_data.data(), _length};
  }

  [[nodiscard]] auto data() const -> char const *
  {
    return data();
  }

  auto data() -> char *
  {
    return _data.data();
  }

private:
  std::uint64_t _length;
  std::array<char, Max_length> _data;
};

static constexpr auto max_sender_name_length{64};
struct Sb_packet_sender {
  enum class Type : std::uint8_t {
    Client,
    Administrator,
    Server,
    Undefined,
  };

  Type type;
  Field<max_sender_name_length> name;
};

enum class Sb_packet_type : uint8_t {
  Login,
  Message,
  Undefined,
};

struct Sb_packet {
  static constexpr auto protocol_length{4};
  static constexpr auto max_body_length{256};
  static constexpr std::array<char, protocol_length> this_protocol_name{
      's', 'b', 'p', '\0'};

  struct Header {
    std::array<char, protocol_length> protocol_name{this_protocol_name};
    Sb_packet_sender sender;
  };

  explicit Sb_packet()
      : header{.sender{.type = Sb_packet_sender::Type::Undefined,
                       .name{"Undefined sender name"}}},
        type{Sb_packet_type::Undefined}, body{"Undefine body"}
  {
  }

  Sb_packet(Sb_packet_sender const sender,
            Sb_packet_type const type = Sb_packet_type::Message,
            std::string_view const data = "")
      : header{.sender{sender}}, type{type}, body{data}
  {
  }

  operator asio::mutable_buffer()
  {
    return asio::mutable_buffer{static_cast<void *>(this), sizeof(Sb_packet)};
  }

  operator asio::const_buffer() const
  {
    return asio::const_buffer{static_cast<void const *>(this),
                              sizeof(Sb_packet)};
  }

  [[nodiscard]] auto to_string() const -> std::string
  {
    return body.to_string();
  }

  Header header;
  Sb_packet_type type;
  Field<max_body_length> body;
};

template <> struct fmt::formatter<Sb_packet> : fmt::formatter<std::string> {
  static constexpr auto format(Sb_packet const &packet,
                               format_context &ctx) -> decltype(ctx.out())
  {
    return fmt::format_to(ctx.out(), "[protocol: {}, sender({}): {}, body: {}]",
                          std::string_view{packet.header.protocol_name},
                          static_cast<int>(packet.header.sender.type),
                          packet.header.sender.name.to_string(),
                          packet.body.to_string());
  }
};
