#include <nyx/plugin.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <iostream>


using namespace nyx;
using namespace nyx::syntax;


static inline lua_State *asState(void *L) {
  return reinterpret_cast<lua_State *>(L);
}


Plugin::Plugin(void *lua, const std::string &language, const std::vector<std::string> &opts):
  lang(language),
  L(lua) {
  options.reserve(opts.size());
  options.insert(options.end(), opts.begin(), opts.end());
}


Plugin::~Plugin() {
  if(L) {
    lua_close(asState(L));
  }
}

std::unique_ptr<Plugin> Plugin::load(const Filesystem &fs,
                                     const std::string &lang,
                                     const std::vector<std::string> &opts) {
  std::string file("nyxlang-");

  if(auto path = fs.locate(file.append(lang).append(".lua"))) {
    if(auto lua = luaL_newstate()) {
      luaL_openlibs(lua);

      if(!luaL_dofile(lua, path->c_str())) {
        return std::unique_ptr<Plugin>(new Plugin(lua, lang, opts));
      }
      else {
        std::cerr << "Error loading " << lang << " plugin: " << lua_tostring(lua, -1) << std::endl;
        lua_close(lua);
      }
    }
    else {
      std::cerr << "Unable to create plugin context" << std::endl;
    }
  }
  else {
    std::cerr << "Unable to locate plugin for " << lang << std::endl;
  }

  return nullptr;
}


static std::string &toArray(const std::vector<std::string> &list, std::string &dst) {
  for(auto iter = list.begin(), end = list.end(); iter != end;) {
    dst.append(1, '"').append(*iter).append(1, '"');
    if(++iter != end) {
      dst.append(", ");
    }
  }

  return dst;
}


static const char *toSize(const std::string &type) {
  switch(type[1]) {
    case '8': return "1"; break;
    case '1': return "2"; break;
    case '3': return "4"; break;
    case '6': return "8"; break;
  }

  return "1"; // something is weird here
}


static void translateStage(std::string &script, const Stage &stage) {
  script.append("          {\n");
  if(stage.isPrimitive()) {
    script.append("            type = \"ExactMatch\",\n");
    script.append("            pattern = { ");
    for(auto val : stage.pattern()) {
      script.append(std::to_string(val)).append(", ");
    }
    script.append("},\n");
  }
  else if(stage.isWildcard()) {
    script.append("            type = \"PatternMatch\",\n");
    script.append("            pattern = {\n");
    script.append("              mask  = ");
    script.append(std::to_string(stage.wildcard().first)).append(",\n");
    script.append("              value = ");
    script.append(std::to_string(stage.wildcard().second)).append("\n");
    script.append("            },\n");
  }
  else if(stage.isCompound()) {
    script.append("            type = \"Group\",\n");
    for(auto ptr = stage.group(); ptr; ptr = ptr->next()) {
      translateStage(script, *ptr);
    }
  }
  else if(stage.isMatch()) {
    script.append("            type = \"Select\",\n");
    script.append("            pattern = {\n");
    script.append("              reference = \"").append(stage.reference()).append("\",\n");
    script.append("              keys = { ");
    for(auto &val : stage.match()) {
      script.append(std::to_string(val.first)).append(", ");
    }
    script.append("},\n");
    for(auto &val : stage.match()) {
      script.append("              [").append(std::to_string(val.first)).append("] = \"");
      script.append(val.second).append("\",\n");
    }
    script.append("            },\n");
  }
  else {
    auto &type = stage.reference();
    if(type == "u8"  || type == "i8"  ||
       type == "i16" || type == "u16" ||
       type == "i32" || type == "u32" ||
       type == "f32" || type == "f64" ||
       type == "i64" || type == "u64") {
      // read in an integer or float in machine byte order
      script.append("            type = \"Numeric\",\n");
      script.append("            pattern = {\n");
      script.append("              type = \"").append(type).append("\",\n");
      script.append("              size = ").append(toSize(type)).append(",\n");
      script.append("              order = \"machine\",\n");
      if(type != "f32" && type != "f64") {
        script.append("              signed = ").append(type[0] == 'i' ? "true" : "false").append(",\n");
      }
      script.append("            },\n");
    }
    else if(type == "i16l" || type == "u16l" ||
            type == "i32l" || type == "u32l" ||
            type == "f32l" || type == "f64l" ||
            type == "i64l" || type == "u64l") {
      // read in an integer or float in little endian byte order
      script.append("            type = \"Numeric\",\n");
      script.append("            pattern = {\n");
      script.append("              type = \"").append(type).append("\",\n");
      script.append("              size = ").append(toSize(type)).append(",\n");
      script.append("              order = \"little\",\n");
      if(type != "f32l" && type != "f64l") {
        script.append("              signed = ").append(type[0] == 'i' ? "true" : "false").append(",\n");
      }
      script.append("            },\n");
    }
    else if(type == "i16b" || type == "u16b" ||
            type == "i32b" || type == "u32b" ||
            type == "f32b" || type == "f64b" ||
            type == "i64b" || type == "u64b") {
      // read in an integer or float in big endian byte order
      script.append("            type = \"Numeric\",\n");
      script.append("            pattern = {\n");
      script.append("              type = \"").append(type).append("\",\n");
      script.append("              size = ").append(toSize(type)).append(",\n");
      script.append("              order = \"big\",\n");
      if(type != "f32b" && type != "f64b") {
        script.append("              signed = ").append(type[0] == 'i' ? "true" : "false").append(",\n");
      }
      script.append("            },\n");
    }
    else {
      script.append("            type = \"Identifier\",\n");
      script.append("            pattern = \"").append(stage.reference()).append("\",\n");
    }
  }

  if(isalpha(stage.minimum()[0])) {
    script.append("            minimum = \"").append(stage.minimum()).append("\",\n");
  }
  else {
    script.append("            minimum = ").append(stage.minimum()).append(",\n");
  }
  if(isalpha(stage.maximum()[0])) {
    script.append("            maximum = \"").append(stage.maximum()).append("\",\n");
  }
  else {
    script.append("            maximum = ").append(stage.maximum()).append(",\n");
  }
  if(stage.hasName()) {
    script.append("            ident = \"").append(stage.name()).append("\",\n");
  }
  script.append("          },\n");
}


