#include "nyx/syntax/concrete.h"

#include <type_traits>


using namespace nyx::syntax;


ConcreteElement::ConcreteElement(ConcreteElementType type):
  kind(type) {
}


ConcreteElement::~ConcreteElement() {
  // nothing to do here
}


bool ConcreteElement::is(Lexeme lexeme) const {
  (void) lexeme; // intentionally ignored
  return false;
}


std::vector<std::shared_ptr<ConcreteElement>>::size_type ConcreteElement::size() const {
  return 1;
}


namespace nyx {
  namespace syntax {

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


bool ConcreteTokenElement::isValid() const {
  return static_cast<bool>(value);
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


bool ConcreteCompoundElement::isValid() const {
  if(!size()) {
    return false; // empty is not valid for a compound element
  }

  bool retVal = true;

  for(auto iter = begin(), end = this->end(); retVal && iter != end; ++iter) {
    retVal = (*iter)->isValid();
  }

  return retVal;
}


std::vector<std::shared_ptr<ConcreteElement>> &
ConcreteCompoundElement::getPeerChildren(ConcreteCompoundElement *peer) {
  return peer->children;
}


const std::vector<std::shared_ptr<ConcreteElement>> &
ConcreteCompoundElement::getPeerChildren(const ConcreteCompoundElement *peer) {
  return peer->children;
}


ConcreteIdentifierElement::ConcreteIdentifierElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Identifier, elements) {
}


ConcreteIdentifierElement::~ConcreteIdentifierElement() {
  // nothing to do here
}


bool ConcreteIdentifierElement::isValid() const {
  bool retVal = (size() & 1) == 1;
  bool even = true;

  for(auto iter = begin(); retVal && iter != end(); ++iter, even = !even) {
    retVal = (*iter)->is(ConcreteElementType::Token, even ? Lexeme::Identifier : Lexeme::Dot);
  }

  return retVal;
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


bool ConcreteAliasElement::isValid() const {
  return size() == 3                                                     &&
         children[0]->is(ConcreteElementType::Token, Lexeme::Alias     ) &&
         children[1]->is(ConcreteElementType::Identifier) &&
         children[1]->isValid()                           &&
         children[2]->is(ConcreteElementType::Identifier) &&
         children[2]->isValid()                           &&
         children[2]->size() == 1;
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


bool ConcreteCommentElement::isValid() const {
  auto iter = begin();
  bool retVal = size() > 0 &&
                (*iter)->is(ConcreteElementType::Token, Lexeme::Comment)  ||
                (*iter)->is(ConcreteElementType::Token, Lexeme::DocStart);

  ++iter;
  for(auto end = this->end(); retVal && iter != end; ++iter) {
    retVal = (*iter)->is(ConcreteElementType::Token, Lexeme::Comment);
  }

  return retVal;
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


bool ConcreteDocumentationElement::isValid() const {
  bool retVal = size() >= 2                                                        &&
                children.front()->is(ConcreteElementType::Token, Lexeme::DocStart) &&
                children.back()->is( ConcreteElementType::Token, Lexeme::DocEnd  );

  for(int i = 1, max = size() - 1; retVal && i < max; ++i) {
    retVal = children[i]->is(ConcreteElementType::Token, Lexeme::Comment);
  }

  return retVal;
}


ConcreteImportElement::ConcreteImportElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Import, elements) {
}


ConcreteImportElement::~ConcreteImportElement() {
  // nothing to do here
}


bool ConcreteImportElement::isValid() const {
  return false;
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


bool ConcreteNamespaceElement::isValid() const {
  auto size = this->size();
  // minimum size of two elements, the first one must be a namespace token
  bool retVal = size >= 2 && children[0]->is(ConcreteElementType::Token, Lexeme::Namespace);

  // must be even
  if((retVal &= !(size & 1))) {
    // every odd element must be an identifier token
    for(int i = 1; retVal && i < size; i += 2) {
      retVal = children[i]->is(ConcreteElementType::Token, Lexeme::Identifier);
    }
    // every other even element must be a dot token
    for(int i = 2; retVal && i < size; i += 2) {
      retVal = children[i]->is(ConcreteElementType::Token, Lexeme::Dot);
    }
  }

  return retVal;
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


bool ConcreteModuleElement::isValid() const {
  auto size = this->size();
  // minimum size of two elements, the first one must be a module token
  bool retVal = size >= 2 && children[0]->is(ConcreteElementType::Token, Lexeme::Module);

  // must be even
  if((retVal &= !(size & 1))) {
    // every odd element must be an identifier token
    for(int i = 1; retVal && i < size; i += 2) {
      retVal = children[i]->is(ConcreteElementType::Token, Lexeme::Identifier);
    }
    // every other even element must be a dot token
    for(int i = 2; retVal && i < size; i += 2) {
      retVal = children[i]->is(ConcreteElementType::Token, Lexeme::Dot);
    }
  }

  return retVal;
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


bool ConcreteRuleElement::isValid() const {
  auto size = this->size();
  // minimum size of four elements, the first one must be an identifier token
  // second element must be an open curly token
  // the last element must be a close curly token
  bool retVal = size >= 4 &&
                children[0]->is(    ConcreteElementType::Token, Lexeme::Identifier) &&
                children[1]->is(    ConcreteElementType::Token, Lexeme::OpenCurly ) &&
                children.back()->is(ConcreteElementType::Token, Lexeme::CloseCurly);

  int decode = 0, encode = 0, pattern = 0, storage = 0, validate = 0;
  for(int i = 2, max = size - 1; retVal && i < max; ++i) {
    auto &child = children[i];
    switch(child->type()) {
      case ConcreteElementType::Decode:
        ++decode;
        retVal = child->isValid();
      break;

      case ConcreteElementType::Encode:
        ++encode;
        retVal = child->isValid();
      break;

      case ConcreteElementType::Pattern:
        ++pattern;
        retVal = child->isValid();
      break;

      case ConcreteElementType::Storage:
        ++storage;
        retVal = child->isValid();
      break;

      case ConcreteElementType::Validate:
        ++validate;
        retVal = child->isValid();
      break;

      default:
        retVal = false;
      break;
    }
  }

  return retVal && decode  <= 1 && encode  <= 1 &&
                   pattern <= 1 && storage <= 1 && validate <= 1;
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


bool ConcretePatternElement::isValid() const {
  auto size = this->size();
  // minimum size of two elements, the first one must be a module token
  bool retVal = size >= 2 && children[0]->is(Lexeme::Pattern);

  // every other element must start with an identifier, literal, or pattern
  for(int i = 1; retVal && i < size; ++i) {
    retVal = (children[i]->is(Lexeme::Identifier        ) ||
              children[i]->is(Lexeme::OctalLiteral      ) ||
              children[i]->is(Lexeme::OctalPattern      ) ||
              children[i]->is(Lexeme::BinaryLiteral     ) ||
              children[i]->is(Lexeme::BinaryPattern     ) ||
              children[i]->is(Lexeme::StringLiteral     ) ||
              children[i]->is(Lexeme::DecimalLiteral    ) ||
              children[i]->is(Lexeme::HexadecimalLiteral) ||
              children[i]->is(Lexeme::HexadecimalPattern)) &&
             children[i]->isValid();
  }

  return retVal;
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


bool ConcreteSexprElement::isValid() const {
  bool retVal = size() >= 3                                                         &&
                children[0]->is(    ConcreteElementType::Token, Lexeme::OpenParen ) &&
                children.back()->is(ConcreteElementType::Token, Lexeme::CloseParen);

  for(int i = 1, max = size() - 1; retVal && i < max; ++i) {
    if(children[i]->is(ConcreteElementType::SExpr)) {
      retVal = children[i]->isValid();
    }
    else if(children[i]->is(ConcreteElementType::Token)) {
      switch(asToken(children[i])->lexeme()) {
        case Lexeme::INVALID:
        case Lexeme::OpenCurly:
        case Lexeme::CloseCurly:
        case Lexeme::OpenParen:
        case Lexeme::CloseParen:
        case Lexeme::OpenSquare:
        case Lexeme::CloseSquare:
        case Lexeme::Alias:
        case Lexeme::Decode:
        case Lexeme::Comma:
        case Lexeme::Encode:
        case Lexeme::Import:
        case Lexeme::Match:
        case Lexeme::Namespace:
        case Lexeme::Pattern:
        case Lexeme::Storage:
        case Lexeme::Validate:
          retVal = false;
        break;
      }
    }
    else {
     retVal = false;
    }
  }

  return retVal;
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


bool ConcreteDecodeElement::isValid() const {
  return size() == 2                                                 &&
         children[0]->is(ConcreteElementType::Token, Lexeme::Decode) &&
         children[1]->is(ConcreteElementType::SExpr)                 &&
         children[1]->isValid();
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


bool ConcreteEncodeElement::isValid() const {
  return size() == 2                                                 &&
         children[0]->is(ConcreteElementType::Token, Lexeme::Encode) &&
         children[1]->is(ConcreteElementType::SExpr)                 &&
         children[1]->isValid();
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


bool ConcreteStorageElement::isValid() const {
  if(size() == 2 && children.front()->is(ConcreteElementType::Token, Lexeme::Storage)) {
    if(children.back()->is(ConcreteElementType::Token)) {
      return children.back()->is(Lexeme::Identifier);
    }
    else if(children.back()->is(ConcreteElementType::Bound)) {
      return children.back()->isValid();
    }
    else if(children.back()->is(ConcreteElementType::List)) {
      bool retVal = true;
      auto &list = getPeerChildren(
        reinterpret_cast<const ConcreteListElement *>(children.back().get())
      );

      for(int i = 1, max = size() - 1; retVal && i < max; ++i) {
        if(list[i]->is(ConcreteElementType::Token)) {
          retVal = list[i]->is(Lexeme::Identifier);
        }
        else if(list[i]->is(ConcreteElementType::Bound)) {
          retVal = list[i]->isValid();
        }
      }

      return retVal;
    }
  }

  return false;
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


bool ConcreteValidateElement::isValid() const {
  return size() == 2                                                   &&
         children[0]->is(ConcreteElementType::Token, Lexeme::Validate) &&
         children[1]->is(ConcreteElementType::SExpr)                   &&
         children[1]->isValid();
}


ConcreteListElement::ConcreteListElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::List, elements) {
}


ConcreteListElement::~ConcreteListElement() {
  // nothing to do here
}


bool ConcreteListElement::isValid() const {
  return size() >= 3                                      &&
         children.front()->is(ConcreteElementType::Token) &&
         children.back()->is( ConcreteElementType::Token) &&
         ((children.front()->is(Lexeme::OpenSquare) &&
           children.back()->is(Lexeme::CloseSquare)) ||
          (children.front()->is(Lexeme::OpenParen)  &&
           children.back()->is(Lexeme::CloseParen)));
}


ConcreteMatchElement::ConcreteMatchElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Match, elements) {
}


ConcreteMatchElement::~ConcreteMatchElement() {
  // nothing to do here
}


bool ConcreteMatchElement::isValid() const {
  return false;
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


bool ConcreteBoundElement::isValid() const {
  bool retVal = size() == 3                                                     &&
                children[1]->is(ConcreteElementType::Token, Lexeme::Bind      ) &&
                children[2]->is(ConcreteElementType::Token, Lexeme::Identifier);

  switch(children[0]->type()) {
    case ConcreteElementType::List:
    case ConcreteElementType::Repetition:
      retVal = retVal && children[0]->isValid();
    break;

    case ConcreteElementType::Token:
      switch(asToken(children[0])->lexeme()) {
        case Lexeme::Identifier:
        case Lexeme::OctalLiteral:
        case Lexeme::OctalPattern:
        case Lexeme::BinaryLiteral:
        case Lexeme::BinaryPattern:
        case Lexeme::StringLiteral:
        case Lexeme::DecimalLiteral:
        case Lexeme::HexadecimalLiteral:
        case Lexeme::HexadecimalPattern:
          retVal &= true;
        break;

        default:
          retVal = false;
        break;
      }
    break;

    default:
     retVal = false;
    break;
  }

  return retVal;
}


ConcreteRepetitionElement::ConcreteRepetitionElement(
    const std::vector<std::shared_ptr<ConcreteElement>> &elements):
  ConcreteCompoundElement(ConcreteElementType::Repetition, elements) {
}


ConcreteRepetitionElement::~ConcreteRepetitionElement() {
  // nothing to do here
}


bool ConcreteRepetitionElement::isValid() const {
  auto size = this->size();
  bool retVal = (size == 4 || size == 6)                                            &&
                children[1]->is(ConcreteElementType::Token, Lexeme::OpenCurly     ) &&
                children.back()->is(ConcreteElementType::Token, Lexeme::CloseCurly);

  if(retVal && children[2]->is(ConcreteElementType::Token)) {
    retVal = children[2]->is(Lexeme::DecimalLiteral)       ||
             children[2]->is(Lexeme::HexadecimalLiteral)   ||
             children[2]->is(Lexeme::OctalLiteral)         ||
             children[2]->is(Lexeme::BinaryLiteral)        ||
             (size == 4 && (children[2]->is(Lexeme::Times) ||
                            children[2]->is(Lexeme::Plus)  ||
                            children[2]->is(Lexeme::Query)));
    if(retVal && size == 6) {
      retVal = children[3]->is(ConcreteElementType::Token, Lexeme::Comma) &&
               children[2]->is(ConcreteElementType::Token)    &&
               (children[4]->is(Lexeme::DecimalLiteral)       ||
                children[4]->is(Lexeme::HexadecimalLiteral)   ||
                children[4]->is(Lexeme::OctalLiteral)         ||
                children[4]->is(Lexeme::BinaryLiteral));
    }
  }

  return retVal;
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


