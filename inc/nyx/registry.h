#pragma once

#include <nyx/filesystem.h>
#include <nyx/syntax/abstract.h>

#include <map>
#include <memory>
#include <string>


namespace nyx {


class Registry {
  public:
    Registry();

    bool parse(const Filesystem &filesystem, const std::string &file);

    auto &abstractSyntaxTrees() {
      return parsed;
    }

    auto &abstractSyntaxTrees() const {
      return parsed;
    }

    auto &namespaces() {
      return emit;
    }

    auto &namespaces() const {
      return emit;
    }

    auto &modules() {
      return ref;
    }

    auto &modules() const {
      return ref;
    }

    auto &fullyQualified() {
      return global;
    }

    auto &fullyQualified() const {
      return global;
    }

  protected:
    bool parse(const Filesystem &filesystem, const std::string &file, bool search);

    std::map<const std::string, std::shared_ptr<nyx::syntax::AbstractSyntaxTree>>       parsed;
    std::map<const std::string, std::shared_ptr<nyx::syntax::AbstractNamespaceElement>> emit;
    std::map<const std::string, std::shared_ptr<nyx::syntax::AbstractNamespaceElement>> ref;
    std::map<const std::string, std::shared_ptr<nyx::syntax::AbstractElement>>          global;
};


}

