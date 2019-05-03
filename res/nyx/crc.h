#pragma once

#include <vector>
#include <cstdint>


namespace nyx {
  namespace crc {


    std::uint8_t
    crc8(std::uint8_t                     poly, std::uint8_t seed,
         const std::vector<std::uint8_t> &data, std::uint8_t mask);

    std::uint16_t
    crc16(std::uint16_t                     poly, std::uint16_t seed,
          const std::vector<std::uint16_t> &data, std::uint16_t mask);

    std::uint32_t
    crc32(std::uint32_t                     poly, std::uint32_t seed,
          const std::vector<std::uint32_t> &data, std::uint32_t mask);

    std::uint64_t
    crc64(std::uint64_t                     poly, std::uint8_t seed,
          const std::vector<std::uint64_t> &data, std::uint8_t mask);


  }
}

