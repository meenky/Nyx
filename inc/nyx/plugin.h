#pragma once

#include <nyx/plan.h>


namespace nyx {


class Plugin {
  public:
    ~Plugin();

    static std::shared_ptr<Plugin> load(const Filesystem &fs,
                                        const std::string &lang,
                                        const std::vector<std::string> &opts);

    int execute(std::shared_ptr<Plan> plan);

  private:
    Plugin(void *, const std::string &);

    std::string lang;
    void *L;
};


}

