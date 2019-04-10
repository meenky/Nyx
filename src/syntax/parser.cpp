#include "nyx/syntax/parser.h"


using namespace nyx::syntax;


Parser::Parser(std::shared_ptr<Tokenizer> src):
  tokenizer(src) {
}


//static auto asToken(std::shared_ptr<const ConcreteElement> ce) {
//  return reinterpret_cast<const ConcreteTokenElement *>(ce.get())->token();
//}


typedef std::vector<std::shared_ptr<Token>>::iterator      token_iterator;
typedef std::vector<std::shared_ptr<ConcreteElement>>      concrete_vector;


auto extract(token_iterator start, token_iterator end) {
  std::vector<std::shared_ptr<ConcreteElement>> retVal;

  retVal.reserve(end - start);

  while(start != end) {
    retVal.emplace_back(std::make_shared<ConcreteTokenElement>(*start));
    ++start;
  }

  return retVal;
}


auto extractNoEol(token_iterator start, token_iterator end) {
  std::vector<std::shared_ptr<ConcreteElement>> retVal;

  retVal.reserve(end - start);

  while(start != end) {
    if((*start)->lexeme() != Lexeme::EndOfLine) {
      retVal.emplace_back(std::make_shared<ConcreteTokenElement>(*start));
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


enum class AliasParseState {
  Error = -1,
  Ready,
  Keyword,
  First,
  Second
};


static int parseAlias(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  AliasParseState state = AliasParseState::Ready;

  while(state != AliasParseState::Error && iter != last) {
    switch(state) {
      case AliasParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Alias) {
          state = AliasParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = AliasParseState::Error;
        }
      break;

      case AliasParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = AliasParseState::First;
        }
        else {
          unexpectedToken(*iter);
          state = AliasParseState::Error;
        }
      break;

      case AliasParseState::First:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = AliasParseState::Second;
        }
        else {
          unexpectedToken(*iter);
          state = AliasParseState::Error;
        }
      break;

      case AliasParseState::Second:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteAliasElement>(extract(start, iter)));
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


enum class ImportParseState {
  Error = -1,
  Ready,
  Keyword,
  Identifier,
  Dot,
  Namespace,
  From,
  As,
  Alias
};


static int parseImport(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  ImportParseState state = ImportParseState::Ready;

  while(state != ImportParseState::Error && iter != last) {
    switch(state) {
      case ImportParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Import) {
          state = ImportParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = ImportParseState::Identifier;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Identifier:
        if((*iter)->lexeme() == Lexeme::Dot) {
          state = ImportParseState::Dot;
        }
        else if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteImportElement>(extract(start, iter)));
          start = iter;
          return 0;
        }
        else if((*iter)->text() == "from") {
          state = ImportParseState::From;
        }
        else if((*iter)->text() == "as") {
          state = ImportParseState::As;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::From:
      case ImportParseState::Dot:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = ImportParseState::Namespace;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Namespace:
        if((*iter)->lexeme() == Lexeme::Dot) {
          state = ImportParseState::Dot;
        }
        else if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteImportElement>(extract(start, iter)));
          start = iter;
          return 0;
        }
        else if((*iter)->text() == "as") {
          state = ImportParseState::As;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::As:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = ImportParseState::Alias;
        }
        else {
          unexpectedToken(*iter);
          state = ImportParseState::Error;
        }
      break;

      case ImportParseState::Alias:
        if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteImportElement>(extract(start, iter)));
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
  Identifier,
  Dot
};


static int parseModule(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  ModuleParseState state = ModuleParseState::Ready;

  while(state != ModuleParseState::Error && iter != last) {
    switch(state) {
      case ModuleParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Module) {
          state = ModuleParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = ModuleParseState::Error;
        }
      break;

      case ModuleParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = ModuleParseState::Identifier;
        }
        else {
          unexpectedToken(*iter);
          state = ModuleParseState::Error;
        }
      break;

      case ModuleParseState::Identifier:
        if((*iter)->lexeme() == Lexeme::Dot) {
          state = ModuleParseState::Dot;
        }
        else if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteModuleElement>(extract(start, iter)));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = ModuleParseState::Error;
        }
      break;

      case ModuleParseState::Dot:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = ModuleParseState::Identifier;
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
  Identifier,
  Dot
};


