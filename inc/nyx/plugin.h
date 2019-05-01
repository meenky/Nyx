#pragma once

#include <nyx/plan.h>


namespace nyx {


class Plugin {
  public:
    ~Plugin();

    static std::unique_ptr<Plugin> load(const Filesystem &fs,
                                        const std::string &lang,
                                        const std::vector<std::string> &opts);

    int execute(Plan &plan);

  private:
    Plugin(void *, const std::string &, const std::vector<std::string> &);

    std::vector<std::string>  options;
    std::string               lang;
    void                     *L;
};


}

