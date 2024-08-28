#pragma once

#include "json.h"
#include "random.h"
#include <spdlog/spdlog.h>

namespace uuid {
template <typename Byte> struct Uuid {
  static constexpr std::size_t num_bytes{16};

  Uuid()
  {
    for (auto &byte : data_bytes) {
      byte = little_sb::random::uniform(std::numeric_limits<Byte>::min(),
                                        std::numeric_limits<Byte>::max());
    }
  }

  auto operator<=>(Uuid const &rhs) const -> std::strong_ordering
  {
    return data_bytes <=> rhs.data_bytes;
  }

  std::array<Byte, num_bytes> data_bytes{};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Uuid, data_bytes)
};
} // namespace uuid

using Uuid = uuid::Uuid<std::uint8_t>;

template <> struct fmt::formatter<Uuid> : fmt::formatter<std::string> {
  static auto format(Uuid const &uuid,
                     format_context &ctx) -> decltype(ctx.out())
  {
    return fmt::format_to(
        ctx.out(),
        "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}"
        "{:02x}{:02x}{:02x}{:02x}{:02x}",
        uuid.data_bytes[0], uuid.data_bytes[1], uuid.data_bytes[2],
        uuid.data_bytes[3], uuid.data_bytes[4], uuid.data_bytes[5],
        uuid.data_bytes[6], uuid.data_bytes[7], uuid.data_bytes[8],
        uuid.data_bytes[9], uuid.data_bytes[10], uuid.data_bytes[11],
        uuid.data_bytes[12], uuid.data_bytes[13], uuid.data_bytes[14],
        uuid.data_bytes[15]);
  }
};
