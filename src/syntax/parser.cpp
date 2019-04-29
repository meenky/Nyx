#include "nyx/syntax/parser.h"


using namespace nyx::syntax;


typedef std::vector<std::shared_ptr<Token>>::iterator token_iterator;
typedef std::vector<std::shared_ptr<ConcreteElement>> concrete_vector;


template<typename CONCRETE>
static CONCRETE &as(concrete_vector::iterator iter) {
  return *std::dynamic_pointer_cast<CONCRETE>(*iter);
}


template<typename CONCRETE>
static CONCRETE &as(std::shared_ptr<ConcreteElement> ptr) {
  return *std::dynamic_pointer_cast<CONCRETE>(ptr);
}


static std::shared_ptr<ConcreteTokenElement> toToken(token_iterator iter) {
  return std::make_shared<ConcreteTokenElement>(*iter);
}


static concrete_vector extract(token_iterator start, token_iterator end) {
  concrete_vector retVal;

  retVal.reserve(end - start);

  while(start != end) {
    retVal.emplace_back(toToken(start));
    ++start;
  }

  return retVal;
}


static std::shared_ptr<ConcreteIdentifierElement> toIdentifier(token_iterator iter) {
  return std::make_shared<ConcreteIdentifierElement>(extract(iter, iter + 1));
}


static concrete_vector extractNoEol(token_iterator start, token_iterator end) {
  concrete_vector retVal;

  retVal.reserve(end - start);

  while(start != end) {
    if((*start)->lexeme() != Lexeme::EndOfLine) {
      retVal.emplace_back(toToken(start));
    }
    ++start;
  }

  return retVal;
}


static void unexpectedToken(std::shared_ptr<Token> token) {
  std::cerr << token->fullLine() << std::endl;
  if(token->columnNumber() > 0) {
    auto old = std::cerr.width();
    std::cerr.width(token->columnNumber());
    std::cerr << ' ';
    std::cerr.width(old);
  }
  std::cerr << '^' << std::endl;
  std::cerr << "Unexpected Token '" << token->text() << "' at " <<
               token->fileName() << ':' << token->lineNumber() << std::endl;
}


static void illegalComplexIdentifier(ConcreteIdentifierElement &ident) {
  auto token = as<ConcreteTokenElement>(ident[0]).token();

  std::cerr << token->fullLine() << std::endl;
  if(token->columnNumber() > 0) {
    auto old = std::cerr.width();
    std::cerr.width(token->columnNumber());
    std::cerr << ' ';
    std::cerr.width(old);
  }
  std::cerr << '^' << std::endl;
  std::cerr << "Illegal compound identifier '" << ident << "' at " <<
               token->fileName() << ':' << token->lineNumber() << std::endl;
}


enum class AliasParseState {
  Error = -1,
  Ready,
  Keyword,
  First,
  Second
};


static int parseAlias(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  concrete_vector children;
  AliasParseState state = AliasParseState::Ready;


  children.reserve(3);

  while(state != AliasParseState::Error && iter != last) {
    switch(state) {
      case AliasParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Alias) {
          children.emplace_back(toToken(iter));
          state = AliasParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = AliasParseState::Error;
        }
      break;

      case AliasParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          children.emplace_back(new ConcreteIdentifierElement(extract(iter, iter + 1)));
          state = AliasParseState::First;
        }
        else {
          unexpectedToken(*iter);
          state = AliasParseState::Error;
        }
      break;

      case AliasParseState::First:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          children.emplace_back(new ConcreteIdentifierElement(extract(iter, iter + 1)));
          state = AliasParseState::Second;
        }
        else {
          unexpectedToken(*iter);
          state = AliasParseState::Error;
        }
      break;

      case AliasParseState::Second:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteAliasElement>(children));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = AliasParseState::Error;
        }
      break;

      default:
        unexpectedToken(*iter);
        state = AliasParseState::Error;
      break;
    }

    ++iter;
  }

  return -1;
}


enum class CommentParseState {
  Error = -1,
  Ready,
  Comment,
  Complete
};


static int parseComment(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  CommentParseState state = CommentParseState::Ready;

  while(state != CommentParseState::Error && iter != last) {
    switch(state) {
      case CommentParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Comment) {
          state = CommentParseState::Comment;
        }
        else {
          unexpectedToken(*iter);
          state = CommentParseState::Error;
        }
      break;

      case CommentParseState::Comment:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          state = CommentParseState::Complete;
        }
        else {
          unexpectedToken(*iter);
          state = CommentParseState::Error;
        }
      break;

      case CommentParseState::Complete:
        if((*iter)->lexeme() == Lexeme::Comment) {
          state = CommentParseState::Comment;
        }
        else {
          roots.emplace_back(std::make_shared<ConcreteCommentElement>(extractNoEol(start, iter)));
          start = --iter;
          return 0;
        }
      break;

      default:
        unexpectedToken(*iter);
        state = CommentParseState::Error;
      break;
    }

    ++iter;
  }

  if(state == CommentParseState::Complete) {
    roots.emplace_back(std::make_shared<ConcreteCommentElement>(extractNoEol(start, iter)));
    start = iter;
    return 0;
  }

  return -1;
}


enum class DocParseState {
  Error = -1,
  Ready,
  DocStart,
  Between,
  Comment
};


static int parseDocumentation(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  DocParseState state = DocParseState::Ready;

  while(state != DocParseState::Error && iter != last) {
    switch(state) {
      case DocParseState::Ready:
        if((*iter)->lexeme() == Lexeme::DocStart) {
          state = DocParseState::DocStart;
        }
        else {
          unexpectedToken(*iter);
          state = DocParseState::Error;
        }
      break;

      case DocParseState::Comment:
      case DocParseState::DocStart:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          state = DocParseState::Between;
        }
        else {
          unexpectedToken(*iter);
          state = DocParseState::Error;
        }
      break;

      case DocParseState::Between:
        if((*iter)->lexeme() == Lexeme::Comment) {
          state = DocParseState::Comment;
        }
        else if((*iter)->lexeme() == Lexeme::DocEnd) {
          roots.emplace_back(
            std::make_shared<ConcreteDocumentationElement>(extractNoEol(start, ++iter))
          );
          start = iter;
          return 0;
        }
      break;

      default:
        unexpectedToken(*iter);
        state = DocParseState::Error;
      break;
    }

    ++iter;
  }

  if(state == DocParseState::Between) {
    roots.emplace_back(std::make_shared<ConcreteDocumentationElement>(extractNoEol(start, iter)));
    start = iter;
    return 0;
  }

  return -1;
}


enum class IdentifierParseState {
  Error = -1,
  Ready,
  Identifier,
  Dot
};


static std::shared_ptr<ConcreteIdentifierElement>
parseIdentifier(token_iterator &start, token_iterator last) {
  token_iterator iter = start;
  IdentifierParseState state = IdentifierParseState::Ready;

  while(state != IdentifierParseState::Error && iter != last) {
    switch(state) {
      case IdentifierParseState::Dot:
      case IdentifierParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = IdentifierParseState::Identifier;
        }
        else {
          unexpectedToken(*iter);
          state = IdentifierParseState::Error;
        }
      break;

      case IdentifierParseState::Identifier:
        if((*iter)->lexeme() == Lexeme::Dot) {
          state = IdentifierParseState::Dot;
        }
        else {
          auto retVal = std::make_shared<ConcreteIdentifierElement>(extract(start, iter));
          start = iter - 1;
          return retVal;
        }
      break;
    }

    ++iter;
  }

  return nullptr;
}


