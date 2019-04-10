#pragma once

#include "nyx/syntax/concrete.h"
#include "nyx/syntax/tokenizer.h"

#include <memory>


namespace nyx {
  namespace syntax {


class Parser {
  public:
    Parser(std::shared_ptr<Tokenizer> tokenizer);

    Parser(Parser &&) = delete;
    Parser(const Parser &) = delete;
    Parser &operator=(const Parser &) = delete;

    std::shared_ptr<nyx::syntax::ConcreteSyntaxTree> parse();

  private:
    std::shared_ptr<Tokenizer> tokenizer;
};


  }
}

