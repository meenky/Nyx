#include <nyx/syntax/abstract.h>

#include <array>


using namespace nyx::syntax;


static const std::array<const char *, 16> ABSTRACT_ELEMENT_TYPE_STR{
  "Alias",           "AliasList",  "Code",           "Identifier",
  "Import",          "ImportList", "Match",          "MatchCase",
  "Module",          "Namespace",  "Pattern",        "SimplePattern",
  "CompoundPattern", "Rule",       "StorageElement", "StorageList"
};


const char *nyx::syntax::toString(AbstractElementType type) {
  if(static_cast<int>(type) <= static_cast<int>(AbstractElementType::StorageList)) {
    return ABSTRACT_ELEMENT_TYPE_STR[static_cast<int>(type)];
  }

  return "<INVALID>";
}


AbstractElement::AbstractElement(AbstractElementType          type,
                                 int                          line,
                                 int                          column,
                                 std::shared_ptr<std::string> ptr):
  fileName(ptr),
  kind(type),
  lineNumber(line),
  columnNumber(column) {
}


AbstractElement::~AbstractElement() {
  // nothing to do here
}


static const std::string INVALID("<INVALID>");


int AbstractElement::line() const {
  return lineNumber;
}


int AbstractElement::column() const {
  return columnNumber;
}


