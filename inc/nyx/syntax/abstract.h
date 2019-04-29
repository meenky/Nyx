#pragma once

#include "nyx/syntax/token.h"

#include <map>
#include <memory>
#include <vector>
#include <iostream>


namespace nyx {
  namespace syntax {


enum class AbstractElementType {
  Alias,
  AliasList,
  Code,
  Identifier,
  Import,
  ImportList,
  Match,
  MatchCase,
  Module,
  Namespace,
  Pattern,
  SimplePatternElement,
  CompoundPatternElement,
  PatternList,
  Rule,
  StorageElement,
  StorageList
};


class AbstractElement {
  protected:
    AbstractElement(AbstractElementType, int = -1, int = -1, std::shared_ptr<std::string> = nullptr);

  public:
    virtual ~AbstractElement();

    inline AbstractElementType type() const {
      return kind;
    }

    int line() const;
    int column() const;
    const std::string &file() const;
    virtual std::ostream &print(std::ostream &os) const = 0;
    virtual std::ostream &debug(std::ostream &os) const = 0;

    inline bool is(AbstractElementType type) const {
      return kind == type;
    }

  private:
    std::shared_ptr<std::string> fileName;
    AbstractElementType          kind;
    int                          lineNumber;
    int                          columnNumber;
};


std::ostream &operator<<(std::ostream &os, const AbstractElement &ce);
std::ostream &operator<<(std::ostream &os, const std::shared_ptr<const AbstractElement> &ce);


template<typename BASE = AbstractElement>
class AbstractCompoundMixin {
  public:
    virtual ~AbstractCompoundMixin() {}

    typedef typename std::vector<std::shared_ptr<BASE>>::iterator iterator;
    typedef typename std::vector<std::shared_ptr<BASE>>::const_iterator const_iterator;
    typedef typename std::vector<std::shared_ptr<BASE>>::reverse_iterator reverse_iterator;
    typedef typename std::vector<std::shared_ptr<BASE>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { elements.begin();   }
    inline iterator               end()           { elements.end();     }
    inline const_iterator         begin()   const { elements.begin();   }
    inline const_iterator         end()     const { elements.end();     }
    inline const_iterator         cbegin()  const { elements.cbegin();  }
    inline const_iterator         cend()    const { elements.cend();    }
    inline reverse_iterator       rbegin()        { elements.rbegin();  }
    inline reverse_iterator       rend()          { elements.rend();    }
    inline const_reverse_iterator rbegin()  const { elements.rbegin();  }
    inline const_reverse_iterator rend()    const { elements.rend();    }
    inline const_reverse_iterator crbegin() const { elements.crbegin(); }
    inline const_reverse_iterator crend()   const { elements.crend();   }

    inline auto size() const {
      return elements.size();
    }

    inline bool isSimple() const {
      return size() == 1;
    }

    inline bool isEmpty() const {
      return size() == 0;
    }

    inline std::shared_ptr<AbstractElement> &operator[](int idx) {
      return elements[idx];
    }

    inline std::shared_ptr<const AbstractElement> &operator[](int idx) const {
      return elements[idx];
    }

  protected:
    AbstractCompoundMixin():
      elements() {
    }

    AbstractCompoundMixin(std::shared_ptr<BASE> simple):
      elements() {
      elements.emplace_back(simple);
    }

    AbstractCompoundMixin(const std::vector<std::shared_ptr<BASE>> &compound):
      elements(compound) {
    }

    AbstractCompoundMixin(iterator start, iterator end):
      elements(start, end) {
    }

    std::vector<std::shared_ptr<BASE>> elements;
};


template<typename BASE = AbstractElement>
class AbstractLookupMixin {
  public:
    virtual ~AbstractLookupMixin() {}

    typedef typename std::map<const std::string, std::shared_ptr<BASE>>::iterator iterator;
    typedef typename std::map<const std::string, std::shared_ptr<BASE>>::const_iterator const_iterator;
    typedef typename std::map<const std::string, std::shared_ptr<BASE>>::reverse_iterator reverse_iterator;
    typedef typename std::map<const std::string, std::shared_ptr<BASE>>::const_reverse_iterator const_reverse_iterator;

