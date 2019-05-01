#include <nyx/plan.h>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <stddef.h>
#include <algorithm>


using namespace nyx;
using namespace nyx::syntax;


static std::vector<std::string> toVector(const std::shared_ptr<AbstractIdentifierElement> &ident) {
  std::vector<std::string> vec;
  if(ident) {
    vec.reserve(ident->size());
    for(auto &tok : *ident) {
      vec.emplace_back(tok->text());
    }
  }
  return vec;
}


Stage::Stage() {
}


Stage::Stage(const nyx::syntax::AbstractMatchElement &match) {

}


Stage::Stage(const nyx::syntax::AbstractSimplePatternElement &simple):
  stage(nullptr),
  sub(nullptr) {

  if(simple.isToken()) {
    pat = simple.token()->text();
    what = simple.token()->lexeme();
  }
  else {
    pat = simple.identifier()->toString();
    what = Lexeme::Identifier;
  }

  if(simple.hasMinimum()) {
    min = simple.minimum()->text();

    if(simple.hasMaximum()) {
      max = simple.maximum()->text();

      if(max == "*") {
        max = "-1";
      }
    }
    else {
      if(min == "*") {
        min =  "0";
        max = "-1";
      }
      else if(min == "+") {
        min =  "1";
        max = "-1";
      }
      else if(min == "?") {
        min = "0";
        max = "1";
      }
      else {
        max = min;
      }
    }
  }
  else {
    min = max = "1";
  }

  if(simple.hasBinding()) {
    ident = simple.binding()->text();
  }
}


static auto make_stage(const AbstractPatternElement &pat) {
  if(pat.is(AbstractElementType::SimplePattern)) {
    return std::make_unique<Stage>(*reinterpret_cast<const AbstractSimplePatternElement *>(&pat));
  }
  else if(pat.is(AbstractElementType::CompoundPattern)) {
    return std::make_unique<Stage>(*reinterpret_cast<const AbstractCompoundPatternElement *>(&pat));
  }
  else if(pat.is(AbstractElementType::Match)) {
    return std::make_unique<Stage>(*reinterpret_cast<const AbstractMatchElement *>(&pat));
  }

  return std::unique_ptr<Stage>(nullptr);
}


Stage::Stage(const nyx::syntax::AbstractCompoundPatternElement &compound):
  stage(nullptr) {

  auto iter = compound.begin();
  sub = make_stage(**iter);
  auto parent = sub.get();
  for(auto end = compound.end(); ++iter != end; parent = parent->stage.get()) {
    parent->stage = make_stage(**iter);
  }

  if(compound.hasMinimum()) {
    min = compound.minimum()->text();

    if(compound.hasMaximum()) {
      max = compound.maximum()->text();
    }
    else {
      if(min == "*") {
        min =  "0";
        max = "-1";
      }
      else if(min == "+") {
        min =  "1";
        max = "-1";
      }
      else if(min == "?") {
        min = "0";
        max = "1";
      }
      else {
        max = min;
      }
    }
  }
  else {
    min = max = "1";
  }

  if(compound.hasBinding()) {
    ident = compound.binding()->text();
  }
}


Stage::Stage(const Stage &that):
  stage(static_cast<bool>(that.stage) ? new Stage(*that.stage) : nullptr),
  sub(static_cast<bool>(that.sub) ? new Stage(*that.sub) : nullptr),
  min(that.min),
  max(that.max),
  ident(that.ident),
  pat(that.pat),
  what(that.what) {
}

Stage& Stage::operator=(const Stage &that) {
  stage.reset(static_cast<bool>(that.stage) ? new Stage(*that.stage) : nullptr);
  sub.reset(static_cast<bool>(that.sub) ? new Stage(*that.sub) : nullptr);
  min =   that.min;
  max =   that.max;
  ident = that.ident;
  pat =   that.pat;
  what =  that.what;
}