static void translatePattern(std::string &script, const Pattern &pattern) {
  script.append("      pattern = {\n");
  for(auto &alt : pattern.alternates()) {
    translateStage(script, alt.pattern());
  }
  script.append("      },\n");
}


static void translateStorage(std::string &script, const Storage &storage) {
  script.append("      storage = {\n");
  for(auto &element : storage.elements()) {
    std::string type;
    script.append("        { name = \"").append(element.first).append("\", ");
    script.append(" type = {").append(toArray(element.second, type)).append("} },\n");
  }
  script.append("      },\n");
}


static void translateSexpr(std::string &script, const AbstractSexpr *sexpr) {
  while(sexpr) {
    if(sexpr->isToken()) {
      auto &token = *sexpr->token();
      script.append("{ value = \"").append(token.text()).append("\", type = \"");
      script.append(toString(token.lexeme())).append("\" ");
      switch(token.lexeme()) {
        case Lexeme::Assignment:
        case Lexeme::BitwiseAnd:
        case Lexeme::BitwiseNot:
        case Lexeme::BitwiseOr:
        case Lexeme::BitwiseXor:
        case Lexeme::CloseAngle:
        case Lexeme::Division:
        case Lexeme::Equality:
        case Lexeme::Minus:
        case Lexeme::Modulo:
        case Lexeme::OpenAngle:
        case Lexeme::Plus:
        case Lexeme::Times:
        case Lexeme::LeftShift:
        case Lexeme::RightShift:
          script.append(", mode = \"BinOp\" ");
        break;
      }
      script.append("}, ");
    }
    else if(sexpr->isIdentifier()) {
      std::string list;
      auto &ident = *sexpr->identifier();
      script.append("{ value = { ");
      for(auto &tok : ident) {
        script.append("\"").append(tok->text()).append("\", ");
      }
      script.append("}, type = \"Identifier\" }, ");
    }
    else if(sexpr->isSexpr()) {
      script.append("{ value = ");
      translateSexpr(script, sexpr->sexpr().get());
      script.append("type = \"Sexpr\" }, ");
    }

    sexpr = sexpr->next().get();
  }
}

static void translateCode(std::string &script, const std::string &name, const Code &code) {
  script.append("      ").append(name).append(" = { ");
  translateSexpr(script, code.sexpr().get());
  script.append("},\n");
}