    inline iterator               begin()         { elements.begin();   }
    inline iterator               end()           { elements.end();     }
    inline const_iterator         begin()   const { elements.begin();   }
    inline const_iterator         end()     const { elements.end();     }
    inline const_iterator         cbegin()  const { elements.cbegin();  }
    inline const_iterator         cend()    const { elements.cend();    }
    inline reverse_iterator       rbegin()        { elements.rbegin();  }
    inline reverse_iterator       rend()          { elements.rend();    }
    inline const_reverse_iterator rbegin()  const { elements.rbegin();  }
    inline const_reverse_iterator rend()    const { elements.rend();    }
    inline const_reverse_iterator crbegin() const { elements.crbegin(); }
    inline const_reverse_iterator crend()   const { elements.crend();   }

    inline auto size() const {
      return elements.size();
    }

    inline bool isEmpty() const {
      return size() == 0;
    }

     const std::shared_ptr<BASE> operator[](const std::string key) const {
      auto iter = elements.find(key);

      if(iter != end) {
        return iter->second;
      }

      return nullptr;
    }

    std::shared_ptr<BASE> &operator[](const std::string key) {
      return elements[key];
    }

  protected:
    AbstractLookupMixin():
      elements() {
    }

    AbstractLookupMixin(const std::string &key, std::shared_ptr<BASE> simple):
      elements() {
      elements.emplace(key, simple);
    }

    AbstractLookupMixin(const std::map<const std::string, std::shared_ptr<BASE>> &compound):
      elements(compound) {
    }

    AbstractLookupMixin(iterator start, iterator end):
      elements(start, end) {
    }

    std::map<const std::string, std::shared_ptr<BASE>> elements;
};


class AbstractMultiTokenElement: public AbstractElement, public AbstractCompoundMixin<Token> {
  public:
    virtual ~AbstractMultiTokenElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<Token>);

  protected:
    AbstractMultiTokenElement(AbstractElementType type);
    AbstractMultiTokenElement(AbstractElementType type, std::shared_ptr<Token> simple);
    AbstractMultiTokenElement(AbstractElementType type,
                              const std::vector<std::shared_ptr<Token>> &compound);
    AbstractMultiTokenElement(AbstractElementType type, iterator first, iterator end);
};


class AbstractIdentifierElement: public AbstractMultiTokenElement {
  public:
    AbstractIdentifierElement(std::shared_ptr<Token> simple);
    AbstractIdentifierElement(const std::vector<std::shared_ptr<Token>> &compound);
    AbstractIdentifierElement(iterator first, iterator end);
    AbstractIdentifierElement(const AbstractIdentifierElement &first,
                              const AbstractIdentifierElement &second);

    virtual ~AbstractIdentifierElement();

    virtual std::ostream &print(std::ostream &os) const;

    inline bool isCompound() {
      return size() > 1;
    }

    std::string toString() const;
};


class AbstractImportElement: public AbstractElement {
  public:
    static std::shared_ptr<AbstractImportElement>
    importModule(std::shared_ptr<AbstractIdentifierElement> name);

    static std::shared_ptr<AbstractImportElement>
    importModule(std::shared_ptr<AbstractIdentifierElement> name,
                 std::shared_ptr<AbstractIdentifierElement> alias);

    static std::shared_ptr<AbstractImportElement>
    importElement(std::shared_ptr<AbstractIdentifierElement> name,
                  std::shared_ptr<AbstractIdentifierElement> module);

    static std::shared_ptr<AbstractImportElement>
    importElement(std::shared_ptr<AbstractIdentifierElement> name,
                  std::shared_ptr<AbstractIdentifierElement> module,
                  std::shared_ptr<AbstractIdentifierElement> alias);

    virtual ~AbstractImportElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    inline auto module() {
      return module_ptr;
    }

    inline auto module() const {
      return module_ptr;
    }

    inline auto alias() {
      return alias_ptr;
    }

    inline auto alias() const {
      return alias_ptr;
    }

    inline auto element() {
      return element_ptr;
    }

    inline auto element() const {
      return element_ptr;
    }

    inline bool hasAlias() const {
      return static_cast<bool>(alias_ptr);
    }

    inline bool hasElement() const {
      return static_cast<bool>(element_ptr);
    }


  protected:
    std::shared_ptr<AbstractIdentifierElement> module_ptr;
    std::shared_ptr<AbstractIdentifierElement> alias_ptr;
    std::shared_ptr<AbstractIdentifierElement> element_ptr;

    AbstractImportElement(std::shared_ptr<AbstractIdentifierElement> module,
                          std::shared_ptr<AbstractIdentifierElement> alias,
                          std::shared_ptr<AbstractIdentifierElement> element);
};


class AbstractImportList: public AbstractElement, public AbstractCompoundMixin<AbstractImportElement> {
  public:
    AbstractImportList();
    AbstractImportList(std::shared_ptr<AbstractImportElement> simple);
    AbstractImportList(const std::vector<std::shared_ptr<AbstractImportElement>> &multi);
    AbstractImportList(iterator first, iterator end);