enum class ImportParseState {
  Error = -1,
  Ready,
  Keyword,
  Identifier,
  Namespace,
  From,
  As,
  Alias
};


static int parseImport(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  concrete_vector children;
  ImportParseState state = ImportParseState::Ready;

  children.reserve(6);

  while(state != ImportParseState::Error && iter != last) {
    switch(state) {
      case ImportParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Import) {
          children.emplace_back(toToken(iter));
          state = ImportParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          if(auto ident = parseIdentifier(iter, last)) {
            children.emplace_back(ident);
            state = ImportParseState::Identifier;
          }
          else {
            state = ImportParseState::Error;
           }
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Identifier:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteImportElement>(children));
          start = iter;
          return 0;
        }
        else if((*iter)->text() == "from") {
          children.emplace_back(toToken(iter));
          state = ImportParseState::From;
        }
        else if((*iter)->text() == "as") {
          children.emplace_back(toToken(iter));
          state = ImportParseState::As;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::From:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          if(auto ident = parseIdentifier(iter, last)) {
            children.emplace_back(ident);
            state = ImportParseState::Namespace;
          }
          else {
            state = ImportParseState::Error;
           }
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Namespace:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteImportElement>(children));
          start = iter;
          return 0;
        }
        else if((*iter)->text() == "as") {
          children.emplace_back(toToken(iter));
          state = ImportParseState::As;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::As:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          if(auto ident = parseIdentifier(iter, last)) {
            children.emplace_back(ident);
            state = ImportParseState::Alias;
          }
          else {
            state = ImportParseState::Error;
           }
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Alias:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteImportElement>(children));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;
    }

    ++iter;
  }

  return -1;
}


enum class ModuleParseState {
  Error = -1,
  Ready,
  Keyword,
  Identifier
};


static int parseModule(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  concrete_vector children;
  ModuleParseState state = ModuleParseState::Ready;

  children.reserve(2);

  while(state != ModuleParseState::Error && iter != last) {
    switch(state) {
      case ModuleParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Module) {
          children.emplace_back(toToken(iter));
          state = ModuleParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = ModuleParseState::Error;
        }
      break;

      case ModuleParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          if(auto ident = parseIdentifier(iter, last)) {
            children.emplace_back(ident);
            state = ModuleParseState::Identifier;
          }
          else {
            state = ModuleParseState::Error;
           }
        }
        else {
          unexpectedToken(*iter);
          state = ModuleParseState::Error;
        }
      break;

      case ModuleParseState::Identifier:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteModuleElement>(children));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = ModuleParseState::Error;
        }
      break;
    }

    ++iter;
  }

  return -1;
}


enum class NamespaceParseState {
  Error = -1,
  Ready,
  Keyword,
  Identifier
};


static int parseNamespace(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  concrete_vector children;
  NamespaceParseState state = NamespaceParseState::Ready;

  children.reserve(2);

  while(state != NamespaceParseState::Error && iter != last) {
    switch(state) {
      case NamespaceParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Namespace) {
          children.emplace_back(toToken(iter));
          state = NamespaceParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = NamespaceParseState::Error;
        }
      break;

      case NamespaceParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          if(auto ident = parseIdentifier(iter, last)) {
            children.emplace_back(ident);
            state = NamespaceParseState::Identifier;
          }
          else {
            state = NamespaceParseState::Error;
           }
        }
        else {
          unexpectedToken(*iter);
          state = NamespaceParseState::Error;
        }
      break;

      case NamespaceParseState::Identifier:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteNamespaceElement>(children));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = NamespaceParseState::Error;
        }
      break;
    }

    ++iter;
  }

  return -1;
}


static std::shared_ptr<ConcreteSexprElement>
parseSExpression(token_iterator &start, token_iterator last) {
  token_iterator  iter = start;
  concrete_vector parts;

  while(iter != last && (*iter)->lexeme() == Lexeme::EndOfLine) {
    ++iter; // ignore line endings
  }

  if(iter == last || (*iter)->lexeme() != Lexeme::OpenParen) {
    return nullptr;
  }

  parts.emplace_back(toToken(iter));
  while(++iter != last) {
    switch((*iter)->lexeme()) {
      case Lexeme::EndOfLine:
        // do nothing
      break;

      case Lexeme::CloseParen:
        parts.emplace_back(toToken(iter));
        start = iter;
        return std::make_shared<ConcreteSexprElement>(parts);
      break;

      case Lexeme::OpenParen:
        if(auto child = parseSExpression(iter, last)) {
          parts.emplace_back(child);
          continue;
        }
        else {
          return nullptr;
        }
      break;

      case Lexeme::Assignment:
      case Lexeme::BitwiseAnd:
      case Lexeme::BitwiseNot:
      case Lexeme::BitwiseOr:
      case Lexeme::BitwiseXor:
      case Lexeme::CloseAngle:
      case Lexeme::Division:
      case Lexeme::Dot:
      case Lexeme::LogicalNot:
      case Lexeme::Minus:
      case Lexeme::Modulo:
      case Lexeme::OpenAngle:
      case Lexeme::Plus:
      case Lexeme::Times:
      case Lexeme::AndAssignment:
      case Lexeme::BinaryLiteral:
      case Lexeme::DecimalLiteral:
      case Lexeme::DivAssignment:
      case Lexeme::Equality:
      case Lexeme::FloatLiteral:
      case Lexeme::GreaterThanOrEqual:
      case Lexeme::HexadecimalLiteral:
      case Lexeme::Inequality:
      case Lexeme::LeftShift:
      case Lexeme::LessThanOrEqual:
      case Lexeme::LogicalAnd:
      case Lexeme::LogicalOr:
      case Lexeme::LogicalXor:
      case Lexeme::MinusAssignment:
      case Lexeme::ModuloAssignment:
      case Lexeme::OctalLiteral:
      case Lexeme::OrAssignment:
      case Lexeme::PlusAssignment:
      case Lexeme::RightShift:
      case Lexeme::StringLiteral:
      case Lexeme::TimesAssignment:
      case Lexeme::XorAssignment:
        parts.emplace_back(toToken(iter));
      break;

      case Lexeme::Identifier:
        if(auto ident = parseIdentifier(iter, last)) {
          parts.emplace_back(ident);
        }
        else {
          return nullptr;
         }
      break;

      default:
        return nullptr;
      break;
    }
  }

  return nullptr;
}


static int parseRuleDecode(token_iterator &start, token_iterator last, concrete_vector &rule) {
  token_iterator iter = start + 1;
  auto           sexpr = parseSExpression(iter, last);

  if(sexpr) {
    auto children = extract(start, start + 1);
    children.emplace_back(sexpr);
    rule.emplace_back(std::make_shared<ConcreteDecodeElement>(children));
    start = iter;
    return 0;
  }

  return -1;
}


static int parseRuleEncode(token_iterator &start, token_iterator last, concrete_vector &rule) {
  token_iterator iter = start + 1;
  auto           sexpr = parseSExpression(iter, last);

  if(sexpr) {
    auto children = extract(start, start + 1);
    children.emplace_back(sexpr);
    rule.emplace_back(std::make_shared<ConcreteEncodeElement>(children));
    start = iter;
    return 0;
  }

  return -1;
}


