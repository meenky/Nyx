#pragma once

#include <nyx/registry.h>


namespace nyx {


class Plan {
  public:
    Plan();

    static std::shared_ptr<Plan> generate(Registry &);
};


}
