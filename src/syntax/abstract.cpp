#include <nyx/syntax/abstract.h>


using namespace nyx::syntax;


AbstractElement::AbstractElement(AbstractElementType type):
  kind(type) {
}


AbstractElement::~AbstractElement() {
  // nothing to do here
}


namespace nyx {
  namespace syntax {

std::ostream &operator<<(std::ostream &os, const AbstractElement &ce) {
  ce.print(os);
  return os;
}


std::ostream &operator<<(std::ostream &os, const std::shared_ptr<const AbstractElement> &ce) {
  if(ce) {
    ce->print(os);
  }
  else {
    os << "(null)";
  }
}

  }
}


AbstractMultiTokenElement::AbstractMultiTokenElement(
    AbstractElementType type,
    std::shared_ptr<Token> simple):
  AbstractElement(type),
  tokens() {
  tokens.emplace_back(simple);
}


AbstractMultiTokenElement::AbstractMultiTokenElement(
    AbstractElementType type,
    const std::vector<std::shared_ptr<Token>> &compound):
  AbstractElement(type),
  tokens(compound) {
}


AbstractMultiTokenElement::AbstractMultiTokenElement(
    AbstractElementType type,
    iterator first,
    iterator end):
  AbstractElement(type),
  tokens(first, end) {
}


AbstractMultiTokenElement::~AbstractMultiTokenElement() {
  // nothing to do here
}


std::ostream &AbstractMultiTokenElement::print(std::ostream &os) const {
  auto iter = begin(), end = this->end();

  while(iter != end) {
    if(*iter) {
      os << (*iter)->text();
    }
    else {
      os << "(null)";
    }

    if(++iter != end) {
      os << ' ';
    }
  }

  return os;
}


std::ostream &AbstractMultiTokenElement::debug(std::ostream &os) const {
  for(auto &token : tokens) {
    if(token) {
      os << token->fileName()     << ":"
         << token->lineNumber()   << "."
         << token->columnNumber() << "  "
         << token->text()         << std::endl;
    }
    else {
      os << "(null)" << std::endl;
    }
  }

  return os;
}


void AbstractMultiTokenElement::add(std::shared_ptr<Token> token) {
  tokens.emplace_back(token);
}


AbstractIdentifierElement::AbstractIdentifierElement(std::shared_ptr<Token> simple):
  AbstractMultiTokenElement(AbstractElementType::Identifier, simple) {
}


AbstractIdentifierElement::AbstractIdentifierElement(
    const std::vector<std::shared_ptr<Token>> &compound):
  AbstractMultiTokenElement(AbstractElementType::Identifier, compound) {
}


AbstractIdentifierElement::AbstractIdentifierElement(iterator first, iterator end):
  AbstractMultiTokenElement(AbstractElementType::Identifier, first, end) {
}


AbstractIdentifierElement::~AbstractIdentifierElement() {
}


std::ostream &AbstractIdentifierElement::print(std::ostream &os) const {
  auto iter = begin(), end = this->end();

  while(iter != end) {
    if(*iter) {
      os << (*iter)->text();
    }
    else {
      os << "(null)";
    }

    if(++iter != end) {
      os << '.';
    }
  }

  return os;
}

std::string AbstractIdentifierElement::toString() const {
  std::string retVal;
  auto count = size();
  for(auto &token : tokens) {
    if(token) {
      count += token->text().size();
    }
  }

  retVal.reserve(count);
  for(auto &token : tokens) {
    retVal.append(token->text()).append(1, '.');
  }

  if(retVal.size() > 0) {
    retVal.resize(retVal.size() - 1);
  }

  return retVal;
}


std::shared_ptr<AbstractImportElement>
AbstractImportElement::importModule(std::shared_ptr<AbstractIdentifierElement> name) {
  return std::shared_ptr<AbstractImportElement>(new AbstractImportElement(name, nullptr, nullptr));
}

std::shared_ptr<AbstractImportElement>
AbstractImportElement::importModule(std::shared_ptr<AbstractIdentifierElement> name,
                                    std::shared_ptr<AbstractIdentifierElement> alias) {
  return std::shared_ptr<AbstractImportElement>(new AbstractImportElement(name, alias, nullptr));
}