static std::shared_ptr<ConcreteBoundElement> make_bound(
    concrete_vector::iterator start, token_iterator token) {
  concrete_vector parts(start, start + 2);
  parts.emplace_back(toToken(token));
  return std::make_shared<ConcreteBoundElement>(parts);
}


static concrete_vector &compactBinding(concrete_vector &parts, token_iterator token) {
  auto binding = make_bound(parts.end() - 2, token);
  parts.resize(parts.size() - 2);
  parts.emplace_back(binding);
  return parts;
}


enum class RepetitionParseState {
  Error = -1,
  Ready,
  HasLowerBound,
  Comma,
  HasBothBounds,
};


static std::shared_ptr<ConcreteRepetitionElement>
parseRepetition(token_iterator &start, token_iterator last, std::shared_ptr<ConcreteElement> elem) {
  concrete_vector   parts;
  token_iterator    iter = start;
  RepetitionParseState state = RepetitionParseState::Ready;

  parts.push_back(elem);
  parts.emplace_back(toToken(start));

  while(state != RepetitionParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // always ignore line ends
    }

    switch(state) {
      case RepetitionParseState::Ready:
        switch((*iter)->lexeme()) {
          case Lexeme::Identifier:
            parts.emplace_back(toIdentifier(iter));
            state = RepetitionParseState::HasLowerBound;
          break;

          case Lexeme::BinaryLiteral:
          case Lexeme::OctalLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
            parts.emplace_back(toToken(iter));
            state = RepetitionParseState::HasLowerBound;
          break;

          case Lexeme::Plus:
          case Lexeme::Query:
          case Lexeme::Times:
            parts.emplace_back(toToken(iter));
            state = RepetitionParseState::HasBothBounds;
          break;

          default:
            unexpectedToken(*iter);
            state = RepetitionParseState::Error;
          break;
        }
      break;

      case RepetitionParseState::HasLowerBound:
        if((*iter)->lexeme() == Lexeme::Comma) {
          parts.emplace_back(toToken(iter));
          state = RepetitionParseState::Comma;
        }
        else if((*iter)->lexeme() == Lexeme::CloseCurly) {
          parts.emplace_back(toToken(iter));
          start = iter;
          return std::make_shared<ConcreteRepetitionElement>(parts);
        }
        else {
          unexpectedToken(*iter);
          state = RepetitionParseState::Error;
        }
      break;

      case RepetitionParseState::Comma:
        switch((*iter)->lexeme()) {
          case Lexeme::Identifier:
            parts.emplace_back(toIdentifier(iter));
            state = RepetitionParseState::HasBothBounds;
          break;

          case Lexeme::Plus:
          case Lexeme::Times:
          case Lexeme::BinaryLiteral:
          case Lexeme::OctalLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
            parts.emplace_back(toToken(iter));
            state = RepetitionParseState::HasBothBounds;
          break;

          default:
            unexpectedToken(*iter);
            state = RepetitionParseState::Error;
          break;
        }
      break;

      case RepetitionParseState::HasBothBounds:
        if((*iter)->lexeme() == Lexeme::CloseCurly) {
          parts.emplace_back(toToken(iter));
          start = iter;
          return std::make_shared<ConcreteRepetitionElement>(parts);
        }
        else {
          unexpectedToken(*iter);
          state = RepetitionParseState::Error;
        }
      break;
    }
  }

  return nullptr;
}


enum class MatchParseState {
  Error = -1,
  Ready,
  InHead,
  HasDiscriminant,
  HasHead,
  InBody,
  HasElement,
  Binding,
  HasBoundElement,
  HasRepeatingElement
};