    virtual ~AbstractImportList();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<AbstractImportElement>);
};


class AbstractAliasElement: public AbstractElement {
  public:
    AbstractAliasElement(std::shared_ptr<AbstractIdentifierElement> original,
                         std::shared_ptr<AbstractIdentifierElement> alias);
    virtual ~AbstractAliasElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    inline std::shared_ptr<AbstractIdentifierElement> original() {
      return original_ptr;
    }


    inline std::shared_ptr<AbstractIdentifierElement> alias() {
      return alias_ptr;
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> original_ptr;
    std::shared_ptr<AbstractIdentifierElement> alias_ptr;
};


class AbstractAliasList: public AbstractElement, public AbstractLookupMixin<AbstractAliasElement> {
  public:
    AbstractAliasList();
    AbstractAliasList(const std::string &key, std::shared_ptr<AbstractAliasElement> simple);
    AbstractAliasList(const std::map<const std::string, std::shared_ptr<AbstractAliasElement>> &multi);
    AbstractAliasList(iterator first, iterator end);
    virtual ~AbstractAliasList();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<AbstractAliasElement>);
};


class AbstractMatchCaseElement: public AbstractElement {
  public:
    AbstractMatchCaseElement(std::shared_ptr<Token>                     key,
                             std::shared_ptr<AbstractIdentifierElement> value);

    virtual ~AbstractMatchCaseElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    auto key() {
      return key_ptr;
    }

    auto key() const {
      return key_ptr;
    }

    auto value() {
      return value_ptr;
    }

    auto value() const {
      return value_ptr;
    }

  protected:
    std::shared_ptr<Token>                     key_ptr;
    std::shared_ptr<AbstractIdentifierElement> value_ptr;
};


class AbstractPatternElement: public AbstractElement {
  public:
    virtual ~AbstractPatternElement();

    virtual std::ostream &print(std::ostream &os) const = 0;
    virtual std::ostream &debug(std::ostream &os) const = 0;

    inline bool isSimple() const {
      return simple;
    }

    inline bool isCompound() const {
      return !isSimple();
    }

    inline bool isMatch() const {
      return type() == AbstractElementType::Match;
    }

    inline bool hasMinimum() const {
      return static_cast<bool>(min);
    }

    inline bool hasMaximum() const {
      return static_cast<bool>(max);
    }

    inline bool hasBinding() const {
      return static_cast<bool>(bind);
    }

    auto minimum() {
      return min;
    }

    auto maximum() {
      return max;
    }

    auto binding() {
      return bind;
    }

  protected:
    AbstractPatternElement(AbstractElementType    type,
                           bool                   simple,
                           std::shared_ptr<Token> lower,
                           std::shared_ptr<Token> upper,
                           std::shared_ptr<Token> bind);

    std::shared_ptr<Token>                     min;
    std::shared_ptr<Token>                     max;
    std::shared_ptr<Token>                     bind;
    bool simple;
};


class AbstractSimplePatternElement: public AbstractPatternElement {
  public:
    AbstractSimplePatternElement(std::shared_ptr<AbstractIdentifierElement> member,
                                 std::shared_ptr<Token> lower = nullptr,
                                 std::shared_ptr<Token> upper = nullptr,
                                 std::shared_ptr<Token> bind = nullptr);
    AbstractSimplePatternElement(std::shared_ptr<Token> member,
                                 std::shared_ptr<Token> lower = nullptr,
                                 std::shared_ptr<Token> upper = nullptr,
                                 std::shared_ptr<Token> bind = nullptr);
    virtual ~AbstractSimplePatternElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    inline bool isToken() const {
      return static_cast<bool>(min);
    }

    inline bool isIdentifier() const {
      return static_cast<bool>(max);
    }

    auto token() {
      return tok;
    }

    auto token() const {
      return tok;
    }

    auto identifier() {
      return ident;
    }

    auto identifier() const {
      return ident;
    }

  protected:
    std::shared_ptr<Token>                     tok;
    std::shared_ptr<AbstractIdentifierElement> ident;
};