std::shared_ptr<AbstractImportElement>
AbstractImportElement::importElement(std::shared_ptr<AbstractIdentifierElement> name,
                                     std::shared_ptr<AbstractIdentifierElement> module) {
  return std::shared_ptr<AbstractImportElement>(new AbstractImportElement(module, nullptr, name));
}

std::shared_ptr<AbstractImportElement>
AbstractImportElement::importElement(std::shared_ptr<AbstractIdentifierElement> name,
                                     std::shared_ptr<AbstractIdentifierElement> module,
                                     std::shared_ptr<AbstractIdentifierElement> alias) {
  return std::shared_ptr<AbstractImportElement>(new AbstractImportElement(module, alias, name));
}


AbstractImportElement::~AbstractImportElement() {
  // nothing to do here
}


std::ostream &AbstractImportElement::print(std::ostream &os) const {
  os << "Import: ";

  if(element_ptr) {
    os << *element_ptr << " from ";
  }

  if(module_ptr) {
    os << *module_ptr;
  }
  else {
    os << "(null)";
  }

  if(alias_ptr) {
    os << " as " << *alias_ptr;
  }

  os << std::endl;

  return os;
}


std::ostream &AbstractImportElement::debug(std::ostream &os) const {
  os << "Import:" << std::endl << "Module:" << std::endl;

  if(module_ptr) {
    module_ptr->debug(os);
  }
  else {
    os << "(null)" << std::endl;
  }

  os << "Element:" << std::endl;

  if(element_ptr) {
    element_ptr->debug(os);
  }
  else {
    os << "(null)" << std::endl;
  }

  os << "Alias:" << std::endl;

  if(alias_ptr) {
    alias_ptr->debug(os);
  }
  else {
    os << "(null)" << std::endl;
  }

  return os;
}


AbstractImportElement::AbstractImportElement(std::shared_ptr<AbstractIdentifierElement> module,
                                             std::shared_ptr<AbstractIdentifierElement> alias,
                                             std::shared_ptr<AbstractIdentifierElement> element):
  AbstractElement(AbstractElementType::Import),
  module_ptr(module),
  alias_ptr(alias),
  element_ptr(element) {
}


AbstractImportList::AbstractImportList():
  AbstractCompoundElement<AbstractImportElement>(AbstractElementType::ImportList) {
}


AbstractImportList::AbstractImportList(std::shared_ptr<AbstractImportElement> simple):
  AbstractCompoundElement<AbstractImportElement>(AbstractElementType::ImportList, simple) {
}


AbstractImportList::AbstractImportList(
    const std::vector<std::shared_ptr<AbstractImportElement>> &multi):
  AbstractCompoundElement<AbstractImportElement>(AbstractElementType::ImportList, multi) {
}


AbstractImportList::AbstractImportList(iterator first, iterator end):
  AbstractCompoundElement<AbstractImportElement>(AbstractElementType::ImportList, first, end) {
}


AbstractImportList::~AbstractImportList() {
  // nothing to do here
}


std::ostream &AbstractImportList::print(std::ostream &os) const {
  auto iter = begin(), end = this->end();

  while(iter != end) {
    os << *iter;

    if(++iter != end) {
      os << std::endl;
    }
  }

  return os;
}


void AbstractImportList::add(std::shared_ptr<AbstractImportElement> import) {
  elements.emplace_back(import);
}


AbstractAliasElement::AbstractAliasElement(
    std::shared_ptr<AbstractIdentifierElement> original,
    std::shared_ptr<AbstractIdentifierElement> alias):
  AbstractElement(AbstractElementType::Alias),
  original_ptr(original),
  alias_ptr(alias) {
}


AbstractAliasElement::~AbstractAliasElement() {
  // nothing to do here
}


std::ostream &AbstractAliasElement::print(std::ostream &os) const {
  os << "Alias: " << original_ptr << " as " << alias_ptr;
  return os;
}


std::ostream &AbstractAliasElement::debug(std::ostream &os) const {
  os << "Alias:" << std::endl << "Original: ";
  if(original_ptr) {
    original_ptr->debug(os);
  }
  else {
    os << "(null)" << std::endl;
  }

  os << "New: ";
  if(alias_ptr) {
    alias_ptr->debug(os);
  }
  else {
    os << "(null)" << std::endl;
  }

  return os;
}


