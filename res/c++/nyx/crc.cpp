#include "nyx/crc.h"

std::uint8_t
nyx::crc::crc8(std::uint8_t                     poly, std::uint8_t seed,
               const std::vector<std::uint8_t> &data, std::uint8_t mask) {
  auto crc = seed;

  for(auto byte : data) {
    crc ^= byte;

    for(int bit = 0; bit < 8; ++bit) {
      if(crc & 0x80U) {
        crc = (crc << 1) ^ poly;
      }
      else {
        crc <<= 1;
      }
    }
  }

  return crc ^ mask;
}


std::uint16_t
nyx::crc::crc16(std::uint16_t                     poly, std::uint16_t seed,
                const std::vector<std::uint16_t> &data, std::uint16_t mask) {
  auto crc = seed;

  for(auto byte : data) {
    crc ^= static_cast<std::uint16_t>(byte) << 8;

    for(int bit = 0; bit < 8; ++bit) {
      if(crc & 0x8000U) {
        crc = (crc << 1) ^ poly;
      }
      else {
        crc <<= 1;
      }
    }
  }

  return crc ^ mask;
}


std::uint32_t
nyx::crc::crc32(std::uint32_t                     poly, std::uint32_t seed,
                const std::vector<std::uint32_t> &data, std::uint32_t mask) {
  auto crc = seed;

  for(auto byte : data) {
    crc ^= static_cast<std::uint32_t>(byte) << 24;

    for(int bit = 0; bit < 8; ++bit) {
      if(crc & 0x80000000U) {
        crc = (crc << 1) ^ poly;
      }
      else {
        crc <<= 1;
      }
    }
  }

  return crc ^ mask;
}


std::uint64_t
nyx::crc::crc64(std::uint64_t                     poly, std::uint8_t seed,
                const std::vector<std::uint64_t> &data, std::uint8_t mask) {
  auto crc = seed;

  for(auto byte : data) {
    crc ^= static_cast<std::uint64_t>(byte) << 56;

    for(int bit = 0; bit < 8; ++bit) {
      if(crc & 0x8000000000000000UL) {
        crc = (crc << 1) ^ poly;
      }
      else {
        crc <<= 1;
      }
    }
  }

  return crc ^ mask;
}

