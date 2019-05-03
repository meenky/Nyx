#include "nyx/runtime.h"


std::vector<std::uint8_t> concat(const std::string &one, const std::string &two) {
  std::vector<std::uint8_t> retVal(one.begin(), one.end());
  retVal.insert(retVal.end(), two.begin(), two.end());
  return retVal;
}


std::vector<std::uint8_t> concat(const std::string &one, const std::vector<std::uint8_t> &two) {
  std::vector<std::uint8_t> retVal(one.begin(), one.end());
  retVal.insert(retVal.end(), two.begin(), two.end());
  return retVal;
}


std::vector<std::uint8_t> concat(const std::vector<std::uint8_t> &one, const std::string &two) {
  std::vector<std::uint8_t> retVal(one.begin(), one.end());
  retVal.insert(retVal.end(), two.begin(), two.end());
  return retVal;
}


std::vector<std::uint8_t> concat(const std::vector<std::uint8_t> &one,
                                 const std::vector<std::uint8_t> &two) {
  std::vector<std::uint8_t> retVal(one.begin(), one.end());
  retVal.insert(retVal.end(), two.begin(), two.end());
  return retVal;
}