static std::shared_ptr<ConcreteMatchElement>
parseRulePatternMatch(token_iterator &start, token_iterator last) {
  concrete_vector parts;
  token_iterator  iter = start;
  MatchParseState state = MatchParseState::Ready;

  parts.emplace_back(toToken(start));

  while(state != MatchParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // always ignore line ends
    }

    switch(state) {
      case MatchParseState::Ready:
        if((*iter)->lexeme() == Lexeme::OpenParen) {
          parts.emplace_back(toToken(iter));
          state = MatchParseState::InHead;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::InHead:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          if(auto ident = parseIdentifier(iter, last)) {
            parts.emplace_back(ident);
            state = MatchParseState::HasDiscriminant;
          }
          else {
            state = MatchParseState::Error;
           }
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::HasDiscriminant:
        if((*iter)->lexeme() == Lexeme::CloseParen) {
          parts.emplace_back(toToken(iter));
          state = MatchParseState::HasHead;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::HasHead:
        if((*iter)->lexeme() == Lexeme::OpenCurly) {
          parts.emplace_back(toToken(iter));
          state = MatchParseState::InBody;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::InBody:
        switch((*iter)->lexeme()) {
          case Lexeme::BinaryLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::OctalLiteral:
            parts.emplace_back(toToken(iter));
            state = MatchParseState::HasElement;
          break;

          default:
            unexpectedToken(*iter);
            state = MatchParseState::Error;
          break;
        }
      break;

      case MatchParseState::HasElement:
        if((*iter)->lexeme() == Lexeme::Bind) {
          parts.emplace_back(toToken(iter));
          state = MatchParseState::Binding;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::Binding:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          compactBinding(parts, iter);
          state = MatchParseState::HasBoundElement;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::HasBoundElement:
        switch((*iter)->lexeme()) {
          case Lexeme::BinaryLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::OctalLiteral:
            parts.emplace_back(toToken(iter));
            state = MatchParseState::HasElement;
          break;

          case Lexeme::CloseCurly:
            parts.emplace_back(toToken(iter));
            start = iter;
            return std::make_shared<ConcreteMatchElement>(parts);
          break;

          case Lexeme::OpenCurly:
            if(auto rep = parseRepetition(iter, last, parts.back())) {
              parts.back() = rep;
              state = MatchParseState::HasRepeatingElement;
            }
            else {
              state = MatchParseState::Error;
            }
          break;

          default:
            unexpectedToken(*iter);
            state = MatchParseState::Error;
          break;
        }
      break;

      case MatchParseState::HasRepeatingElement:
        switch((*iter)->lexeme()) {
          case Lexeme::BinaryLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::OctalLiteral:
            parts.emplace_back(toToken(iter));
            state = MatchParseState::HasElement;
          break;

          case Lexeme::CloseCurly:
            parts.emplace_back(toToken(iter));
            start = iter;
            return std::make_shared<ConcreteMatchElement>(parts);
          break;

          default:
            unexpectedToken(*iter);
            state = MatchParseState::Error;
          break;
        }
      break;
    }
  }

  return nullptr;
}


enum class PatternParseState {
  Error = -1,
  Ready,
  HasElement,
  HasRepeatingElement,
  Binding,
  HasBoundElement
};


static std::shared_ptr<ConcreteElement>
parseRulePatternElement(token_iterator &start, token_iterator last);


static std::shared_ptr<ConcreteListElement>
parseRulePatternList(token_iterator &start, token_iterator last) {
  concrete_vector   parts;
  token_iterator    iter = start;
  PatternParseState state = PatternParseState::Ready;

  parts.emplace_back(toToken(start));

  while(state != PatternParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // always ignore line ends
    }

    switch(state) {
      case PatternParseState::Ready:
        switch((*iter)->lexeme()) {
          case Lexeme::Match:
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
            if(auto elem = parseRulePatternElement(iter, last)) {
              parts.emplace_back(elem);
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          case Lexeme::OpenParen:
            if(auto list = parseRulePatternList(iter, last)) {
              parts.emplace_back(list);
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          default:
            unexpectedToken(*iter);
            state = PatternParseState::Error;
          break;
        }
      break;

      case PatternParseState::HasElement:
        switch((*iter)->lexeme()) {
          case Lexeme::Match:
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
            if(auto elem = parseRulePatternElement(iter, last)) {
              parts.emplace_back(elem);
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          case Lexeme::OpenParen:
            if(auto list = parseRulePatternList(iter, last)) {
              parts.emplace_back(list);
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          case Lexeme::CloseParen:
            parts.emplace_back(toToken(iter));
            start = iter;
            return std::make_shared<ConcreteListElement>(parts);
          break;

          default:
            unexpectedToken(*iter);
            state = PatternParseState::Error;
          break;
        }
      break;
    }
  }
  return nullptr;
}


static std::shared_ptr<ConcreteElement>
parseRulePatternElement(token_iterator &start, token_iterator last) {
  token_iterator                   iter = start;
  std::shared_ptr<ConcreteElement> base, op;
  PatternParseState                state = PatternParseState::Ready;

  while(state != PatternParseState::Error && iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      ++iter;
      continue; // always ignore line ends
    }

    switch(state) {
      case PatternParseState::Ready:
        switch((*iter)->lexeme()) {
          case Lexeme::Identifier:
            if((base = parseIdentifier(iter, last))) {
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
             }
          break;

          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
          case Lexeme::StringLiteral:
            base = toToken(iter);
            state = PatternParseState::HasElement;
          break;

          case Lexeme::OpenParen:
            if((base = parseRulePatternList(iter, last))) {
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          case Lexeme::Match:
            if((base = parseRulePatternMatch(iter, last))) {
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          default:
            unexpectedToken(*iter);
            state = PatternParseState::Error;
          break;
        }
      break;

      case PatternParseState::HasElement:
        switch((*iter)->lexeme()) {
          case Lexeme::Match:
          case Lexeme::BitwiseOr:
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
          case Lexeme::StringLiteral:
          case Lexeme::OpenParen:
          case Lexeme::CloseParen:
          case Lexeme::Decode:
          case Lexeme::Encode:
          case Lexeme::Storage:
          case Lexeme::Validate:
          case Lexeme::CloseCurly:
            start = --iter;
            return base;
          break;

          case Lexeme::Bind:
            op = toToken(iter);
            state = PatternParseState::Binding;
          break;

          case Lexeme::OpenCurly:
            if((base = parseRepetition(iter, last, base))) {
              state = PatternParseState::HasRepeatingElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          default:
            unexpectedToken(*iter);
            state = PatternParseState::Error;
          break;
        }
      break;

      case PatternParseState::Binding:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          start = iter;
          return std::make_shared<ConcreteBoundElement>(
            concrete_vector{base, op, toIdentifier(iter)}
          );
        }
        else {
          unexpectedToken(*iter);
          state = PatternParseState::Error;
        }
      break;

      case PatternParseState::HasRepeatingElement:
        switch((*iter)->lexeme()) {
          case Lexeme::Match:
          case Lexeme::BitwiseOr:
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
          case Lexeme::StringLiteral:
          case Lexeme::OpenParen:
          case Lexeme::CloseParen:
          case Lexeme::Decode:
          case Lexeme::Encode:
          case Lexeme::Storage:
          case Lexeme::Validate:
          case Lexeme::CloseCurly:
            start = --iter;
            return base;
          break;

          case Lexeme::Bind:
            op = toToken(iter);
            state = PatternParseState::Binding;
          break;

          default:
            unexpectedToken(*iter);
            state = PatternParseState::Error;
          break;
        }
      break;
    }

    ++iter;
  }

  return nullptr;
}


static int parseRulePattern(token_iterator &start, token_iterator last, concrete_vector &rule) {
  concrete_vector   parts;
  token_iterator    iter = start;
  PatternParseState state = PatternParseState::Ready;

  parts.emplace_back(toToken(start));

  while(state != PatternParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // always ignore line ends
    }

    switch(state) {
      case PatternParseState::Ready:
        switch((*iter)->lexeme()) {
          case Lexeme::OpenParen:
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
          case Lexeme::StringLiteral:
            if(auto elem = parseRulePatternElement(iter, last)) {
              parts.emplace_back(elem);
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          default:
            unexpectedToken(*iter);
            state = PatternParseState::Error;
          break;
        }
      break;

      case PatternParseState::HasElement:
        switch((*iter)->lexeme()) {
          case Lexeme::Match:
          case Lexeme::OpenParen:
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
          case Lexeme::StringLiteral:
            if(auto elem = parseRulePatternElement(iter, last)) {
              parts.emplace_back(elem);
              state = PatternParseState::HasElement;
            }
            else {
              state = PatternParseState::Error;
            }
          break;

          case Lexeme::BitwiseOr:
            parts.emplace_back(toToken(iter));
            state = PatternParseState::Ready;
          break;

          case Lexeme::Decode:
          case Lexeme::Encode:
          case Lexeme::Storage:
          case Lexeme::Validate:
          case Lexeme::CloseCurly:
            rule.emplace_back(std::make_shared<ConcretePatternElement>(parts));
            start = --iter;
            return 0;
          break;

          default:
            unexpectedToken(*iter);
            state = PatternParseState::Error;
          break;
        }
      break;
    }
  }

  return -1;
}


enum class StorageParseState {
  Error = -1,
  Ready,
  SingleIdentifier,
  Binding,
  ListReady,
  InList,
  ListIdentifier,
  ListBinding,
};


static int parseRuleStorage(token_iterator &start, token_iterator last, concrete_vector &rule) {
  concrete_vector   parts;
  token_iterator    iter = start;
  StorageParseState state = StorageParseState::Ready;

  parts.emplace_back(toToken(start));

  while(state != StorageParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // line endings mean nothing
    }

    switch(state) {
      case StorageParseState::Ready:
        if((*iter)->lexeme() == Lexeme::OpenSquare) {
          parts.emplace_back(toToken(iter));
          state = StorageParseState::ListReady;
        }
        else if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(toIdentifier(iter));
          state = StorageParseState::SingleIdentifier;
        }
        else {
          unexpectedToken(*iter);
          state = StorageParseState::Error;
        }
      break;

      case StorageParseState::SingleIdentifier:
        switch((*iter)->lexeme()) {
          case Lexeme::Bind:
            parts.emplace_back(toToken(iter));
            state = StorageParseState::Binding;
          break;

          case Lexeme::Decode:
          case Lexeme::Encode:
          case Lexeme::Pattern:
          case Lexeme::Validate:
          case Lexeme::CloseCurly:
            rule.emplace_back(std::make_shared<ConcreteStorageElement>(parts));
            start = --iter;
            return 0;
          break;

          default:
            unexpectedToken(*iter);
            state = StorageParseState::Error;
          break;
        }
      break;

      case StorageParseState::Binding:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          rule.emplace_back(std::make_shared<ConcreteStorageElement>(compactBinding(parts, iter)));
          start = ++iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = StorageParseState::Error;
        }
      break;

      case StorageParseState::ListReady:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(toIdentifier(iter));
          state = StorageParseState::ListIdentifier;
        }
        else {
          unexpectedToken(*iter);
          state = StorageParseState::Error;
        }
      break;

      case StorageParseState::ListIdentifier:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(toIdentifier(iter));
          state = StorageParseState::ListIdentifier;
        }
        else if((*iter)->lexeme() == Lexeme::Bind) {
          parts.emplace_back(toToken(iter));
          state = StorageParseState::ListBinding;
        }
        else if((*iter)->lexeme() == Lexeme::CloseSquare) {
          parts.emplace_back(toToken(iter));
          rule.emplace_back(std::make_shared<ConcreteStorageElement>(parts));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = StorageParseState::Error;
        }
      break;

      case StorageParseState::ListBinding:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          compactBinding(parts, iter);
          state = StorageParseState::InList;
        }
        else {
          unexpectedToken(*iter);
          state = StorageParseState::Error;
        }
      break;

      case StorageParseState::InList:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(toIdentifier(iter));
          state = StorageParseState::ListIdentifier;
        }
        else if((*iter)->lexeme() == Lexeme::CloseSquare) {
          parts.emplace_back(toToken(iter));
          rule.emplace_back(std::make_shared<ConcreteStorageElement>(parts));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = StorageParseState::Error;
        }
      break;
    }
  }

  return -1;
}