AbstractAliasList::AbstractAliasList():
  AbstractCompoundElement<AbstractAliasElement>(AbstractElementType::AliasList) {
}


AbstractAliasList::AbstractAliasList(std::shared_ptr<AbstractAliasElement> simple):
  AbstractCompoundElement<AbstractAliasElement>(AbstractElementType::AliasList, simple) {
}


AbstractAliasList::AbstractAliasList(const std::vector<std::shared_ptr<AbstractAliasElement>> &multi):
  AbstractCompoundElement<AbstractAliasElement>(AbstractElementType::AliasList, multi) {
}


AbstractAliasList::AbstractAliasList(iterator first, iterator end):
  AbstractCompoundElement<AbstractAliasElement>(AbstractElementType::AliasList, first, end) {
}


AbstractAliasList::~AbstractAliasList() {
  //nothing to do here
}


std::ostream &AbstractAliasList::print(std::ostream &os) const {
  auto iter = begin(), end = this->end();

  while(iter != end) {
    os << *iter;

    if(++iter != end) {
      os << std::endl;
    }
  }

  return os;
}


void AbstractAliasList::add(std::shared_ptr<AbstractAliasElement> alias) {
  elements.emplace_back(alias);
}


AbstractPatternElement::AbstractPatternElement(std::shared_ptr<Token> token,
                                               std::shared_ptr<Token> lower,
                                               std::shared_ptr<Token> upper,
                                               std::shared_ptr<Token> bind):
  AbstractElement(AbstractElementType::PatternElement),
  elem(token),
  min(lower),
  max(upper),
  ident(bind) {
}


AbstractPatternElement::~AbstractPatternElement() {
  // nothing to do here
}


std::ostream &AbstractPatternElement::print(std::ostream &os) const {
  os << "Element: ";
  if(elem) {
    os  << elem->text();
  }
  else {
    os << "(null)";
  }

  if(min || max) {
    os << " from ";
    if(min) {
      os  << min->text();
    }
    else {
      os << "(null)";
    }
    os << " to ";
    if(max) {
      os  << max->text();
    }
    else {
      os << "(null)";
    }
  }

  if(ident) {
    os << " as " << ident->text();
  }

  return os;
}


std::ostream &AbstractPatternElement::debug(std::ostream &os) const {
  os << "Element: ";
  if(elem) {
    os << elem->fileName()     << ":"
       << elem->lineNumber()   << "."
       << elem->columnNumber() << "  "
       << elem->text()         << std::endl;
  }
  else {
    os << "(null)" << std::endl;
  }

  os << "Lower: ";
  if(min) {
    os << min->fileName()     << ":"
       << min->lineNumber()   << "."
       << min->columnNumber() << "  "
       << min->text()         << std::endl;
  }
  else {
    os << "(null)" << std::endl;
  }

  os << "Upper: ";
  if(max) {
    os << max->fileName()     << ":"
       << max->lineNumber()   << "."
       << max->columnNumber() << "  "
       << max->text()         << std::endl;
  }
  else {
    os << "(null)" << std::endl;
  }

  os << "Identifier: ";
  if(ident) {
    os << ident->fileName()     << ":"
       << ident->lineNumber()   << "."
       << ident->columnNumber() << "  "
       << ident->text()         << std::endl;
  }
  else {
    os << "(null)" << std::endl;
  }

  return os;
}


AbstractPatternGroup::AbstractPatternGroup(std::shared_ptr<AbstractPatternElement> simple,
                                           std::shared_ptr<Token>                  lower,
                                           std::shared_ptr<Token>                  upper,
                                           std::shared_ptr<Token>                  bind):
  AbstractCompoundElement<AbstractPatternElement>(AbstractElementType::PatternGroup, simple),
  min(lower),
  max(upper),
  ident(bind) {
}


AbstractPatternGroup::~AbstractPatternGroup(){
  // nothing to do here
}


