#include "nyx/syntax/concrete.h"

#include <type_traits>


using namespace nyx::syntax;


ConcreteElement::ConcreteElement(ConcreteElementType type):
  kind(type) {
}


ConcreteElement::~ConcreteElement() {
  // nothing to do here
}


std::vector<std::shared_ptr<ConcreteElement>>::size_type ConcreteElement::size() const {
  return 0; // no children
}


namespace nyx {
  namespace syntax {

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
#define PRINT_ENUM(e) case ConcreteElementType::e: return STRINGIFY(e); break

const char *stringify(ConcreteElementType cet) {
  switch(cet) {
    PRINT_ENUM(Alias);
    PRINT_ENUM(Bound);
    PRINT_ENUM(Comment);
    PRINT_ENUM(Decode);
    PRINT_ENUM(Documentation);
    PRINT_ENUM(Encode);
    PRINT_ENUM(Identifier);
    PRINT_ENUM(Import);
    PRINT_ENUM(List);
    PRINT_ENUM(Match);
    PRINT_ENUM(Module);
    PRINT_ENUM(Namespace);
    PRINT_ENUM(Pattern);
    PRINT_ENUM(Repetition);
    PRINT_ENUM(Root);
    PRINT_ENUM(Rule);
    PRINT_ENUM(SExpr);
    PRINT_ENUM(Storage);
    PRINT_ENUM(Token);
    PRINT_ENUM(Validate);
  }

  return "INVALID";
}


std::ostream &operator<<(std::ostream &os, const ConcreteElement &ce) {
  ce.print(os);
  return os;
}


std::ostream &operator<<(std::ostream &os, const std::shared_ptr<ConcreteElement> &ce) {
  if(ce) {
    ce->print(os);
  }
  else {
    os << "(null)";
  }

  return os;
}


  }
}


ConcreteTokenElement::ConcreteTokenElement(std::shared_ptr<Token> token):
  ConcreteElement(ConcreteElementType::Token),
  value(token) {
}


ConcreteTokenElement::~ConcreteTokenElement() {
  // nothing to do here
}


bool ConcreteTokenElement::is(Lexeme lexeme) const {
  return value ? value->is(lexeme) : false;
}


std::ostream &ConcreteTokenElement::print(std::ostream &os) const {
  if(value) {
    os << value->text();
  }
  else {
    os << "(null)";
  }

  return os;
}


std::ostream &ConcreteTokenElement::debug(std::ostream &os) const {
  if(value) {
    os << value->fileName()     << ":"
       << value->lineNumber()   << "."
       << value->columnNumber() << "  "
       << value->text()         << std::endl;
  }
  else {
    os << "(null)";
  }

  return os;
}


static const std::string INVALID("<INVALID>");


int ConcreteTokenElement::line() const {
  if(value) {
    return value->lineNumber();
  }
  else {
    return -1;
  }
}


int ConcreteTokenElement::column() const {
  if(value) {
    return value->columnNumber();
  }
  else {
    return -1;
  }
}


const std::string &ConcreteTokenElement::file() const {
  if(value) {
    return value->fileName();
  }
  else {
    return INVALID;
  }
}


ConcreteCompoundElement::ConcreteCompoundElement(
    ConcreteElementType type, const std::vector<std::shared_ptr<ConcreteElement>> &subs):
  ConcreteElement(type),
  children(subs) {
}


ConcreteCompoundElement::~ConcreteCompoundElement() {
  // nothing to do here
}


std::vector<std::shared_ptr<ConcreteElement>>::size_type ConcreteCompoundElement::size() const {
  return children.size();
}


bool ConcreteCompoundElement::is(Lexeme lexeme) const {
  return size() > 0 ? children[0]->is(lexeme) : false;
}


std::ostream &ConcreteCompoundElement::print(std::ostream &os) const {
  auto iter = begin(), end = this->end();

  for(;;) {
    os << **iter;
    if(++iter == end) {
      break;
    }
    os << " ";
  }

  return os;
}


std::ostream &ConcreteCompoundElement::debug(std::ostream &os) const {
  for(auto child : children) {
    child->debug(os);
  }

  return os;
}


std::vector<std::shared_ptr<ConcreteElement>> &
ConcreteCompoundElement::getPeerChildren(ConcreteCompoundElement *peer) {
  return peer->children;
}


const std::vector<std::shared_ptr<ConcreteElement>> &
ConcreteCompoundElement::getPeerChildren(const ConcreteCompoundElement *peer) {
  return peer->children;
}


int ConcreteCompoundElement::line() const {
  if(children.size() && children[0]) {
    return children[0]->line();
  }
  else {
    return -1;
  }
}


int ConcreteCompoundElement::column() const {
  if(children.size() && children[0]) {
    return children[0]->column();
  }
  else {
    return -1;
  }
}


const std::string &ConcreteCompoundElement::file() const {
  if(children.size() && children[0]) {
    return children[0]->file();
  }
  else {
    return INVALID;
  }
}


ConcreteIdentifierElement::ConcreteIdentifierElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Identifier, elements) {
}


ConcreteIdentifierElement::~ConcreteIdentifierElement() {
  // nothing to do here
}


std::ostream &ConcreteIdentifierElement::print(std::ostream &os) const {
  for(auto iter = begin(), end = this->end(); iter != end; ++iter) {
    os << **iter;
  }

  return os;
}


ConcreteAliasElement::ConcreteAliasElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Alias, elements) {
}


ConcreteAliasElement::~ConcreteAliasElement() {
  // nothing to do here
}


std::ostream &ConcreteAliasElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteCommentElement::ConcreteCommentElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &comments):
  ConcreteCompoundElement(ConcreteElementType::Comment, comments) {
}