static int parseRuleValidate(token_iterator &start, token_iterator last, concrete_vector &rule) {
  token_iterator iter = start + 1;
  auto           sexpr = parseSExpression(iter, last);

  if(sexpr) {
    auto children = extract(start, start + 1);
    children.emplace_back(sexpr);
    rule.emplace_back(std::make_shared<ConcreteValidateElement>(children));
    start = iter;
    return 0;
  }

  return -1;
}


enum class RuleParseState {
  Error = -1,
  Ready,
  InBody,
};


static int parseRule(token_iterator &start, token_iterator last, concrete_vector &roots) {
  concrete_vector parts;
  token_iterator  iter = start;
  RuleParseState  state = RuleParseState::Ready;

  parts.emplace_back(toIdentifier(start));

  while(state != RuleParseState::Error && ++iter != last) {
    switch(state) {
      case RuleParseState::Ready:
        if((*iter)->lexeme() == Lexeme::OpenCurly) {
          parts.emplace_back(toToken(iter));
          state = RuleParseState::InBody;
        }
        else if((*iter)->lexeme() != Lexeme::EndOfLine) {
          unexpectedToken(*iter);
          state = RuleParseState::Error;
        }
      break;

      case RuleParseState::InBody:
        switch((*iter)->lexeme()) {
          case Lexeme::EndOfLine:
            // ignore this
          break;

          case Lexeme::CloseCurly:
            parts.emplace_back(toToken(iter));
            roots.emplace_back(std::make_shared<ConcreteRuleElement>(parts));
            start = ++iter;
            return 0;
          break;

          case Lexeme::Decode:
            if(parseRuleDecode(iter, last, parts)) {
              state = RuleParseState::Error;
            }
          break;

          case Lexeme::Encode:
            if(parseRuleEncode(iter, last, parts)) {
              state = RuleParseState::Error;
            }
          break;

          case Lexeme::Pattern:
            if(parseRulePattern(iter, last, parts)) {
              state = RuleParseState::Error;
            }
          break;

          case Lexeme::Storage:
            if(parseRuleStorage(iter, last, parts)) {
              state = RuleParseState::Error;
            }
          break;

          case Lexeme::Validate:
            if(parseRuleValidate(iter, last, parts)) {
              state = RuleParseState::Error;
            }
          break;

          default:
            state = RuleParseState::Error;
          break;
        }
      break;
    }
  }

  return -1;
}


std::unique_ptr<ConcreteSyntaxTree> Parser::concreteParse(Tokenizer &tokenizer) {
  std::vector<std::shared_ptr<Token>> tokens;
  std::vector<std::shared_ptr<ConcreteElement>> roots;

  tokens.reserve(8192);

  // read in all the tokens
  while(auto token = tokenizer.next()) {
    tokens.push_back(token);
  }

  auto iter = tokens.begin(), end = tokens.end();
  bool errorFree = true;
  while(errorFree && iter != end) {
    switch((*iter)->lexeme()) {
      case Lexeme::EndOfLine:
        ++iter; // just skip this token
      break;

      case Lexeme::Alias:
        errorFree = !parseAlias(iter, end, roots);
      break;

      case Lexeme::Comment:
        errorFree = !parseComment(iter, end, roots);
      break;

      case Lexeme::DocStart:
      case Lexeme::DocEnd:
        errorFree = !parseDocumentation(iter, end, roots);
      break;

      case Lexeme::Identifier:
        errorFree = !parseRule(iter, end, roots);
      break;

      case Lexeme::Import:
        errorFree = !parseImport(iter, end, roots);
      break;

      case Lexeme::Module:
        errorFree = !parseModule(iter, end, roots);
      break;

      case Lexeme::Namespace:
        errorFree = !parseNamespace(iter, end, roots);
      break;

      default:
        errorFree = false;
      break;
    }
  }

  if(errorFree) {
    return std::make_unique<ConcreteSyntaxTree>(roots);
  }
  else {
    return nullptr;
  }
}


static std::shared_ptr<AbstractIdentifierElement>
convertIdentifier(ConcreteTokenElement &concrete) {
  return std::make_shared<AbstractIdentifierElement>(concrete.token());
}


static std::shared_ptr<AbstractIdentifierElement>
convertIdentifier(ConcreteIdentifierElement &concrete) {
  if(concrete.size() == 1) {
    return std::make_shared<AbstractIdentifierElement>(
             as<ConcreteTokenElement>(concrete[0]).token()
           );
  }
  else if(concrete.size() & 1) {
    std::vector<std::shared_ptr<Token>> tmp;
    tmp.reserve((concrete.size() + 1) / 2);

    for(int i = 0; i < concrete.size(); i += 2) {
      tmp.emplace_back(as<ConcreteTokenElement>(concrete[i]).token());
    }

    return std::make_shared<AbstractIdentifierElement>(tmp);
  }

  return nullptr;
}


static int convertNamespace(AbstractSyntaxTree &ast,
                            ConcreteNamespaceElement &ns,
                            std::shared_ptr<AbstractNamespaceElement> &ptr) {
  if(auto ident = convertIdentifier(as<ConcreteIdentifierElement>(ns[1]))) {
    if(ast.addNamespace(ident, true)) {
      ptr = ast.currentNamespace();
      return 0;
    }
  }

  return -1;
}


static int convertModule(AbstractSyntaxTree &ast,
                         ConcreteModuleElement &mod,
                         std::shared_ptr<AbstractNamespaceElement> &ptr) {
  if(auto ident = convertIdentifier(as<ConcreteIdentifierElement>(mod[1]))) {
    if(ast.addNamespace(ident, false)) {
      ptr = ast.currentNamespace();
      return 0;
    }
  }

  return -1;
}


