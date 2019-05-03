#pragma once

#include <string>
#include <vector>
#include <cstdint>


#ifndef NYX_BUILD_VERSION
#  define NYX_BUILD_VERSION(a,b,c) (((a) << 24) | ((b) << 16) | (c))
#endif

#define NYX_GEN_VERSION_INT    NYX_BUILD_VERSION(0, 0, 2)


namespace nyx {


const std::uint32_t GEN_NYX_VERSION = NYX_GEN_VERSION_INT;


std::vector<std::uint8_t> concat(const std::string &, const std::string &);
std::vector<std::uint8_t> concat(const std::string &, const std::vector<std::uint8_t> &);
std::vector<std::uint8_t> concat(const std::vector<std::uint8_t> &, const std::string &);
std::vector<std::uint8_t> concat(const std::vector<std::uint8_t> &, const std::vector<std::uint8_t> &);


template<typename LAMBDA>
void sequence(const std::vector<std::uint8_t> &vec, LAMBDA lambda) {
  for(std::size_t idx = 0, max = vec.size(); idx < max; ++idx) {
    lambda(vec[idx], idx, max);
  }
}

}