Alternate::Alternate(const AbstractPatternElement &pat) {
  if(pat.is(AbstractElementType::SimplePattern)) {
    stage = Stage(*reinterpret_cast<const AbstractSimplePatternElement *>(&pat));
  }
  else if(pat.is(AbstractElementType::CompoundPattern)) {
    stage = Stage(*reinterpret_cast<const AbstractCompoundPatternElement *>(&pat));
  }
}


Pattern::Pattern(const nyx::syntax::AbstractPatternList &patterns) {
  list.reserve(patterns.size());

  for(auto &alt : patterns) {
    list.emplace_back(*alt);
  }
}


Storage::Storage(const std::shared_ptr<AbstractStorageList> &list) {
  if(list) {
    members.reserve(list->size());

    for(auto &element : *list) {
      members.emplace_back((*element->identifier())[0]->text(), toVector(element->type()));
    }
  }
}


Code::Code(const std::shared_ptr<nyx::syntax::AbstractCodeSnippet> &snippet) {
  if(snippet) {
    expr = snippet->sexpr();
  }
}


Rule::Rule(const AbstractRuleElement &rule):
  ident((*rule.identifier())[0]->text()),
  pat(*rule.pattern()),
  store(rule.storage()),
  enc(rule.encode()),
  dec(rule.decode()),
  val(rule.validation())
{

}


Import::Import(const nyx::syntax::AbstractImportElement &import) {
  space.reserve(import.module()->size());
  for(auto &tok : *import.module()) {
    space.emplace_back(tok->text());
  }

  if(import.hasElement()) {
    mem.assign((*import.element())[0]->text());
  }

  if(import.hasAlias()) {
    ident.assign((*import.alias())[0]->text());
  }
}


Namespace::Namespace(const AbstractIdentifierElement &ident) {
  module.reserve(ident.size());
  for(auto &token : ident) {
    module.emplace_back(token->text());
  }
}


void Namespace::addRule(const AbstractRuleElement &rule) {
  members.emplace_back(rule);
}


void Namespace::addImport(const nyx::syntax::AbstractImportElement &import) {
  requires.emplace_back(import);
}


Plan::Plan() {
}


struct Dependency {
  Dependency(): rule(nullptr) {}
  Dependency(const std::string &name,
             const AbstractNamespaceElement *parent,
             std::shared_ptr<AbstractRuleElement> r): fqn(name), space(parent), rule(r) {}

  ~Dependency() {
    space = nullptr; // not owned by us
  }

  std::string fqn;
  const AbstractNamespaceElement *space;
  std::shared_ptr<AbstractRuleElement> rule;
  std::vector<std::shared_ptr<Dependency>> needs;

  size_t depth() const {
    size_t children = 0;

    for(auto &child : needs) {
      children = std::max(child->depth(), children);
    }

    return children + 1;
  }
};


template<typename ABSTRACT>
static ABSTRACT &as(std::shared_ptr<AbstractPatternElement> &abstract) {
  return *std::dynamic_pointer_cast<ABSTRACT>(abstract);
}


template<typename ABSTRACT>
static const ABSTRACT &as(const std::shared_ptr<AbstractElement> &abstract) {
  return *std::dynamic_pointer_cast<ABSTRACT>(abstract);
}


static bool traceDependencies(Registry                                           &reg,
                              std::map<std::string, std::shared_ptr<Dependency>> &deps,
                              const AbstractNamespaceElement                     &ns,
                              std::shared_ptr<AbstractRuleElement>                rule,
                              Dependency                                         &dep);


static void missingNamespace(Registry &reg, const std::string &ns) {
  for(auto &entry : reg.fullyQualified()) {
    entry.second->print(std::cerr << entry.first << ": ") << std::endl;
  }
  std::cerr << "Impossibly missing namespace: " << ns << std::endl;
}