static int convertImport(AbstractSyntaxTree &ast, ConcreteImportElement &imp) {
  switch(imp.size()) {
    case 2: // simple import
      if(auto ident = convertIdentifier(as<ConcreteIdentifierElement>(imp[1]))) {
        ast.addImport(AbstractImportElement::importModule(ident));
        return 0;
      }
    break;

    case 4: // single element import or alias
      if(auto ident1 = convertIdentifier(as<ConcreteIdentifierElement>(imp[1]))) {
        if(auto ident2 = convertIdentifier(as<ConcreteIdentifierElement>(imp[3]))) {
          auto &joiner = as<ConcreteTokenElement>(imp[2]).token()->text();
          if(joiner == "as") {
            ast.addImport(AbstractImportElement::importModule(ident1, ident2));
            return 0;
          }
          else if(joiner == "from") {
            ast.addImport(AbstractImportElement::importElement(ident1, ident2));
            return 0;
          }
        }
      }
    break;

    case 6: // single element import with alias
      if(auto ident1 = convertIdentifier(as<ConcreteIdentifierElement>(imp[1]))) {
        if(auto ident2 = convertIdentifier(as<ConcreteIdentifierElement>(imp[3]))) {
          if(auto ident3 = convertIdentifier(as<ConcreteIdentifierElement>(imp[5]))) {
            ast.addImport(AbstractImportElement::importElement(ident1, ident2, ident3));
            return 0;
          }
        }
      }
    break;
  }

  return -1;
}


static int convertAlias(AbstractSyntaxTree &ast, ConcreteAliasElement &alias) {
  if(auto original = convertIdentifier(as<ConcreteIdentifierElement>(alias[1]))) {
    if(auto rename = convertIdentifier(as<ConcreteIdentifierElement>(alias[2]))) {
      ast.addAlias(std::make_shared<AbstractAliasElement>(original, rename));
      return 0;
    }
  }

  return -1;
}


std::shared_ptr<AbstractSexpr> convertSexpr(ConcreteSexprElement &src) {
  if(src.size() == 2) {
    return std::make_shared<AbstractSexpr>();
  }
  else if(src.size() > 2) {
    AbstractSexpr head, *tail = &head;

    for(auto iter = src.begin() + 1, end = src.end() - 1; iter != end; ++iter) {
      switch((*iter)->type()) {
        case ConcreteElementType::Identifier:
          tail->setNext(std::make_shared<AbstractSexpr>(
            convertIdentifier(as<ConcreteIdentifierElement>(*iter))
          ));
        break;

        case ConcreteElementType::SExpr:
          if(auto sub = convertSexpr(as<ConcreteSexprElement>(*iter))) {
            tail->setNext(std::make_shared<AbstractSexpr>(sub));
          }
          else {
            return nullptr;
          }
        break;

        case ConcreteElementType::Token:
          tail->setNext(std::make_shared<AbstractSexpr>(as<ConcreteTokenElement>(*iter).token()));
        break;
      }

      tail = tail->next().get();
    }

    return head.next();
  }

  return nullptr;
}


static std::shared_ptr<AbstractCodeSnippet> convertDecode(ConcreteDecodeElement &decode) {
  if(auto sexpr = convertSexpr(as<ConcreteSexprElement>(decode[1]))) {
    return std::make_shared<AbstractCodeSnippet>(sexpr);
  }

  return nullptr;
}


static std::shared_ptr<AbstractCodeSnippet> convertEncode(ConcreteEncodeElement &encode) {
  if(auto sexpr = convertSexpr(as<ConcreteSexprElement>(encode[1]))) {
    return std::make_shared<AbstractCodeSnippet>(sexpr);
  }

  return nullptr;
}


static std::shared_ptr<AbstractPatternElement>
convertCompoundPattern(ConcreteListElement &list,
                       std::shared_ptr<Token> lower,
                       std::shared_ptr<Token> upper,
                       std::shared_ptr<Token> bind);


static std::shared_ptr<Token> convertTokenOrIdentifier(ConcreteElement &concrete) {
  if(concrete.type() == ConcreteElementType::Token) {
    return reinterpret_cast<ConcreteTokenElement *>(&concrete)->token();
  }
  else if(concrete.type() == ConcreteElementType::Identifier) {
    auto name = reinterpret_cast<ConcreteIdentifierElement *>(&concrete);

    if(name->size() == 1) {
      return as<ConcreteTokenElement>((*name)[0]).token();
    }
    else {
      illegalComplexIdentifier(*name);
    }
  }

  return nullptr;
}


static std::shared_ptr<AbstractMatchCaseElement>
convertMatchCase(ConcreteBoundElement &binding) {
  if(binding.size() == 3) {
    auto key = binding[0], value = binding[2];

    if(key   && key->type()   == ConcreteElementType::Token &&
       value && value->type() == ConcreteElementType::Token) {
      auto key_token    = as<ConcreteTokenElement>(key).token();
      auto &value_token = as<ConcreteTokenElement>(value);

      if(key_token->isNumeric() || key_token->is(Lexeme::StringLiteral)) {
        if(value_token.token()->is(Lexeme::Identifier)) {
          return std::make_shared<AbstractMatchCaseElement>(
            key_token,
            convertIdentifier(value_token)
          );
        }
        else {
          unexpectedToken(value_token.token());
        }
      }
      else {
        unexpectedToken(key_token);
      }
    }
    else {
      std::cerr << "Malformed match case" << std::endl;
    }
  }
  else {
    std::cerr << "Invalid match case size" << std::endl;
  }

  return nullptr;
}


static std::shared_ptr<AbstractPatternElement>
convertMatchPattern(ConcreteMatchElement &match,
                    std::shared_ptr<Token> lower,
                    std::shared_ptr<Token> upper,
                    std::shared_ptr<Token> bound) {
  std::vector<std::shared_ptr<AbstractMatchCaseElement>> tmp;

  if(match.size() > 6) {
    if(auto discriminant = convertIdentifier(as<ConcreteIdentifierElement>(match[2]))) {
      for(decltype(match.size()) idx = 5, end = match.size() - 1; idx < end; ++idx) {
        if(auto bind = match[idx]) {
          if(auto element = convertMatchCase(as<ConcreteBoundElement>(bind))) {
            tmp.emplace_back(element);
          }
          else {
            return nullptr;
          }
        }
        else {
          std::cerr << "NULL match case" << std::endl;
          return nullptr;
        }
      }

      return std::make_shared<AbstractMatchElement>(discriminant, tmp, lower, upper, bound);
    }
    else {
      std::cerr << "Invalid match discriminant" << std::endl;
    }
  }
  else {
    std::cerr << "Empty match body" << std::endl;
  }

  return nullptr;
}


static std::shared_ptr<AbstractPatternElement>
convertMatchPattern(ConcreteMatchElement &match, std::shared_ptr<Token> bind) {
  return convertMatchPattern(match, nullptr, nullptr, bind);
}


static std::shared_ptr<AbstractPatternElement>
convertMatchPattern(ConcreteMatchElement &match) {
  return convertMatchPattern(match, nullptr);
}