class AbstractMatchElement: public AbstractPatternElement,
                            public AbstractCompoundMixin<AbstractMatchCaseElement> {
  public:
    AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                         std::shared_ptr<Token> lower = nullptr,
                         std::shared_ptr<Token> upper = nullptr,
                         std::shared_ptr<Token> bind  = nullptr);
    AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                         std::shared_ptr<AbstractMatchCaseElement> simple,
                         std::shared_ptr<Token> lower = nullptr,
                         std::shared_ptr<Token> upper = nullptr,
                         std::shared_ptr<Token> bind  = nullptr);
    AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                         const std::vector<std::shared_ptr<AbstractMatchCaseElement>> &multi,
                         std::shared_ptr<Token> lower = nullptr,
                         std::shared_ptr<Token> upper = nullptr,
                         std::shared_ptr<Token> bind  = nullptr);
    AbstractMatchElement(std::shared_ptr<AbstractIdentifierElement> discriminant,
                         iterator first, iterator end,
                         std::shared_ptr<Token> lower = nullptr,
                         std::shared_ptr<Token> upper = nullptr,
                         std::shared_ptr<Token> bind  = nullptr);

    virtual ~AbstractMatchElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<AbstractMatchCaseElement>);

    auto discriminant() {
      return key;
    }

    auto discriminant() const {
      return key;
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> key;
};


class AbstractCompoundPatternElement: public AbstractPatternElement,
                                      public AbstractCompoundMixin<AbstractPatternElement> {
  public:
    AbstractCompoundPatternElement(std::shared_ptr<Token> lower = nullptr,
                                   std::shared_ptr<Token> upper = nullptr,
                                   std::shared_ptr<Token> bind  = nullptr);
    AbstractCompoundPatternElement(std::shared_ptr<AbstractPatternElement> member,
                                   std::shared_ptr<Token> lower = nullptr,
                                   std::shared_ptr<Token> upper = nullptr,
                                   std::shared_ptr<Token> bind  = nullptr);
    AbstractCompoundPatternElement(const std::vector<std::shared_ptr<AbstractPatternElement>> &compound,
                                   std::shared_ptr<Token> lower = nullptr,
                                   std::shared_ptr<Token> upper = nullptr,
                                   std::shared_ptr<Token> bind  = nullptr);
    AbstractCompoundPatternElement(iterator start,
                                   iterator end,
                                   std::shared_ptr<Token> lower = nullptr,
                                   std::shared_ptr<Token> upper = nullptr,
                                   std::shared_ptr<Token> bind  = nullptr);
    virtual ~AbstractCompoundPatternElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<AbstractPatternElement>);
};


class AbstractPatternList: public AbstractElement,
                           public AbstractCompoundMixin<AbstractPatternElement> {
  public:
    AbstractPatternList();
    AbstractPatternList(std::shared_ptr<AbstractPatternElement> simple);
    AbstractPatternList(const std::vector<std::shared_ptr<AbstractPatternElement>> &compound);
    AbstractPatternList(iterator start, iterator end);
    virtual ~AbstractPatternList();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<AbstractPatternElement>);
};


class AbstractStorageElement: public AbstractElement {
  public:
    AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name);
    AbstractStorageElement(std::shared_ptr<AbstractIdentifierElement> name,
                           std::shared_ptr<AbstractIdentifierElement> type);
    virtual ~AbstractStorageElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    auto identifier() {
      return ident;
    }

    auto type() {
      return kind;
    }

    inline bool hasType() {
      return static_cast<bool>(kind);
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::shared_ptr<AbstractIdentifierElement> kind;
};


class AbstractStorageList: public AbstractElement,
                           public AbstractCompoundMixin<AbstractStorageElement> {
  public:
    AbstractStorageList();
    AbstractStorageList(std::shared_ptr<AbstractStorageElement> simple);
    AbstractStorageList(const std::vector<std::shared_ptr<AbstractStorageElement>> &compound);
    AbstractStorageList(iterator start, iterator end);
    virtual ~AbstractStorageList();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<AbstractStorageElement>);
};


class AbstractSexpr {
  public:
    AbstractSexpr();
    AbstractSexpr(std::shared_ptr<Token>);
    AbstractSexpr(std::shared_ptr<AbstractSexpr>);
    AbstractSexpr(std::shared_ptr<AbstractIdentifierElement>);

    std::ostream &print(std::ostream &os) const;
    std::ostream &debug(std::ostream &os) const;

    void setNext(std::shared_ptr<AbstractSexpr> next) {
      sexpr_next = next;
    }

    auto next() {
      return sexpr_next;
    }

    auto next() const {
      return sexpr_next;
    }

    auto token() {
      return tok_val;
    }

    auto token() const {
      return tok_val;
    }

