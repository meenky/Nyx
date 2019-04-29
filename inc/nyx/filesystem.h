#pragma once


#include <memory>
#include <string>
#include <vector>
#include <istream>


namespace nyx {


class Filesystem {
  public:
    Filesystem(const std::string &sys, const std::vector<std::string> &user);

    std::shared_ptr<std::string> locate(const std::string &filename) const;

  protected:
    std::vector<std::string> paths;
};


}