static std::shared_ptr<AbstractPatternElement>
convertRepetitionPattern(ConcreteRepetitionElement &rep, std::shared_ptr<Token> bind) {
  if(rep.size() == 4) {
    auto element = rep[0], lower = rep[2];

    if(element && lower) {
      if(auto min = convertTokenOrIdentifier(*lower)) {

        switch(element->type()) {
          case ConcreteElementType::Identifier:
            return std::make_shared<AbstractSimplePatternElement>(
              convertIdentifier(as<ConcreteIdentifierElement>(element)),
              min,
              nullptr,
              bind
            );
          break;

          case ConcreteElementType::List:
            return convertCompoundPattern(as<ConcreteListElement>(element), min, nullptr, bind);
          break;

          case ConcreteElementType::Token:
            return std::make_shared<AbstractSimplePatternElement>(
              as<ConcreteTokenElement>(element).token(),
              min,
              nullptr,
              bind
            );
          break;

          default:
            std::cerr << "Unexpected CST type: " << stringify(element->type()) << std::endl;
          break;
        }
      }
    }
  }
  else if(rep.size() == 6) {
    auto element = rep[0], lower = rep[2], upper = rep[4];

    if(element && lower && upper) {
      auto min = convertTokenOrIdentifier(*lower);
      auto max = convertTokenOrIdentifier(*upper);

      if(min && max) {
        switch(element->type()) {
          case ConcreteElementType::Identifier:
            return std::make_shared<AbstractSimplePatternElement>(
              convertIdentifier(as<ConcreteIdentifierElement>(element)),
              min,
              max,
              bind
            );
          break;

          case ConcreteElementType::List:
            return convertCompoundPattern(as<ConcreteListElement>(element), min, max, bind);
          break;

          case ConcreteElementType::Token:
            return std::make_shared<AbstractSimplePatternElement>(
              as<ConcreteTokenElement>(element).token(),
              min,
              max,
              bind
            );
          break;

          default:
            std::cerr << "Unexpected CST type: " << stringify(element->type()) << std::endl;
          break;
        }
      }
    }
  }

  return nullptr;
}


static std::shared_ptr<AbstractPatternElement> convertBoundPattern(ConcreteBoundElement &bound) {
  if(bound.size() == 3) {
    auto element = bound[0], ident = bound[2];

    if(element && ident) {
      if(auto token = convertTokenOrIdentifier(*ident)) {
        switch(element->type()) {
          case ConcreteElementType::Identifier:
            return std::make_shared<AbstractSimplePatternElement>(
              convertIdentifier(as<ConcreteIdentifierElement>(element)),
              nullptr,
              nullptr,
              token
            );
          break;

          case ConcreteElementType::Repetition:
            return convertRepetitionPattern(
              as<ConcreteRepetitionElement>(element),
              token
            );
          break;

          case ConcreteElementType::List:
            return convertCompoundPattern(as<ConcreteListElement>(element), nullptr, nullptr, token);
          break;

          case ConcreteElementType::Match:
            return convertMatchPattern(as<ConcreteMatchElement>(element), token);
          break;

          case ConcreteElementType::Token:
            return std::make_shared<AbstractSimplePatternElement>(
              as<ConcreteTokenElement>(element).token(),
              nullptr,
              nullptr,
              token
            );
          break;

          default:
            std::cerr << "Unexpected CST type: " << stringify(element->type()) << std::endl;
          break;
        }
      }
    }
  }

  return nullptr;
}


static std::shared_ptr<AbstractPatternElement>
convertRepetitionPattern(ConcreteRepetitionElement &rep) {
  return convertRepetitionPattern(rep, nullptr);
}


static std::shared_ptr<AbstractPatternElement> convertCompoundPattern(ConcreteListElement &list) {
  return convertCompoundPattern(list, nullptr, nullptr, nullptr);
}


static std::shared_ptr<AbstractPatternElement>
convertCompoundPattern(ConcreteListElement &list,
                       std::shared_ptr<Token> lower,
                       std::shared_ptr<Token> upper,
                       std::shared_ptr<Token> bind) {
  if(list.size() > 2) {
    std::vector<std::shared_ptr<AbstractPatternElement>> tmp;

    for(decltype(list.size()) idx = 1, end = list.size() - 1; idx < end; ++idx) {
      auto element = list[idx];

      switch(element->type()) {
        case ConcreteElementType::Bound:
          if(auto bound = convertBoundPattern(as<ConcreteBoundElement>(element))) {
            tmp.emplace_back(bound);
          }
          else {
            return nullptr;
          }
        break;

        case ConcreteElementType::Identifier:
          tmp.emplace_back(std::make_shared<AbstractSimplePatternElement>(
            convertIdentifier(as<ConcreteIdentifierElement>(element))
          ));
        break;

        case ConcreteElementType::List:
          if(auto list = convertCompoundPattern(as<ConcreteListElement>(element))) {
            tmp.emplace_back(list);
          }
          else {
            return nullptr;
          }
        break;

        case ConcreteElementType::Match:
          if(auto match = convertMatchPattern(as<ConcreteMatchElement>(element))) {
            tmp.emplace_back(match);
          }
          else {
            return nullptr;
          }
        break;

        case ConcreteElementType::Repetition:
          if(auto repetition = convertRepetitionPattern(as<ConcreteRepetitionElement>(element))) {
            tmp.emplace_back(repetition);
          }
          else {
            return nullptr;
          }
        break;

        case ConcreteElementType::Token: {
          auto token = as<ConcreteTokenElement>(element).token();

          if(token->isNumeric(true) || token->is(Lexeme::StringLiteral)) {
            tmp.emplace_back(std::make_shared<AbstractSimplePatternElement>(token));
          }
          else {
            unexpectedToken(token);
            return nullptr;
          }
        } break;

        default:
          std::cerr << "Unexpected CST type: " << stringify(element->type()) << std::endl;
        break;
      }
    }

    if(tmp.size()) {
      return std::make_shared<AbstractCompoundPatternElement>(tmp, lower, upper, bind);
    }
  }

  return nullptr;
}


static std::shared_ptr<AbstractPatternList> convertPattern(ConcretePatternElement &pattern) {
  auto retVal = std::make_shared<AbstractPatternList>();
  std::vector<std::shared_ptr<AbstractPatternElement>> tmp;

  for(auto iter = pattern.begin() + 1, end = pattern.end(); iter != end; ++iter) {
    switch((*iter)->type()) {
      case ConcreteElementType::Bound:
        if(auto bound = convertBoundPattern(as<ConcreteBoundElement>(iter))) {
          tmp.emplace_back(bound);
        }
        else {
          return nullptr;
        }
      break;

      case ConcreteElementType::Identifier:
        tmp.emplace_back(std::make_shared<AbstractSimplePatternElement>(
          convertIdentifier(as<ConcreteIdentifierElement>(iter))
        ));
      break;

      case ConcreteElementType::List:
        if(auto list = convertCompoundPattern(as<ConcreteListElement>(iter))) {
          tmp.emplace_back(list);
        }
        else {
          return nullptr;
        }
      break;

      case ConcreteElementType::Match:
        if(auto match = convertMatchPattern(as<ConcreteMatchElement>(iter))) {
          tmp.emplace_back(match);
        }
        else {
          return nullptr;
        }
      break;

      case ConcreteElementType::Repetition:
        if(auto rep = convertRepetitionPattern(as<ConcreteRepetitionElement>(iter))) {
          tmp.emplace_back(rep);
        }
        else {
          return nullptr;
        }
      break;

      case ConcreteElementType::Token: {
        auto token = as<ConcreteTokenElement>(iter).token();

        if(token->text() == "|") {
          if(tmp.size() == 1) {
            retVal->add(tmp[0]);
            tmp.clear();
          }
          else if(tmp.size() > 1) {
            retVal->add(std::make_shared<AbstractCompoundPatternElement>(tmp));
            tmp.clear();
          }
          else {
            unexpectedToken(token);
            return nullptr;
          }
        }
        else if(token->isNumeric(true) || token->is(Lexeme::StringLiteral)) {
          tmp.emplace_back(std::make_shared<AbstractSimplePatternElement>(token));
        }
        else {
          unexpectedToken(token);
          return nullptr;
        }
      } break;
    }
  }

  if(tmp.size() == 1) {
    retVal->add(tmp[0]);
  }
  else if(tmp.size() > 1) {
    retVal->add(std::make_shared<AbstractCompoundPatternElement>(tmp));
  }

  return retVal;
}


