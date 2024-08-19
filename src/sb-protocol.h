#pragma once

#include <algorithm>
#include <array>
#include <asio.hpp>
#include <print>
#include <stdexcept>
#include <string_view>

struct Sb_packet {
  static constexpr auto field_length_limit{255};
  template <std::size_t Max_length>
    requires requires { Max_length <= field_length_limit; }
  struct Field {
    Field() = default;

    Field(std::string_view const data)
        : length{static_cast<std::uint8_t>(data.size())}
    {
      if (data.size() >= Max_length) {
        throw std::runtime_error{"Buffer overflow"};
      }
      std::ranges::copy(data, this->data.begin());
    }

    [[nodiscard]] auto to_string() const -> std::string
    {
      return std::string{data.data(), length};
    }

    std::uint8_t length{};
    std::array<char, Max_length> data{};
  };

  enum class Sender_type : std::uint8_t {
    Client,
    Administrator,
    Server,
  };

  static constexpr auto protocol_length{4};
  static constexpr auto max_body_length{256};
  static constexpr auto max_sender_name_length{32};
  static constexpr std::array<char, protocol_length> this_protocol_name{
      's', 'b', 'p', '\0'};

  struct Header {
    std::array<char, protocol_length> protocol_name{this_protocol_name};
    Sender_type sender_type{};
    Field<max_sender_name_length> sender_name;
  };

  Sb_packet() = default;

  Sb_packet(Sender_type const sender_type, std::string_view const sender_name,
            std::string_view const data)
      : header{
            .sender_type = sender_type,
            .sender_name{sender_name},
        }, body{data}
  {
  }

  operator asio::mutable_buffer()
  {
    return asio::mutable_buffer{static_cast<void *>(this), sizeof(Sb_packet)};
  }

  [[nodiscard]] auto to_string() const -> std::string
  {
    return body.to_string();
  }

  Header header;
  Field<max_body_length> body;
};