    auto identifier() {
      return ident_val;
    }

    auto identifier() const {
      return ident_val;
    }

    auto sexpr() {
      return sexpr_val;
    }

    auto sexpr() const {
      return sexpr_val;
    }

    bool isToken() const {
      static_cast<bool>(tok_val);
    }

    bool isIdentifier() const {
      static_cast<bool>(ident_val);
    }

    bool isSexpr() const {
      static_cast<bool>(sexpr_val);
    }

    bool isEmpty() const {
      return !isToken() && !isSexpr() && !isIdentifier();
    }

  protected:
    std::shared_ptr<Token>                     tok_val;
    std::shared_ptr<AbstractSexpr>             sexpr_val;
    std::shared_ptr<AbstractSexpr>             sexpr_next;
    std::shared_ptr<AbstractIdentifierElement> ident_val;
};


class AbstractCodeSnippet: public AbstractElement {
  public:
    AbstractCodeSnippet(std::shared_ptr<AbstractSexpr>);
    virtual ~AbstractCodeSnippet();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    auto sexpr() {
      return expr;
    }

  protected:
    std::shared_ptr<AbstractSexpr> expr;
};


class AbstractRuleElement: public AbstractElement {
  public:
    AbstractRuleElement(
      std::shared_ptr<AbstractIdentifierElement>,
      std::shared_ptr<AbstractPatternList>,
      std::shared_ptr<AbstractStorageList> = nullptr,
      std::shared_ptr<AbstractCodeSnippet> = nullptr,
      std::shared_ptr<AbstractCodeSnippet> = nullptr,
      std::shared_ptr<AbstractCodeSnippet> = nullptr
    );
    virtual ~AbstractRuleElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    auto identifier() {
      return ident;
    }

    auto pattern() {
      return pat;
    }

    auto storage() {
      return store;
    }

    auto validation() {
      return validate;
    }

    auto encode() {
      return enc;
    }

    auto decode() {
      return dec;
    }

    inline bool hasStorage() {
      return static_cast<bool>(store);
    }

    inline bool hasValidation() {
      return static_cast<bool>(validate);
    }

    inline bool hasEncode() {
      return static_cast<bool>(enc);
    }

    inline bool hasDecode() {
      return static_cast<bool>(dec);
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::shared_ptr<AbstractPatternList>       pat;
    std::shared_ptr<AbstractStorageList>       store;
    std::shared_ptr<AbstractCodeSnippet>       validate;
    std::shared_ptr<AbstractCodeSnippet>       enc;
    std::shared_ptr<AbstractCodeSnippet>       dec;
};


class AbstractNamespaceElement: public AbstractElement,
                                public AbstractLookupMixin<AbstractRuleElement> {
  public:
    AbstractNamespaceElement();
    AbstractNamespaceElement(std::shared_ptr<AbstractIdentifierElement>, bool module = false);
    virtual ~AbstractNamespaceElement();

    virtual std::ostream &print(std::ostream &os) const;
    virtual std::ostream &debug(std::ostream &os) const;

    void add(std::shared_ptr<AbstractRuleElement>);
    void add(std::shared_ptr<AbstractAliasElement>);
    void add(std::shared_ptr<AbstractImportElement>);

    auto identifier() {
      return ident;
    }

    auto identifier() const {
      return ident;
    }

    auto aliasList() {
      return aliases;
    }

    auto aliasList() const {
      return aliases;
    }

    auto importList() {
      return imports;
    }

    auto importList() const {
      return imports;
    }

  protected:
    std::shared_ptr<AbstractIdentifierElement> ident;
    std::shared_ptr<AbstractAliasList>         aliases;
    std::shared_ptr<AbstractImportList>        imports;
};


class AbstractSyntaxTree: public AbstractLookupMixin<AbstractNamespaceElement> {
  public:
    AbstractSyntaxTree();

    std::ostream &print(std::ostream &os) const;
    std::ostream &debug(std::ostream &os) const;

    std::shared_ptr<AbstractNamespaceElement> currentNamespace();
    std::shared_ptr<AbstractNamespaceElement>
    addNamespace(std::shared_ptr<AbstractIdentifierElement>, bool emit);

    std::shared_ptr<AbstractAliasList> currentAliasList();
    void addAlias(std::shared_ptr<AbstractAliasElement>);

    std::shared_ptr<AbstractImportList> currentImportList();
    void addImport(std::shared_ptr<AbstractImportElement>);


  protected:
    std::shared_ptr<AbstractNamespaceElement> current;
};


  }
}