static std::shared_ptr<AbstractStorageElement>
convertStorageElement(std::shared_ptr<ConcreteElement> element) {
  switch(element->type()) {
    case ConcreteElementType::Bound: {
      auto &bound = as<ConcreteBoundElement>(element);

      if(auto ident = convertIdentifier(as<ConcreteIdentifierElement>(bound[0]))) {
        return std::make_shared<AbstractStorageElement>(
          ident,
          convertIdentifier(as<ConcreteTokenElement>(bound[2]))
        );
      }
    } break;

    case ConcreteElementType::Identifier:
      if(auto ident = convertIdentifier(as<ConcreteIdentifierElement>(element))) {
        return std::make_shared<AbstractStorageElement>(ident);
      }
    break;

    default:
      std::cerr << "Invalid CST type: " << stringify(element->type()) << std::endl;
    break;
  }

  return nullptr;
}


static std::shared_ptr<AbstractStorageList> convertStorage(ConcreteStorageElement &storage) {
  if(storage.size() == 2) {
    auto single = storage[1];
    if(single->type() == ConcreteElementType::Identifier) {
      auto &ident = as<ConcreteIdentifierElement>(single);
      if(ident.size() == 1 && as<ConcreteTokenElement>(ident[0]).token()->text() == "none") {
        return std::make_shared<AbstractStorageList>(); //empty list implies no storage at all
      }
      else if(auto element = convertStorageElement(single)) {
        return std::make_shared<AbstractStorageList>(element);
      }
    }
    else if(single->type() == ConcreteElementType::Bound) {
      if(auto element = convertStorageElement(single)) {
        return std::make_shared<AbstractStorageList>(element);
      }
    }
    else {
      std::cerr << "Invalid CST type: " << stringify(single->type()) << std::endl;
    }
  }
  else if(storage.size() > 3) {
    auto list = std::make_shared<AbstractStorageList>();
    auto count = storage.size() - 1;

    for(decltype(count) idx = 2; idx < count; ++idx) {
      if(auto element = convertStorageElement(storage[idx])) {
        list->add(element);
      }
      else {
        return nullptr;
      }
    }

    return list;
  }

  return nullptr;
}


static std::shared_ptr<AbstractCodeSnippet> convertValidate(ConcreteValidateElement &validate) {
  if(auto sexpr = convertSexpr(as<ConcreteSexprElement>(validate[1]))) {
    return std::make_shared<AbstractCodeSnippet>(sexpr);
  }

  return nullptr;
}


static void duplicate(ConcreteCompoundElement &concrete, AbstractElement &abstract, const char *name) {
  std::cerr << "Duplicate "    << name << " at " << concrete.file() << ':' <<
               concrete.line() << '.'  << concrete.column() << std::endl <<
               "Previously seen at "   << abstract.file()   << ':' <<
               abstract.line() << '.'  << abstract.column() << std::endl;
}


static int convertRule(AbstractSyntaxTree &ast, ConcreteRuleElement &rule) {
  auto count = rule.size();

  if(3 < count && count < 9) {
    auto ident = convertIdentifier(as<ConcreteIdentifierElement>(rule[0]));
    std::shared_ptr<AbstractPatternList> pattern( nullptr);
    std::shared_ptr<AbstractStorageList> storage( nullptr);
    std::shared_ptr<AbstractCodeSnippet> validate(nullptr);
    std::shared_ptr<AbstractCodeSnippet> encode(  nullptr);
    std::shared_ptr<AbstractCodeSnippet> decode(  nullptr);

    for(auto idx = count - 1; idx-- > 2;) {
      auto element = rule[idx];
      switch(element->type()) {
        case ConcreteElementType::Decode:
          if(decode) {
            duplicate(as<ConcreteCompoundElement>(element), *decode, "decode:");
            return -1;
          }

          if(!(decode = convertDecode(as<ConcreteDecodeElement>(element)))) {
            return -1;
          }
        break;

        case ConcreteElementType::Encode:
          if(encode) {
            duplicate(as<ConcreteCompoundElement>(element), *encode, "encode:");
            return -1;
          }

          if(!(encode = convertEncode(as<ConcreteEncodeElement>(element)))) {
            return -1;
          }
        break;

        case ConcreteElementType::Pattern:
          if(pattern) {
            duplicate(as<ConcreteCompoundElement>(element), *pattern, "pattern:");
            return -1;
          }

          if(!(pattern = convertPattern(as<ConcretePatternElement>(element)))) {
            return -1;
          }
        break;

        case ConcreteElementType::Storage:
          if(storage) {
            duplicate(as<ConcreteCompoundElement>(element), *storage, "storage:");
            return -1;
          }

          if(!(storage = convertStorage(as<ConcreteStorageElement>(element)))) {
            return -1;
          }
        break;

        case ConcreteElementType::Validate:
          if(validate) {
            duplicate(as<ConcreteCompoundElement>(element), *validate, "validate:");
            return -1;
          }

          if(!(validate = convertValidate(as<ConcreteValidateElement>(element)))) {
            return -1;
          }
        break;

        default:
          return -1;
        break;
      }
    }

    ast.currentNamespace()->add(std::make_shared<AbstractRuleElement>(
                                  ident, pattern, storage, validate, encode, decode
                                ));
    return 0;
  }

  return -1;
}


std::unique_ptr<AbstractSyntaxTree>
Parser::abstractParse(ConcreteSyntaxTree &concrete) {
  auto tree  = std::make_unique<AbstractSyntaxTree>();
  auto space = tree->currentNamespace();
  auto iter  = concrete.begin(), end = concrete.end();
  bool errorFree = true;

  while(errorFree && iter != end) {
    switch((*iter)->type()) {
      case ConcreteElementType::Comment:
      case ConcreteElementType::Documentation:
        // ignore this
      break;

      case ConcreteElementType::Namespace:
        errorFree = !convertNamespace(*tree, as<ConcreteNamespaceElement>(iter), space);
      break;

      case ConcreteElementType::Module:
        errorFree = !convertModule(*tree, as<ConcreteModuleElement>(iter), space);
      break;

      case ConcreteElementType::Import:
        errorFree = !convertImport(*tree, as<ConcreteImportElement>(iter));
      break;

      case ConcreteElementType::Alias:
        errorFree = !convertAlias(*tree, as<ConcreteAliasElement>(iter));
      break;

      case ConcreteElementType::Rule:
        errorFree = !convertRule(*tree, as<ConcreteRuleElement>(iter));
      break;

      default:
        std::cout << **iter << std::endl;
      break;
    }

    ++iter;
  }

  if(errorFree) {
    return tree;
  }
  else {
    return nullptr;
  }
}


std::unique_ptr<AbstractSyntaxTree> Parser::parse(Tokenizer &tokenizer) {
  if(auto concrete = concreteParse(tokenizer)) {
    return abstractParse(*concrete);
  }
  else {
    return nullptr;
  }
}