std::ostream &AbstractPatternGroup::print(std::ostream &os) const {
  os << "Group:";

  if(min || max) {
    os << " from ";
    if(min) {
      os  << min->text();
    }
    else {
      os << "(null)";
    }
    os << " to ";
    if(max) {
      os  << max->text();
    }
    else {
      os << "(null)";
    }
  }

  if(ident) {
    os << " as " << ident->text();
  }

  os << std::endl;

  for(auto &pattern : elements) {
    os << pattern << std::endl;
  }

  return os;
}


void AbstractPatternGroup::add(std::shared_ptr<AbstractPatternElement> element) {
  elements.emplace_back(element);
}


AbstractPatternAlternates::AbstractPatternAlternates(std::shared_ptr<AbstractPatternGroup> simple):
  AbstractCompoundElement<AbstractPatternGroup>(AbstractElementType::Pattern, simple) {
}


AbstractPatternAlternates::~AbstractPatternAlternates() {
  // nothing to do here
}


std::ostream &AbstractPatternAlternates::print(std::ostream &os) const {
  auto iter = begin(), end = this->end();
  os << "Alternates:" << std::endl;

  while(iter != end) {
    os << *iter;

    if(++iter != end) {
      os << " |";
    }

    os << std::endl;
  }

  return os;
}


void AbstractPatternAlternates::add(std::shared_ptr<AbstractPatternGroup> group) {
  elements.emplace_back(group);
}


AbstractPatternList::AbstractPatternList(std::shared_ptr<AbstractPatternAlternates> simple):
  AbstractCompoundElement<AbstractPatternAlternates>(AbstractElementType::Pattern, simple) {
}

AbstractPatternList::~AbstractPatternList() {
  // nothing to do here
}


std::ostream &AbstractPatternList::print(std::ostream &os) const {
  os << "Patterns:" << std::endl;

  for(auto &pattern : elements) {
    os << pattern << std::endl;
  }

  return os;
}


void AbstractPatternList::add(std::shared_ptr<AbstractPatternAlternates> alt) {
  elements.emplace_back(alt);
}


AbstractStorageElement::AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name):
  AbstractElement(AbstractElementType::StorageElement),
  ident(name),
  kind(nullptr) {
}


AbstractStorageElement::AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name,
                                               std::shared_ptr<AbstractIdentifierElement> type):
  AbstractElement(AbstractElementType::StorageElement),
  ident(name),
  kind(type) {
}


AbstractStorageElement::~AbstractStorageElement(){
  // nothing to do here
}

std::ostream &AbstractStorageElement::print(std::ostream &os) const {
  os << "Storage: " << ident;

  if(kind) {
    os << " as " << kind;
  }

  return os;
}


std::ostream &AbstractStorageElement::debug(std::ostream &os) const {
  os << "Storage:" << std::endl << "Name: ";
  if(ident) {
    ident->debug(os);
  }
  else {
    os << "(null)";
  }

  os << "Type: ";
  if(kind) {
    kind->debug(os);
  }
  else {
    os << "(null)";
  }

  return os;
};


//class AbstractStorageList: public AbstractCompoundElement<AbstractStorageElement> {
AbstractStorageList::AbstractStorageList(std::shared_ptr<AbstractStorageElement> simple):
  AbstractCompoundElement<AbstractStorageElement>(AbstractElementType::StorageList, simple) {
}


AbstractStorageList::~AbstractStorageList() {
  // nothing to do here
}


std::ostream &AbstractStorageList::print(std::ostream &os) const {
  os << "StorageList:" << std::endl;

  for(auto &storage : elements) {
    os << storage << std::endl;
  }

  return os;
}


void AbstractStorageList::add(std::shared_ptr<AbstractStorageElement> storage) {
  elements.emplace_back(storage);
}


AbstractSExpr::AbstractSExpr():
  tok_val(nullptr),
  sexpr_val(nullptr),
  sexpr_next(nullptr) {
}


AbstractSExpr::AbstractSExpr(std::shared_ptr<Token> token):
  tok_val(token),
  sexpr_val(nullptr),
  sexpr_next(nullptr) {
}


AbstractSExpr::AbstractSExpr(std::shared_ptr<AbstractSExpr> sexpr):
  tok_val(nullptr),
  sexpr_val(sexpr),
  sexpr_next(nullptr) {
}


