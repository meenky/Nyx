#pragma once

#include <nyx/registry.h>

#include <map>
#include <vector>
#include <stdint.h>


namespace nyx {


class Stage {
  public:
    Stage();
    Stage(const nyx::syntax::AbstractMatchElement &);
    Stage(const nyx::syntax::AbstractSimplePatternElement &);
    Stage(const nyx::syntax::AbstractCompoundPatternElement &);
    Stage(const std::vector<uint8_t> &, const std::string &min, const std::string &max,
          const std::string & name);

    Stage(const Stage &);
    Stage& operator=(const Stage&);

    const Stage *next() const {
      return stage.get();
    }

    bool isVariableRepeat() const {
      return min != max;
    }

    bool isUnbounded() const {
      return max == "-1";
    }

    bool isOptional() const {
      return min == "0";
    }

    bool isSingleRepeat() const {
      return min == "1" && max == "1";
    }

    bool isPrimitive() const {
      return exact.size() > 0;
    }

    bool isCompound() const {
      return static_cast<bool>(sub);
    }

    bool isMatch() const {
      return select.size() > 0;
    }

    bool isWildcard() const {
      switch(what) {
        case nyx::syntax::Lexeme::BinaryPattern:
        case nyx::syntax::Lexeme::OctalPattern:
        case nyx::syntax::Lexeme::HexadecimalPattern:
          return true;
        break;
      }

      return false;
    }

    const Stage *group() const {
      return sub.get();
    }

    const std::string &minimum() const {
      return min;
    }

    const std::string &maximum() const {
      return max;
    }

    bool hasName() const {
      return ident.size() > 0;
    }

    const std::string &name() const {
      return ident;
    }

    const std::string &reference() const {
      return ref;
    }

    const std::vector<uint8_t> &pattern() const {
      return exact;
    }

    std::pair<uint8_t, uint8_t> wildcard() const {
      return wild;
    }

    const std::map<uint64_t, std::string> &match() const {
      return select;
    }

    nyx::syntax::Lexeme lexeme() const {
      return what;
    }

  protected:
    std::unique_ptr<Stage>          stage;
    std::unique_ptr<Stage>          sub;
    std::string                     min;
    std::string                     max;
    std::vector<uint8_t>            exact;
    std::string                     ident;
    std::string                     ref;
    std::pair<uint8_t, uint8_t>     wild;
    std::map<uint64_t, std::string> select;
    nyx::syntax::Lexeme             what;

  private:
    void assignMetadata(const nyx::syntax::AbstractPatternElement &);
};

class Alternate {
  public:
    Alternate(const nyx::syntax::AbstractPatternElement &);

    const Stage &pattern() const {
      return *stage;
    }

  protected:
    std::unique_ptr<Stage> stage;
};


class Pattern {
  public:
    Pattern(const nyx::syntax::AbstractPatternList &);

    const std::vector<Alternate> &alternates() const {
      return list;
    }

  protected:
    std::vector<Alternate> list;
};


class Storage {
  public:
    Storage(const std::shared_ptr<nyx::syntax::AbstractStorageList> &);

    bool isValid() const {
      return members.size() > 0;
    }

    const std::vector<std::pair<std::string, std::vector<std::string>>> &elements() const {
      return members;
    }

  protected:
    std::vector<std::pair<std::string, std::vector<std::string>>> members;
};


class Code {
  public:
    Code(const std::shared_ptr<nyx::syntax::AbstractCodeSnippet> &);

    bool isValid() const {
      return static_cast<bool>(expr);
    }

    const std::shared_ptr<nyx::syntax::AbstractSexpr> sexpr() const {
      return expr;
    }

  protected:
    std::shared_ptr<nyx::syntax::AbstractSexpr> expr;
};


class Rule {
  public:
    Rule(const nyx::syntax::AbstractRuleElement &);

    const std::string &name() const {
      return ident;
    }

    Pattern &pattern() {
      return pat;
    }

    const Pattern &pattern() const {
      return pat;
    }

    bool hasStorage() const {
      return store.isValid();
    }

    Storage &storage() {
      return store;
    }

    const Storage &storage() const {
      return store;
    }

    bool hasEncode() const {
      return enc.isValid();
    }

    Code &encode() {
      return enc;
    }

    const Code &encode() const {
      return enc;
    }

    bool hasDecode() const {
      return dec.isValid();
    }

    Code &decode() {
      return dec;
    }

    const Code &decode() const {
      return dec;
    }

    bool hasValidation() const {
      return val.isValid();
    }

    Code &validation() {
      return val;
    }

    const Code &validation() const {
      return val;
    }

  protected:
    std::string ident;
    Pattern pat;
    Storage store;
    Code    enc;
    Code    dec;
    Code    val;
};


class Import {
  public:
    Import(const nyx::syntax::AbstractImportElement &);

    bool hasMember() const {
      return mem.size() > 0;
    }

    const std::string &member() const {
      return mem;
    }

    const std::vector<std::string> &module() const {
      return space;
    }

    bool hasAlias() const {
      return ident.size() > 0;
    }

    const std::string &alias() const {
      return ident;
    }

  protected:
    std::vector<std::string> space;
    std::string              ident;
    std::string              mem;
};


class Namespace {
  public:
    Namespace(const nyx::syntax::AbstractIdentifierElement &);

    const std::vector<std::string> &parts() const {
      return module;
    }

    const std::vector<Import> &imports() const {
      return requires;
    }

    const std::vector<Rule> &rules() const {
      return members;
    }

    void addRule(const nyx::syntax::AbstractRuleElement &);
    void addImport(const nyx::syntax::AbstractImportElement &);

  protected:
    std::vector<std::string> module;
    std::vector<Rule>        members;
    std::vector<Import>      requires;
};


class Plan {
  public:
    Plan();

    const std::vector<Namespace> &namespaces() const {
      return spaces;
    }

    static std::unique_ptr<Plan> generate(Registry &);

  protected:
    Namespace &addNamespace(const nyx::syntax::AbstractIdentifierElement &);

    std::vector<Namespace> spaces;
};


}