static bool traceDependencies(Registry                                           &reg,
                              std::map<std::string, std::shared_ptr<Dependency>> &deps,
                              const AbstractNamespaceElement                     &ns,
                              AbstractIdentifierElement                          &ident,
                              Dependency                                         &dep) {
  auto iter = reg.resolve(ns, ident);

  if(iter == reg.badResolve()) {
    std::cerr << "Symbol '" << ident.toString() << "' not defined in current scope" << std::endl;
    return false;
  }

  switch(iter->second->type()) {
    case AbstractElementType::Rule: {
      auto name = iter->first.substr(0, iter->first.rfind('.'));
      auto space = reg.resolve(name);

      if(space != reg.badResolve()) {
        if(!traceDependencies(
            reg,
            deps,
            as<AbstractNamespaceElement>(space->second),
            std::dynamic_pointer_cast<AbstractRuleElement>(
              std::const_pointer_cast<AbstractElement>(iter->second)
            ),
            dep)) {
          std::cerr << "Failed Identifier:52" << std::endl;
          return false;
        }
      }
      else {
        missingNamespace(reg, name);
        return false;
      }
    } break;

    case AbstractElementType::Alias: {
      auto name = iter->first.substr(0, iter->first.rfind('.'));
      if(name == "nyx") {
        return true; // we are done here
      }

      auto space = reg.resolve(name);

      if(space != reg.badResolve()) {
        std::cerr << "Ignoring alias: " << name << std::endl;
      }
      else {
        missingNamespace(reg, name);
        return false;
      }
    } break;

    default:
      std::cerr << "Unexpected resolve type: " << toString(iter->second->type()) << std::endl;
      iter->second->print(std::cerr) << std::endl;
      return false;
    break;
  }

  return true;
}


static bool traceDependencies(Registry                                           &reg,
                              std::map<std::string, std::shared_ptr<Dependency>> &deps,
                              const AbstractNamespaceElement                     &ns,
                              AbstractSimplePatternElement                       &simple,
                              Dependency                                         &dep) {
  if(simple.isIdentifier()) {
    return traceDependencies(reg, deps, ns, *simple.identifier(), dep);
  }

  return true;
}


static bool traceDependencies(Registry                                           &reg,
                              std::map<std::string, std::shared_ptr<Dependency>> &deps,
                              const AbstractNamespaceElement                     &ns,
                              AbstractMatchElement                               &match,
                              Dependency                                         &dep) {
  for(auto &mc : match) {
    if(!traceDependencies(reg, deps, ns, *mc->value(), dep)) {
      return false;
    }
  }

  return true;
}


static bool traceDependencies(Registry                                           &reg,
                              std::map<std::string, std::shared_ptr<Dependency>> &deps,
                              const AbstractNamespaceElement                     &ns,
                              AbstractCompoundPatternElement                     &compound,
                              Dependency                                         &dep) {
  for(auto &pattern : compound) {
    if(pattern->is(AbstractElementType::SimplePattern)) {
      if(!traceDependencies(reg, deps, ns, as<AbstractSimplePatternElement>(pattern), dep)) {
        return false;
      }
    }
    else if(pattern->is(AbstractElementType::CompoundPattern)) {
      if(!traceDependencies(reg, deps, ns, as<AbstractCompoundPatternElement>(pattern), dep)) {
        return false;
      }
    }
    else if(pattern->is(AbstractElementType::Match)) {
      if(!traceDependencies(reg, deps, ns, as<AbstractMatchElement>(pattern), dep)) {
        return false;
      }
    }
    else {
      std::cerr << "Unexpected AST type: " << toString(pattern->type()) << std::endl;
      return false;
    }
  }

  return true;
}


static bool traceDependencies(Registry                                           &reg,
                              std::map<std::string, std::shared_ptr<Dependency>> &deps,
                              const AbstractNamespaceElement                     &ns,
                              std::shared_ptr<AbstractRuleElement>                rule,
                              Dependency                                         &dep) {
  auto fqn  = AbstractIdentifierElement(*ns.identifier(), *rule->identifier()).toString();
  auto iter = deps.find(fqn);

  if(iter != deps.end()) {
    dep.needs.emplace_back(iter->second);
    return true; // already done, is a dependency for some other rule
  }

  dep.needs.emplace_back(deps.emplace(fqn, std::make_shared<Dependency>(fqn, &ns, rule)).first->second);
  auto &depend = *dep.needs.back();

  for(auto &pattern : *rule->pattern()) {
    if(pattern->is(AbstractElementType::SimplePattern)) {
      if(!traceDependencies(reg, deps, ns, as<AbstractSimplePatternElement>(pattern), depend)) {
        return false;
      }
    }
    else if(pattern->is(AbstractElementType::CompoundPattern)) {
      if(!traceDependencies(reg, deps, ns, as<AbstractCompoundPatternElement>(pattern), depend)) {
        return false;
      }
    }
    else {
      std::cerr << "Unexpected AST type: " << toString(pattern->type()) << std::endl;
      return false;
    }
  }

  return true;
}