static void translateEncode(std::string &script, const Code &code) {
  translateCode(script, "encode", code);
}


static void translateDecode(std::string &script, const Code &code) {
  translateCode(script, "decode", code);
}


static void translateValidation(std::string &script, const Code &code) {
  translateCode(script, "validate", code);
}


static void translateRule(std::string &script, const Rule &rule) {
  script.append("    {\n");
  script.append("      name = \"").append(rule.name()).append("\",\n");
  translatePattern(script, rule.pattern());
  if(rule.hasStorage()) {
    translateStorage(script, rule.storage());
  }
  if(rule.hasEncode()) {
    translateEncode(script, rule.encode());
  }
  if(rule.hasDecode()) {
    translateDecode(script, rule.decode());
  }
  if(rule.hasValidation()) {
    translateValidation(script, rule.validation());
  }
  script.append("    },\n");
}


static void translateImport(std::string &script, const Import &import) {
  std::string moduleName;
  toArray(import.module(), moduleName);

  script.append("      {\n");
  script.append("        [\"namespace\"] = {").append(moduleName).append("},\n");
  script.append("        [\"module\"] = {").append(moduleName).append("},\n");
  script.append("      },\n");
}


static void translateNamespace(std::string &script, const Namespace &ns) {
  std::string moduleName;
  toArray(ns.parts(), moduleName);

  script.append("  {\n");
  script.append("    [\"namespace\"] = {").append(moduleName).append("},\n");
  script.append("    [\"module\"] = {").append(moduleName).append("},\n");
  script.append("    [\"imports\"] = {\n");
  for(auto &import : ns.imports()) {
    translateImport(script, import);
  }
  script.append("    },\n");
  for(auto &rule : ns.rules()) {
    translateRule(script, rule);
  }
  script.append("  },\n");
}


static void translateOptions(std::string &script, const std::vector<std::string> &options) {
  for(auto iter = options.begin(), end = options.end(); iter != end;) {
    auto idx = iter->find('=');

    if(idx == std::string::npos) {
      script.append("    [\"").append(*iter).append("\"] = true");
    }
    else {
      script.append("    [\"").append(iter->substr(0, idx)).append("\"] = \"");
      script.append(iter->substr(idx + 1)).append("\"");
    }

    script.append(++iter != end ? ",\n" : "\n");
  }
}

int Plugin::execute(Plan &plan) {
  std::string script("-- BEGIN AUTOGENERATED DATA\n");
  script.reserve(1024 * 1024); // a full megabyte
  script.append("plan = {\n");
  script.append("  options = {\n");
  translateOptions(script, options);
  script.append("  },\n");

  for(auto &ns : plan.namespaces()) {
    translateNamespace(script, ns);
  }

  script.append("}\n");

  std::cerr << script << std::endl;

  script.append(
    "-- END AUTOGENERATED DATA\n\n"
    // add in an extra function to aid in output
    "io.mkdir = function(path) os.execute(\"mkdir -p '\"..path..\"'\"); end\n"
    "-- temp function useful for debugging\n"
    "function dump(o)\n"
    "   if type(o) == 'table' then\n"
    "      local s = '{ '\n"
    "      for k,v in pairs(o) do\n"
    "         if type(k) ~= 'number' then k = '\"'..k..'\"' end\n"
    "         s = s .. '['..k..'] = ' .. dump(v) .. ', '\n"
    "      end\n"
    "      return s .. '}'\n"
    "   else\n"
    "      return tostring(o)\n"
    "   end\n"
    "end\n"
    "-- useful table/array manipulation\n"
    "function table.slice(tbl, first, last, step)\n"
    "  local sliced = {}\n"
    "  for i = first or 1, last or #tbl, step or 1 do\n"
    "    sliced[#sliced+1] = tbl[i]\n"
    "  end\n"
    "  return sliced\n"
    "end\n"
    "-- actually execute the plugin\n"
    "execute(plan)\n"
  );

  //std::cerr << script << std::endl;

  if(luaL_dostring(asState(L), script.c_str())) {
    std::cerr << "Error running " << lang << " plugin: " << lua_tostring(asState(L), -1) << std::endl;
  }
  else if(!lua_tonumber(asState(L), -1)) {
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