ConcreteCommentElement::~ConcreteCommentElement() {
  // nothing to do here
}


std::ostream &ConcreteCommentElement::print(std::ostream &os) const {
  for(auto iter = begin(), end = this->end(); iter != end; ++iter) {
    os << *iter << std::endl;
  }

  return os;
}


ConcreteCommentElement::ConcreteCommentElement(
    ConcreteElementType type,
    const std::vector<std::shared_ptr<ConcreteElement>> &comments):
  ConcreteCompoundElement(type, comments) {
}


ConcreteDocumentationElement::ConcreteDocumentationElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &docs):
  ConcreteCommentElement(ConcreteElementType::Documentation, docs) {
}


ConcreteDocumentationElement::~ConcreteDocumentationElement() {
  // nothing to do here
}


ConcreteImportElement::ConcreteImportElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Import, elements) {
}


ConcreteImportElement::~ConcreteImportElement() {
  // nothing to do here
}


std::ostream &ConcreteImportElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteNamespaceElement::ConcreteNamespaceElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Namespace, elements) {
}


ConcreteNamespaceElement::~ConcreteNamespaceElement() {
  // nothing to do here
}


std::ostream &ConcreteNamespaceElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteModuleElement::ConcreteModuleElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Module, elements) {
}


ConcreteModuleElement::~ConcreteModuleElement() {
  // nothing to do here
}


std::ostream &ConcreteModuleElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteRuleElement::ConcreteRuleElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Rule, elements) {
}


ConcreteRuleElement::~ConcreteRuleElement() {
  // nothing to do here
}


std::ostream &ConcreteRuleElement::print(std::ostream &os) const {
  auto size = this->size();
  if(size >= 4) {
    os << children[0] << " " << children[1] << std::endl;
    for(int i = 2, max = size - 1; i < max; ++i) {
      os << "  " << children[i];
    }
    os << children.back() << std::endl;
  }
  else if(size != 0) {
    for(auto &child : children) {
      os << child;
    }

    os << std::endl;
  }
  else {
    os << "(null)" << std::endl;
  }

  return os;
}


ConcretePatternElement::ConcretePatternElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Pattern, elements) {
}


ConcretePatternElement::~ConcretePatternElement() {
  // nothing to do here
}


std::ostream &ConcretePatternElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteSexprElement::ConcreteSexprElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::SExpr, elements) {
}


ConcreteSexprElement::~ConcreteSexprElement() {
  // nothing to do here
}


static auto asToken(std::shared_ptr<const ConcreteElement> ce) {
  return reinterpret_cast<const ConcreteTokenElement *>(ce.get())->token();
}


ConcreteDecodeElement::ConcreteDecodeElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Decode, elements) {
}


ConcreteDecodeElement::~ConcreteDecodeElement() {
  // nothing to do here
}


std::ostream &ConcreteDecodeElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteEncodeElement::ConcreteEncodeElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Encode, elements) {
}


ConcreteEncodeElement::~ConcreteEncodeElement() {
  // nothing to do here
}


std::ostream &ConcreteEncodeElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteStorageElement::ConcreteStorageElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Storage, elements) {
}


ConcreteStorageElement::~ConcreteStorageElement() {
  // nothing to do here
}


std::ostream &ConcreteStorageElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteValidateElement::ConcreteValidateElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Validate, elements) {
}


ConcreteValidateElement::~ConcreteValidateElement() {
  // nothing to do here
}


std::ostream &ConcreteValidateElement::print(std::ostream &os) const {
  ConcreteCompoundElement::print(os) << std::endl;
  return os;
}


ConcreteListElement::ConcreteListElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::List, elements) {
}


ConcreteListElement::~ConcreteListElement() {
  // nothing to do here
}


ConcreteMatchElement::ConcreteMatchElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Match, elements) {
}


ConcreteMatchElement::~ConcreteMatchElement() {
  // nothing to do here
}


std::ostream &ConcreteMatchElement::print(std::ostream &os) const {
  auto iter = begin(), last = end();

  while(iter != last && !(*iter)->is(Lexeme::OpenCurly)) {
    os << *iter;
    ++iter;
  }
  if(iter != last) {
    os << ' ' << *iter << std::endl;

    while(++iter != last && !(*iter)->is(Lexeme::CloseCurly)) {
      os << "    " << *iter << std::endl;
    }
    if(iter != last) {
      os << "  " << *iter;
    }
  }

  return os;
}


ConcreteBoundElement::ConcreteBoundElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Bound, elements) {
}


ConcreteBoundElement::~ConcreteBoundElement() {
  // nothing to do here
}


ConcreteRepetitionElement::ConcreteRepetitionElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Repetition, elements) {
}


ConcreteRepetitionElement::~ConcreteRepetitionElement() {
  // nothing to do here
}


std::ostream &ConcreteRepetitionElement::print(std::ostream &os) const {
  for(auto iter = begin(), end = this->end(); iter != end; ++iter) {
    os << *iter;
  }

  return os;
}


ConcreteSyntaxTree::ConcreteSyntaxTree(const std::vector<std::shared_ptr<ConcreteElement>> &children):
  ConcreteCompoundElement(ConcreteElementType::Root, children) {
}


ConcreteSyntaxTree::~ConcreteSyntaxTree() {
  // nothing to do here
}


std::ostream &ConcreteSyntaxTree::print(std::ostream &os) const {
  for(auto iter = begin(), end = this->end(); iter != end; ++iter) {
    os << *iter << std::endl;
  }

  return os;
}