static bool traceDependencies(Registry                                           &reg,
                              std::map<std::string, std::shared_ptr<Dependency>> &deps,
                              const AbstractNamespaceElement                     &ns,
                              std::shared_ptr<AbstractRuleElement>                rule) {
  auto fqn  = AbstractIdentifierElement(*ns.identifier(), *rule->identifier()).toString();
  auto iter = deps.find(fqn);

  if(iter != deps.end()) {
    return true; // already done, is a dependency for some other rule
  }

  auto &dep = *deps.emplace(fqn, std::make_shared<Dependency>(fqn, &ns, rule)).first->second;

  for(auto &pattern : *rule->pattern()) {
    if(pattern->is(AbstractElementType::SimplePattern)) {
      if(!traceDependencies(reg, deps, ns, as<AbstractSimplePatternElement>(pattern), dep)) {
        return false;
      }
    }
    else if(pattern->is(AbstractElementType::CompoundPattern)) {
      if(!traceDependencies(reg, deps, ns, as<AbstractCompoundPatternElement>(pattern), dep)) {
        return false;
      }
    }
    else {
      std::cerr << "Unexpected AST type: " << toString(pattern->type()) << std::endl;
      return false;
    }
  }

  return true;
}


static void add(std::set<std::string>                                           &seen,
                std::map<std::string, std::vector<std::shared_ptr<Dependency>>> &spaces,
                std::shared_ptr<Dependency>                                     &dep) {
  if(seen.find(dep->fqn) != seen.end()) {
    return; // already been added to the list of outputs
  }

  seen.emplace(dep->fqn);
  // add the dependencies first
  for(auto &sub : dep->needs) {
    add(seen, spaces, sub);
  }

  // add self last
  spaces[dep->space->identifier()->toString()].emplace_back(dep);
}


std::unique_ptr<Plan> Plan::generate(Registry &reg) {
  // multi root dependency tree
  std::map<std::string, std::shared_ptr<Dependency>> deps;

  for(auto &ns : reg.namespaces()) {
    for(auto &r : *ns.second) {
      if(!traceDependencies(reg, deps, *ns.second, r.second)) {
        return nullptr;
      }
    }
  }

  // sort by dependency depth
  std::vector<std::pair<size_t, std::shared_ptr<Dependency>>> order;
  for(auto &entry : deps) {
    order.emplace_back(entry.second->depth(), entry.second);
  }

  std::sort(order.begin(), order.end(), [](const decltype(order)::value_type &lhs,
                                           const decltype(order)::value_type &rhs) {
    return lhs.first > rhs.first;
  });

  // generate output order for each namsespace
  std::set<std::string> seen;
  std::map<std::string, std::vector<std::shared_ptr<Dependency>>> spaces;
  for(auto &dep : order) {
    add(seen, spaces, dep.second);
  }

  // create an output plan for each namespace
  auto plan = std::make_unique<Plan>();

  for(auto &entry : spaces) {
    auto &ans = as<AbstractNamespaceElement>(reg.fullyQualified()[entry.first]);
    auto &ns = plan->addNamespace(*ans.identifier());

    // add imports to the namespace
    for(auto &import : *ans.importList()) {
      ns.addImport(*import);
    }

    // add rules to the namespace
    for(auto &dep : entry.second) {
      ns.addRule(*dep->rule);
    }
  }

  return plan;
}

Namespace &Plan::addNamespace(const AbstractIdentifierElement &ident) {
  spaces.emplace_back(ident);
  return spaces.back();
}