std::ostream &AbstractSExpr::print(std::ostream &os) const {
  auto ptr = this;
  os << '(';

  while(ptr != nullptr) {
    if(ptr->isToken()) {
      os << ptr->token()->text();
    }
    else if(ptr->isSexpr()) {
      ptr->sexpr()->print(os);
    }
    else {
      os << "[nil]";
    }

    if((ptr = ptr->next().get()) != nullptr) {
      os << ' ';
    }
  }
  os << ')';

  return os;
}


std::ostream &AbstractSExpr::debug(std::ostream &os) const {
  return print(os);
}


AbstractCodeSnippet::AbstractCodeSnippet(std::shared_ptr<AbstractSExpr> sexpr):
  AbstractElement(AbstractElementType::Code),
  expr(sexpr) {
}


AbstractCodeSnippet::~AbstractCodeSnippet() {
  // nothing to do here
}


std::ostream &AbstractCodeSnippet::print(std::ostream &os) const {
  os << "Code: ";

  if(expr) {
    expr->print(os);
  }
  else {
    os << "(null)";
  }

  return os;
}


std::ostream &AbstractCodeSnippet::debug(std::ostream &os) const {
  print(os);
  os << std::endl;
  return os;
}


AbstractRuleElement::AbstractRuleElement(std::shared_ptr<AbstractIdentifierElement> identifier,
                                         std::shared_ptr<AbstractPatternList>       pattern,
                                         std::shared_ptr<AbstractStorageList>       storage,
                                         std::shared_ptr<AbstractCodeSnippet>       validation,
                                         std::shared_ptr<AbstractCodeSnippet>       encode,
                                         std::shared_ptr<AbstractCodeSnippet>       decode):
  AbstractElement(AbstractElementType::Rule),
  ident(identifier),
  pat(pattern),
  store(storage),
  validate(validation),
  enc(encode),
  dec(decode) {
}


AbstractRuleElement::~AbstractRuleElement() {
  // nothing to do here
}


std::ostream &AbstractRuleElement::print(std::ostream &os) const {
  os << "Rule:"  << std::endl;
  os << ident    << std::endl;
  os << pat      << std::endl;
  os << store    << std::endl;
  os << validate << std::endl;
  os << enc      << std::endl;
  os << dec      << std::endl;

  return os;
}


std::ostream &AbstractRuleElement::debug(std::ostream &os) const {
  print(os);
  os << std::endl;
  return os;
}


AbstractNamespaceElement::AbstractNamespaceElement(std::shared_ptr<AbstractIdentifierElement> name,
                                                   bool                                        module):
  AbstractElement(module ? AbstractElementType::Module : AbstractElementType::Namespace),
  ident(name) {
}


AbstractNamespaceElement::~AbstractNamespaceElement() {
  // nothing to do here
}

std::ostream &AbstractNamespaceElement::print(std::ostream &os) const {
  os << "Namespace: " << ident << std::endl;

  for(auto iter = begin(), end = this->end(); iter !=end; ++iter) {
    os << iter->second << std::endl;
  }

  return os;
}


std::ostream &AbstractNamespaceElement::debug(std::ostream &os) const {
  print(os);
  return os;
}


void AbstractNamespaceElement::add(std::shared_ptr<AbstractRuleElement> rule) {
  if(rule) {
    if(auto identifier = rule->identifier()) {
      auto name = identifier->toString();
      elements[name] = rule;
    }
  }
}


AbstractSyntaxTree::AbstractSyntaxTree():
  spaces() {
}


std::ostream &AbstractSyntaxTree::print(std::ostream &os) const {
  os << "AST:" << std::endl;

  for(auto &ns : spaces) {
    os << ns << std::endl;
  }

  return os;
}


std::ostream &AbstractSyntaxTree::debug(std::ostream &os) const {
  print(os);
  return os;
}


std::shared_ptr<AbstractNamespaceElement> AbstractSyntaxTree::currentNamespace() {
  return spaces.back();
}


std::shared_ptr<AbstractNamespaceElement>
AbstractSyntaxTree::addNamespace(std::shared_ptr<AbstractIdentifierElement> ident, bool emit) {
  spaces.emplace_back(std::make_shared<AbstractNamespaceElement>(ident, emit));
  return currentNamespace();
}