static int parseNamespace(token_iterator &start, token_iterator last, concrete_vector &roots) {
  token_iterator iter = start;
  NamespaceParseState state = NamespaceParseState::Ready;

  while(state != NamespaceParseState::Error && iter != last) {
    switch(state) {
      case NamespaceParseState::Ready:
        if((*iter)->lexeme() == Lexeme::Namespace) {
          state = NamespaceParseState::Keyword;
        }
        else {
          unexpectedToken(*iter);
          state = NamespaceParseState::Error;
        }
      break;

      case NamespaceParseState::Keyword:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = NamespaceParseState::Identifier;
        }
        else {
          unexpectedToken(*iter);
          state = NamespaceParseState::Error;
        }
      break;

      case NamespaceParseState::Identifier:
        if((*iter)->lexeme() == Lexeme::Dot) {
          state = NamespaceParseState::Dot;
        }
        else if((*iter)->lexeme() == Lexeme::EndOfLine) {
          roots.emplace_back(std::make_shared<ConcreteNamespaceElement>(extract(start, iter)));
          start = iter;
          return 0;
        }
        else {
          unexpectedToken(*iter);
          state = NamespaceParseState::Error;
        }
      break;

      case NamespaceParseState::Dot:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          state = NamespaceParseState::Identifier;
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

  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
  while(++iter != last) {
    switch((*iter)->lexeme()) {
      case Lexeme::EndOfLine:
        // do nothing
      break;

      case Lexeme::CloseParen:
        parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
      case Lexeme::Identifier:
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
        parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*token));
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
  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*start));

  while(state != RepetitionParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // always ignore line ends
    }

    switch(state) {
      case RepetitionParseState::Ready:
        switch((*iter)->lexeme()) {
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::OctalLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
            state = RepetitionParseState::HasLowerBound;
          break;

          case Lexeme::Plus:
          case Lexeme::Query:
          case Lexeme::Times:
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = RepetitionParseState::Comma;
        }
        else if((*iter)->lexeme() == Lexeme::CloseCurly) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
          case Lexeme::Plus:
          case Lexeme::Times:
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::OctalLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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

  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*start));

  while(state != MatchParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // always ignore line ends
    }

    switch(state) {
      case MatchParseState::Ready:
        if((*iter)->lexeme() == Lexeme::OpenParen) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = MatchParseState::InHead;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::InHead:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = MatchParseState::HasDiscriminant;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::HasDiscriminant:
        if((*iter)->lexeme() == Lexeme::CloseParen) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = MatchParseState::HasHead;
        }
        else if((*iter)->lexeme() == Lexeme::Dot) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = MatchParseState::InHead;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::HasHead:
        if((*iter)->lexeme() == Lexeme::OpenCurly) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = MatchParseState::InBody;
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
        }
      break;

      case MatchParseState::InBody:
        switch((*iter)->lexeme()) {
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::OctalLiteral:
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
          case Lexeme::Identifier:
          case Lexeme::BinaryLiteral:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::OctalLiteral:
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
            state = MatchParseState::HasElement;
          break;

          case Lexeme::CloseCurly:
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
        if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = MatchParseState::HasElement;
        }
        else if((*iter)->lexeme() == Lexeme::CloseCurly) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          start = iter;
          return std::make_shared<ConcreteMatchElement>(parts);
        }
        else {
          unexpectedToken(*iter);
          state = MatchParseState::Error;
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

  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*start));

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
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
          case Lexeme::BinaryLiteral:
          case Lexeme::BinaryPattern:
          case Lexeme::DecimalLiteral:
          case Lexeme::HexadecimalLiteral:
          case Lexeme::HexadecimalPattern:
          case Lexeme::OctalLiteral:
          case Lexeme::OctalPattern:
          case Lexeme::StringLiteral:
            base = std::make_shared<ConcreteTokenElement>(*iter);
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
            op = std::make_shared<ConcreteTokenElement>(*iter);
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
            concrete_vector{base, op, std::make_shared<ConcreteTokenElement>(*iter)}
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
            op = std::make_shared<ConcreteTokenElement>(*iter);
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

  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*start));

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
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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

  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*start));

  while(state != StorageParseState::Error && ++iter != last) {
    if((*iter)->lexeme() == Lexeme::EndOfLine) {
      continue; // line endings mean nothing
    }

    switch(state) {
      case StorageParseState::Ready:
        if((*iter)->lexeme() == Lexeme::OpenSquare) {
          state = StorageParseState::ListReady;
        }
        else if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = StorageParseState::ListIdentifier;
        }
        else {
          unexpectedToken(*iter);
          state = StorageParseState::Error;
        }
      break;

      case StorageParseState::ListIdentifier:
        if((*iter)->lexeme() == Lexeme::Identifier) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = StorageParseState::ListIdentifier;
        }
        else if((*iter)->lexeme() == Lexeme::Bind) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = StorageParseState::ListBinding;
        }
        else if((*iter)->lexeme() == Lexeme::CloseSquare) {
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
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
          state = StorageParseState::ListIdentifier;
        }
        else if((*iter)->lexeme() == Lexeme::CloseSquare) {
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

  parts.emplace_back(std::make_shared<ConcreteTokenElement>(*start));

  while(state != RuleParseState::Error && ++iter != last) {
    switch(state) {
      case RuleParseState::Ready:
        if((*iter)->lexeme() == Lexeme::OpenCurly) {
          parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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
            parts.emplace_back(std::make_shared<ConcreteTokenElement>(*iter));
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


std::shared_ptr<ConcreteSyntaxTree> Parser::parse() {
  std::vector<std::shared_ptr<Token>> tokens;
  std::vector<std::shared_ptr<ConcreteElement>> roots;

  tokens.reserve(8192);

  // read in all the tokens
  while(auto token = tokenizer->next()) {
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

  //if(errorFree){
    return std::make_shared<ConcreteSyntaxTree>(roots);
  //}
  //else {
  //  return nullptr;
  //}
}

