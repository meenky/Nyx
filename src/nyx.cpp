#include "nyx/syntax/parser.h"

#include <iostream>


int main(int argc, char **argv) {
  for(int arg = 1; arg < argc; ++arg) {
    nyx::syntax::Tokenizer input(std::make_shared<std::string>(argv[arg]));

    if(!input) {
      std::cerr << "Failure to open " << argv[arg] << " for reading" << std::endl;
      continue;
    }

    if(auto tree = nyx::syntax::Parser::parse(input)) {
      tree->print(std::cout);
    }
  }

  return 0;
}

