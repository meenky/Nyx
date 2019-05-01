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

    std::map<std::string, std::shared_ptr<nyx::syntax::AbstractElement>>::const_iterator
    resolve(const std::string &key) const {
      return global.find(key);
    }

    std::map<std::string, std::shared_ptr<nyx::syntax::AbstractElement>>::const_iterator
    resolve(const nyx::syntax::AbstractNamespaceElement &,
            const nyx::syntax::AbstractIdentifierElement &) const;

    std::map<std::string, std::shared_ptr<nyx::syntax::AbstractElement>>::const_iterator
    badResolve() const;

  protected:
    bool parse(const Filesystem &filesystem, const std::string &file, bool search);

    std::map<std::string, std::shared_ptr<nyx::syntax::AbstractSyntaxTree>>       parsed;
    std::map<std::string, std::shared_ptr<nyx::syntax::AbstractNamespaceElement>> emit;
    std::map<std::string, std::shared_ptr<nyx::syntax::AbstractNamespaceElement>> ref;
    std::map<std::string, std::shared_ptr<nyx::syntax::AbstractElement>>          global;
};


}

