#include <nyx/registry.h>
#include <nyx/syntax/parser.h>


using namespace nyx;
using namespace nyx::syntax;


Registry::Registry():
  parsed(),
  emit(),
  ref(),
  global() {
}


static void normalize(
    const std::map<std::string, std::shared_ptr<nyx::syntax::AbstractNamespaceElement>> &src,
          std::map<std::string, std::shared_ptr<nyx::syntax::AbstractElement>>          &dst) {
  for(auto &ns : src) {
    auto root = ns.second->identifier();
    // add the namespace itself
    dst.emplace(root->toString(), ns.second);

    // add all the rules
    for(auto &rule : *ns.second) {
      AbstractIdentifierElement ident(*root, *rule.second->identifier());
      dst.emplace(ident.toString(), rule.second);

      auto storageList = rule.second->storage();

      if(storageList) {
        for(auto &storage : *storageList) {
          dst.emplace(AbstractIdentifierElement(ident, *storage->identifier()).toString(), storage);
        }
      }
    }

    // now add all the aliases
    for(auto &alias : *ns.second->aliasList()) {
      dst.emplace(
        AbstractIdentifierElement(*root, *alias.second->alias()).toString(),
        alias.second
      );
    }
  }
}


bool Registry::parse(const Filesystem &filesystem, const std::string &file) {
  if(parse(filesystem, file, false) && parse(filesystem, "nyx.nyx", true)) {
    normalize(emit, global);
    normalize(ref, global);
    return true;
  }

  return false;
}


static auto make_alias(std::shared_ptr<AbstractIdentifierElement> original,
                       std::shared_ptr<AbstractIdentifierElement> alias) {
  return std::make_shared<AbstractAliasElement>(original, alias);
}


static auto make_identifier(std::shared_ptr<AbstractIdentifierElement> first,
                            std::shared_ptr<AbstractIdentifierElement> second) {
  return std::make_shared<AbstractIdentifierElement>(*first, *second);
}


bool Registry::parse(const Filesystem &filesystem, const std::string &file, bool search) {
  if(parsed.find(file) != parsed.end()) {
    return true; // don't reparse files
  }

  if(auto path = search ? filesystem.locate(file) : std::make_shared<std::string>(file)) {
    Tokenizer input(path);

    if(!input) {
      std::cerr << "Failure to open " << file << " for reading" << std::endl;
      return false;
    }

    if(auto tree = std::shared_ptr<AbstractSyntaxTree>(Parser::parse(input).release())) {
      parsed.emplace(file, tree);

      for(auto &ns : *tree) {
        if(ns.second->is(AbstractElementType::Namespace)) {
          emit.emplace(ns.first, ns.second);
        }
        else {
          ref.emplace(ns.first, ns.second);
        }

        auto aliases = ns.second->aliasList();

        for(auto import : *ns.second->importList()) {
          std::string name(import->module()->toString());

          if(!parse(filesystem, name.append(".nyx"), true)) {
            return false;
          }

          if(import->hasAlias()) {
            if(import->hasElement()) {
              aliases->add(make_alias(
                make_identifier(import->module(), import->element()),
                import->alias()
              ));
            }
            else {
              aliases->add(make_alias(import->module(), import->alias()));
            }
          }
          else if(import->hasElement()) {
            aliases->add(make_alias(
              make_identifier(import->module(), import->element()),
              import->element()
            ));
          }
        }
      }
      return true;
    }
  }
  else {
    std::cerr << "Failure to find import " << file << std::endl;
  }

  return false;
}


std::map<std::string, std::shared_ptr<AbstractElement>>::const_iterator
Registry::badResolve() const {
  return global.end();
}


std::map<std::string, std::shared_ptr<AbstractElement>>::const_iterator
Registry::resolve(const AbstractNamespaceElement &ctx, const AbstractIdentifierElement &base) const {
  if(base.size() == 1) {
    auto &name = base[0];
    // check the local rules first
    if(ctx[name->text()]) {
      return global.find(AbstractIdentifierElement(*ctx.identifier(), base).toString());
    }

    // try the aliases
    const auto &aliases = *ctx.aliasList();
    if(auto ptr = aliases[name->text()]) {
      auto &alias = *ptr->original();

      if(alias.size() == 1) { // internal alias?
        if(ctx[alias[0]->text()]) {
          return global.find(AbstractIdentifierElement(*ctx.identifier(), alias).toString());
        }
        else {
          // last ditch effort, check the nyx namespace
          return global.find(std::string("nyx.").append(alias[0]->text()));
        }
      }
      else {
        // all multi token aliases should be to fully quallified names
        return global.find(alias.toString());
      }
    }

    // last ditch effort, check the nyx namespace
    return global.find(std::string("nyx.").append(name->text()));
  }
  else if(base.size() > 1) {
    // probably an alias to a namespace and an element
    auto &name = base[0];
    const auto &aliases = *ctx.aliasList();

    if(auto ptr = aliases[name->text()]) {
      auto alias = ptr->alias()->toString();

      // build the fully qualified name
      for(int i = 1; i < base.size(); ++i) {
        alias.append(1, '.').append(base[i]->text());
      }

      // if the first element was an alias, then we are committed
      return global.find(alias);
    }

    // unaliased multi token identifier should be fully qualified names
    return global.find(base.toString());
  }

  return badResolve();
}