const std::string &AbstractElement::file() const {
  if(fileName) {
    return *fileName;
  }
  else {
    return INVALID;
  }
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


AbstractMultiTokenElement::AbstractMultiTokenElement(AbstractElementType type):
  AbstractElement(type),
  AbstractCompoundMixin<Token>() {
}


AbstractMultiTokenElement::AbstractMultiTokenElement(
    AbstractElementType type,
    std::shared_ptr<Token> simple):
  AbstractElement(type),
  AbstractCompoundMixin<Token>(simple) {
}


AbstractMultiTokenElement::AbstractMultiTokenElement(
    AbstractElementType type,
    const std::vector<std::shared_ptr<Token>> &compound):
  AbstractElement(type),
  AbstractCompoundMixin<Token>(compound) {
}


AbstractMultiTokenElement::AbstractMultiTokenElement(
    AbstractElementType type,
    iterator first,
    iterator end):
  AbstractElement(type),
  AbstractCompoundMixin<Token>(first, end) {
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
  for(auto &token : elements) {
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
  elements.emplace_back(token);
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


AbstractIdentifierElement::AbstractIdentifierElement(const AbstractIdentifierElement &first,
                                                     const AbstractIdentifierElement &second):
  AbstractMultiTokenElement(AbstractElementType::Identifier) {
  elements.reserve(first.size() + second.size());
  elements.insert(elements.end(), first.begin(), first.end());
  elements.insert(elements.end(), second.begin(), second.end());
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
  for(auto &token : elements) {
    if(token) {
      count += token->text().size();
    }
  }

  retVal.reserve(count);
  for(auto &token : elements) {
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
    element_ptr->print(os) << " from ";
  }

  if(module_ptr) {
    module_ptr->print(os);
  }
  else {
    os << "(null)";
  }

  if(alias_ptr) {
    alias_ptr->print(os << " as ");
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
  AbstractElement(AbstractElementType::ImportList),
  AbstractCompoundMixin<AbstractImportElement>() {
}


AbstractImportList::AbstractImportList(std::shared_ptr<AbstractImportElement> simple):
  AbstractElement(AbstractElementType::ImportList),
  AbstractCompoundMixin<AbstractImportElement>(simple) {
}


AbstractImportList::AbstractImportList(
    const std::vector<std::shared_ptr<AbstractImportElement>> &multi):
  AbstractElement(AbstractElementType::ImportList),
  AbstractCompoundMixin<AbstractImportElement>(multi) {
}


AbstractImportList::AbstractImportList(iterator first, iterator end):
  AbstractElement(AbstractElementType::ImportList),
  AbstractCompoundMixin<AbstractImportElement>(first, end) {
}


AbstractImportList::~AbstractImportList() {
  // nothing to do here
}


std::ostream &AbstractImportList::print(std::ostream &os) const {
  for(auto &import : *this) {
    import->print(os) << std::endl;
  }

  return os;
}


std::ostream &AbstractImportList::debug(std::ostream &os) const {
  for(auto &import : *this) {
    import->debug(os) << std::endl;
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
  alias_ptr->print(original_ptr->print(os << "Alias: ") << " as ");
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
  AbstractElement(AbstractElementType::AliasList),
  AbstractLookupMixin<AbstractAliasElement>() {
}


AbstractAliasList::AbstractAliasList(const std::string &key,
                                     std::shared_ptr<AbstractAliasElement> simple):
  AbstractElement(AbstractElementType::AliasList),
  AbstractLookupMixin<AbstractAliasElement>(key, simple) {
}


AbstractAliasList::AbstractAliasList(
    const std::map<std::string, std::shared_ptr<AbstractAliasElement>> &multi):
  AbstractElement(AbstractElementType::AliasList),
  AbstractLookupMixin<AbstractAliasElement>(multi) {
}


AbstractAliasList::AbstractAliasList(iterator first, iterator end):
  AbstractElement(AbstractElementType::AliasList),
  AbstractLookupMixin<AbstractAliasElement>(first, end) {
}


AbstractAliasList::~AbstractAliasList() {
  //nothing to do here
}


std::ostream &AbstractAliasList::print(std::ostream &os) const {
  for(auto &alias : *this) {
    alias.second->print(os) << std::endl;
  }

  return os;
}


std::ostream &AbstractAliasList::debug(std::ostream &os) const {
  for(auto &alias : *this) {
    alias.second->debug(os) << std::endl;
  }

  return os;
}


void AbstractAliasList::add(std::shared_ptr<AbstractAliasElement> alias) {
  if(alias) {
    if(auto rename = alias->alias()) {
      elements.emplace(rename->toString(), alias);
    }
  }
}


AbstractMatchCaseElement::AbstractMatchCaseElement(std::shared_ptr<Token>                     key,
                                                   std::shared_ptr<AbstractIdentifierElement> value):
  AbstractElement(AbstractElementType::MatchCase),
  key_ptr(key),
  value_ptr(value) {
}


AbstractMatchCaseElement::~AbstractMatchCaseElement() {
  // nothing to do here
}


std::ostream &AbstractMatchCaseElement::print(std::ostream &os) const {
  os << "Case: ";
  nyx::syntax::operator<<(os, key_ptr)   << " => ";
  nyx::syntax::operator<<(os, value_ptr);

  return os;
}


std::ostream &AbstractMatchCaseElement::debug(std::ostream &os) const {
  return print(os);
}


AbstractMatchElement::AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                                           std::shared_ptr<Token>                     lower,
                                           std::shared_ptr<Token>                     upper,
                                           std::shared_ptr<Token>                     bind):
  AbstractPatternElement(AbstractElementType::Match, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractMatchCaseElement>() {

}


AbstractMatchElement::AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                                           std::shared_ptr<AbstractMatchCaseElement>  simple,
                                           std::shared_ptr<Token>                     lower,
                                           std::shared_ptr<Token>                     upper,
                                           std::shared_ptr<Token>                     bind):
  AbstractPatternElement(AbstractElementType::Match, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractMatchCaseElement>(simple),
  key(discriminant) {
}


AbstractMatchElement::AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                         const std::vector<std::shared_ptr<AbstractMatchCaseElement>> &multi,
                                           std::shared_ptr<Token>                     lower,
                                           std::shared_ptr<Token>                     upper,
                                           std::shared_ptr<Token>                     bind):
  AbstractPatternElement(AbstractElementType::Match, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractMatchCaseElement>(multi),
  key(discriminant) {
}


AbstractMatchElement::AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                                           iterator                                   first,
                                           iterator                                    end,
                                           std::shared_ptr<Token>                     lower,
                                           std::shared_ptr<Token>                     upper,
                                           std::shared_ptr<Token>                     bind):
  AbstractPatternElement(AbstractElementType::Match, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractMatchCaseElement>(first, end),
  key(discriminant) {
}


AbstractMatchElement::~AbstractMatchElement() {
  // nothing to do here
}


std::ostream &AbstractMatchElement::print(std::ostream &os) const {
  os << "Match: ";
  nyx::syntax::operator<<(os, key);
  if(bind) {
    os << " => ";
    nyx::syntax::operator<<(os, bind);
  }

  os << std::endl;

  for(auto &match : elements) {
    nyx::syntax::operator<<(os, match) << std::endl;
  }

  return os;
}


std::ostream &AbstractMatchElement::debug(std::ostream &os) const {
  return print(os);
}


void AbstractMatchElement::add(std::shared_ptr<AbstractMatchCaseElement> element) {
  elements.emplace_back(element);
}


AbstractPatternElement::AbstractPatternElement(AbstractElementType type,
                                               bool isSimple,
                                               std::shared_ptr<Token> lower,
                                               std::shared_ptr<Token> upper,
                                               std::shared_ptr<Token> binding):
  AbstractElement(type),
  simple(isSimple),
  min(lower),
  max(upper),
  bind(binding) {
}

AbstractPatternElement::~AbstractPatternElement() {
  // nothing to do here
}


AbstractSimplePatternElement::AbstractSimplePatternElement(
    std::shared_ptr<AbstractIdentifierElement> member,
    std::shared_ptr<Token> lower,
    std::shared_ptr<Token> upper,
    std::shared_ptr<Token> bind):
  AbstractPatternElement(AbstractElementType::SimplePattern, true, lower, upper, bind),
  tok(nullptr),
  ident(member) {
}


AbstractSimplePatternElement::AbstractSimplePatternElement(
    std::shared_ptr<Token> member,
    std::shared_ptr<Token> lower,
    std::shared_ptr<Token> upper,
    std::shared_ptr<Token> bind):
  AbstractPatternElement(AbstractElementType::SimplePattern, true, lower, upper, bind),
  tok(member),
  ident(nullptr) {
}


AbstractSimplePatternElement::~AbstractSimplePatternElement() {
  // nothing to do here
}


bool AbstractPatternElement::isLiteral() const {
  if(isSimple()) {
    auto &simple = *reinterpret_cast<const AbstractSimplePatternElement *>(this);

    if(simple.isToken()) {
      switch(simple.token()->lexeme()) {
        case Lexeme::BinaryLiteral:
        case Lexeme::OctalLiteral:
        case Lexeme::DecimalLiteral:
        case Lexeme::HexadecimalLiteral:
        case Lexeme::StringLiteral:
          return true;
        break;
      }
    }
  }

  return false;
}


bool AbstractPatternElement::isVariableRepeat() const {
  if(min) {
    auto &str = min->text();

    if(max && str != max->text()) {
      return true;
    }

    if(str.size() == 1) {
      return str[0] == '?' || str[0] == '+' || str[0] == '*';
    }
  }

  return false;
}


std::ostream &AbstractSimplePatternElement::print(std::ostream &os) const {
  os << "Element: ";
  if(tok) {
    os << tok->text();
  }
  else if(ident) {
     ident->print(os);
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

  if(bind) {
    os << " as " << bind->text();
  }

  return os;
}


std::ostream &AbstractSimplePatternElement::debug(std::ostream &os) const {
  os << "Element: ";
  if(tok) {
    os << tok->fileName()     << ":"
       << tok->lineNumber()   << "."
       << tok->columnNumber() << "  "
       << tok->text()         << std::endl;
  }
  else if(ident) {
    ident->debug(os);
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
  if(bind) {
    os << bind->fileName()     << ":"
       << bind->lineNumber()   << "."
       << bind->columnNumber() << "  "
       << bind->text()         << std::endl;
  }
  else {
    os << "(null)" << std::endl;
  }

  return os;
}


AbstractCompoundPatternElement::AbstractCompoundPatternElement(
    std::shared_ptr<Token> lower,
    std::shared_ptr<Token> upper,
    std::shared_ptr<Token> bind):
  AbstractPatternElement(AbstractElementType::CompoundPattern, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractPatternElement>() {
}


AbstractCompoundPatternElement::AbstractCompoundPatternElement(
    std::shared_ptr<AbstractPatternElement> member,
    std::shared_ptr<Token> lower,
    std::shared_ptr<Token> upper,
    std::shared_ptr<Token> bind):
  AbstractPatternElement(AbstractElementType::CompoundPattern, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractPatternElement>(member) {
}


AbstractCompoundPatternElement::AbstractCompoundPatternElement(
    const std::vector<std::shared_ptr<AbstractPatternElement>> &compound,
    std::shared_ptr<Token> lower,
    std::shared_ptr<Token> upper,
    std::shared_ptr<Token> bind):
  AbstractPatternElement(AbstractElementType::CompoundPattern, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractPatternElement>(compound) {
}


AbstractCompoundPatternElement::AbstractCompoundPatternElement(
    iterator start,
    iterator end,
    std::shared_ptr<Token> lower,
    std::shared_ptr<Token> upper,
    std::shared_ptr<Token> bind):
  AbstractPatternElement(AbstractElementType::CompoundPattern, false, lower, upper, bind),
  AbstractCompoundMixin<AbstractPatternElement>(start, end) {
}


AbstractCompoundPatternElement::~AbstractCompoundPatternElement() {
  // nothing to do here
}


std::ostream &AbstractCompoundPatternElement::print(std::ostream &os) const {
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

    if(!bind) {
      os << ':';
    }
  }

  if(bind) {
    os << " as " << bind->text() << ':';
  }

  os << ' ';

  auto iter = begin(), end = this->end();

  while(iter != end) {
    nyx::syntax::operator<<(os, *iter);

    if(++iter != end) {
      os << ' ';
    }
  }

  return os;
}


std::ostream &AbstractCompoundPatternElement::debug(std::ostream &os) const {
  os << "Group:" << std::endl << "Lower: ";
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

  os << "Bound: ";
  if(bind) {
    os << bind->fileName()     << ":"
       << bind->lineNumber()   << "."
       << bind->columnNumber() << "  "
       << bind->text()         << std::endl;
  }
  else {
    os << "(null)" << std::endl;
  }

  for(auto &pattern : elements) {
    pattern->debug(os);
  }

  return os;
}


void AbstractCompoundPatternElement::add(std::shared_ptr<AbstractPatternElement> element) {
  elements.emplace_back(element);
}


AbstractPatternList::AbstractPatternList(std::shared_ptr<AbstractPatternElement> member):
  AbstractElement(AbstractElementType::Pattern),
  AbstractCompoundMixin<AbstractPatternElement>(member) {
}


AbstractPatternList::AbstractPatternList():
  AbstractElement(AbstractElementType::Pattern),
  AbstractCompoundMixin<AbstractPatternElement>() {
}


AbstractPatternList::AbstractPatternList(
    const std::vector<std::shared_ptr<AbstractPatternElement>> &compound):
  AbstractElement(AbstractElementType::Pattern),
  AbstractCompoundMixin<AbstractPatternElement>(compound) {
}


AbstractPatternList::AbstractPatternList(
    iterator start, iterator end):
  AbstractElement(AbstractElementType::Pattern),
  AbstractCompoundMixin<AbstractPatternElement>(start, end) {
}


AbstractPatternList::~AbstractPatternList() {
  // nothing to do here
}


std::ostream &AbstractPatternList::print(std::ostream &os) const {
  auto iter = begin(), end = this->end();
  os << "Pattern:" << std::endl;

  while(iter != end) {
    nyx::syntax::operator<<(os, *iter);

    if(++iter != end) {
      os << " |";
    }

    os << std::endl;
  }

  return os;
}


std::ostream &AbstractPatternList::debug(std::ostream &os) const {
  auto iter = begin(), end = this->end();
  os << "Pattern:" << std::endl;

  while(iter != end) {
    (*iter)->debug(os);
  }

  return os;
}


void AbstractPatternList::add(std::shared_ptr<AbstractPatternElement> group) {
  elements.emplace_back(group);
}


//AbstractPatternList::AbstractPatternList():
//  AbstractElement(AbstractElementType::Pattern),
//  AbstractCompoundMixin<AbstractPatternAlternates>() {
//}
//
//
//AbstractPatternList::AbstractPatternList(std::shared_ptr<AbstractPatternAlternates> simple):
//  AbstractElement(AbstractElementType::Pattern),
//  AbstractCompoundMixin<AbstractPatternAlternates>(simple) {
//}
//
//
//AbstractPatternList::AbstractPatternList(
//    const std::vector<std::shared_ptr<AbstractPatternAlternates>> &compound):
//  AbstractElement(AbstractElementType::Pattern),
//  AbstractCompoundMixin<AbstractPatternAlternates>(compound) {
//}
//
//
//AbstractPatternList::AbstractPatternList(iterator start, iterator end):
//  AbstractElement(AbstractElementType::Pattern),
//  AbstractCompoundMixin<AbstractPatternAlternates>(start, end) {
//}
//
//
//AbstractPatternList::~AbstractPatternList() {
//  // nothing to do here
//}
//
//
//std::ostream &AbstractPatternList::print(std::ostream &os) const {
//  os << "Patterns:" << std::endl;
//
//  for(auto &pattern : elements) {
//    os << pattern << std::endl;
//  }
//
//  return os;
//}
//
//
//std::ostream &AbstractPatternList::debug(std::ostream &os) const {
//  os << "Patterns:" << std::endl;
//
//  for(auto &pattern : elements) {
//    pattern->debug(os);
//  }
//
//  return os;
//}
//
//
//void AbstractPatternList::add(std::shared_ptr<AbstractPatternAlternates> alt) {
//  elements.emplace_back(alt);
//}


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
  os << "Storage: ";
  nyx::syntax::operator<<(os, ident);

  if(kind) {
    os << " as ";
    nyx::syntax::operator<<(os, kind);
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


AbstractStorageList::AbstractStorageList():
  AbstractElement(AbstractElementType::StorageList),
  AbstractCompoundMixin<AbstractStorageElement>() {
}


AbstractStorageList::AbstractStorageList(std::shared_ptr<AbstractStorageElement> simple):
  AbstractElement(AbstractElementType::StorageList),
  AbstractCompoundMixin<AbstractStorageElement>(simple) {
}


AbstractStorageList::AbstractStorageList(
    const std::vector<std::shared_ptr<AbstractStorageElement>> &compound):
  AbstractElement(AbstractElementType::StorageList),
  AbstractCompoundMixin<AbstractStorageElement>(compound) {
}


AbstractStorageList::AbstractStorageList(iterator start, iterator end):
  AbstractElement(AbstractElementType::StorageList),
  AbstractCompoundMixin<AbstractStorageElement>(start, end) {
}


AbstractStorageList::~AbstractStorageList() {
  // nothing to do here
}


std::ostream &AbstractStorageList::print(std::ostream &os) const {
  os << "StorageList:" << std::endl;

  for(auto &storage : elements) {
    nyx::syntax::operator<<(os, storage) << std::endl;
  }

  return os;
}


std::ostream &AbstractStorageList::debug(std::ostream &os) const {
  os << "StorageList:" << std::endl;

  for(auto &storage : elements) {
    storage->debug(os) << std::endl;
  }

  return os;
}


void AbstractStorageList::add(std::shared_ptr<AbstractStorageElement> storage) {
  elements.emplace_back(storage);
}


AbstractSexpr::AbstractSexpr():
  tok_val(nullptr),
  sexpr_val(nullptr),
  sexpr_next(nullptr),
  ident_val(nullptr) {
}


AbstractSexpr::AbstractSexpr(std::shared_ptr<Token> token):
  tok_val(token),
  sexpr_val(nullptr),
  sexpr_next(nullptr),
  ident_val(nullptr) {
}


AbstractSexpr::AbstractSexpr(std::shared_ptr<AbstractSexpr> sexpr):
  tok_val(nullptr),
  sexpr_val(sexpr),
  sexpr_next(nullptr),
  ident_val(nullptr) {
}


AbstractSexpr::AbstractSexpr(std::shared_ptr<AbstractIdentifierElement> ident):
  tok_val(nullptr),
  sexpr_val(nullptr),
  sexpr_next(nullptr),
  ident_val(ident) {
}


std::ostream &AbstractSexpr::print(std::ostream &os) const {
  auto ptr = this;
  os << '(';

  while(ptr != nullptr) {
    if(ptr->isToken()) {
      os << ptr->token()->text();
    }
    else if(ptr->isSexpr()) {
      ptr->sexpr()->print(os);
    }
    else if(ptr->isIdentifier()) {
      ptr->identifier()->print(os);
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


std::ostream &AbstractSexpr::debug(std::ostream &os) const {
  return print(os);
}


AbstractCodeSnippet::AbstractCodeSnippet(std::shared_ptr<AbstractSexpr> sexpr):
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
  ident->print(os << "Rule: ") << std::endl;
  if(pat)      { pat->print(os)      << std::endl; }
  if(store)    { store->print(os)    << std::endl; }
  if(validate) { validate->print(os) << std::endl; }
  if(enc)      { enc->print(os)      << std::endl; }
  if(dec)      { dec->print(os)      << std::endl; }

  return os;
}


std::ostream &AbstractRuleElement::debug(std::ostream &os) const {
  print(os);
  os << std::endl;
  return os;
}


AbstractNamespaceElement::AbstractNamespaceElement():
  AbstractElement(AbstractElementType::Namespace),
  AbstractLookupMixin<AbstractRuleElement>(),
  ident(
    std::make_shared<AbstractIdentifierElement>(
      std::make_shared<Token>("", nullptr, nullptr, 0, 0, Lexeme::Identifier)
    )
  ),
  aliases(std::make_shared<AbstractAliasList>()),
  imports(std::make_shared<AbstractImportList>()) {
}


AbstractNamespaceElement::AbstractNamespaceElement(std::shared_ptr<AbstractIdentifierElement> name,
                                                   bool                                       emit):
  AbstractElement(emit ? AbstractElementType::Namespace : AbstractElementType::Module),
  AbstractLookupMixin<AbstractRuleElement>(),
  ident(name),
  aliases(std::make_shared<AbstractAliasList>()),
  imports(std::make_shared<AbstractImportList>()) {
}


AbstractNamespaceElement::~AbstractNamespaceElement() {
  // nothing to do here
}

std::ostream &AbstractNamespaceElement::print(std::ostream &os) const {
  ident->print(os << "Namespace: ") << std::endl;

  for(auto &entry : elements) {
    entry.second->print(os);
  }

  aliases->print(os);
  imports->print(os);

  return os;
}


std::ostream &AbstractNamespaceElement::debug(std::ostream &os) const {
  ident->print(os << "Namespace: ") << std::endl;

  for(auto &entry : elements) {
    entry.second->debug(os) << std::endl;
  }

  aliases->debug(os);
  imports->debug(os);

  return os;
}


void AbstractNamespaceElement::add(std::shared_ptr<AbstractImportElement> import) {
  imports->add(import);
}


void AbstractNamespaceElement::add(std::shared_ptr<AbstractAliasElement> alias) {
  aliases->add(alias);
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
  AbstractLookupMixin(),
  current(elements.emplace("", std::make_shared<AbstractNamespaceElement>()).first->second) {
}


std::ostream &AbstractSyntaxTree::print(std::ostream &os) const {
  os << "AST:" << std::endl;

  for(auto &ns : elements) {
    if(ns.first != "" || ns.second->size()) {
      ns.second->print(os) << std::endl;
    }
  }

  return os;
}


std::ostream &AbstractSyntaxTree::debug(std::ostream &os) const {
  os << "AST:" << std::endl;

  for(auto &ns : elements) {
    ns.second->debug(os) << std::endl;
  }

  return os;
}


std::shared_ptr<AbstractNamespaceElement> AbstractSyntaxTree::currentNamespace() {
  return current;
}


std::shared_ptr<AbstractNamespaceElement>
AbstractSyntaxTree::addNamespace(std::shared_ptr<AbstractIdentifierElement> ident, bool emit) {
  if(ident && ident->size() > 0) {
    current = elements.emplace(
                ident->toString(),
                std::make_shared<AbstractNamespaceElement>(ident, emit)
              ).first->second;
    return current;
  }

  return nullptr;
}


std::shared_ptr<AbstractAliasList> AbstractSyntaxTree::currentAliasList() {
  return current->aliasList();
}


void AbstractSyntaxTree::addAlias(std::shared_ptr<AbstractAliasElement> alias) {
  current->add(alias);
}


std::shared_ptr<AbstractImportList> AbstractSyntaxTree::currentImportList() {
  return current->importList();
}


void AbstractSyntaxTree::addImport(std::shared_ptr<AbstractImportElement> import) {
  current->add(import);
}

