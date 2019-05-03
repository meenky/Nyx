-- nyx C++ plugin

function createNamespace(ns, root)
  local dir = root .. table.concat(table.slice(ns, 1, #ns - 1), '/')

  if io.mkdir(dir) == 0 then
    io.write("Faliure to create output directory: '", dir, "'\n")
    return nil
  end

  local filebase = dir .. '/' .. ns[#ns]
  local header = io.open(filebase .. '.h', "w")
  if header == nil then
    io.write("Faliure to create namespace header: '", filebase, ".h'\n")
    return nil, nil
  end

  local code = io.open(filebase .. '.cpp', "w")
  if code == nil then
    io.write("Faliure to create namespace implementation: '", filebase, ".cpp'\n")
    header:close()
    return nil, nil
  end

  header:write("#pragma once\n\n")
  code:write("#include \"", table.concat(ns, '/'), ".h\"\n",
             "\n\n",
             "using namespace ", table.concat(ns, '::'), ";\n")
  return header, code
end


function includeImports(header, imports)
  if imports ~= nil then
    for i = 1, #imports do
      header:write('#include "', table.concat(imports[i].namespace, '/'), '.h"\n')
    end

    if #imports ~= 0 then
      header:write("\n")
    end
  end

  header:write("#include \"nyx.h\"\n",
               "#include <string>\n",
               "#include <vector>\n",
               "#include <cstddef>\n",
               "#include <cstdint>\n",
               "\n\n",
							 "namespace std {\n\n\n",
							 "typedef ptrdiff_t ssize_t;\n\n\n",
							 "}\n")
end


function generateUsing(code, imports)
  if imports ~= nil then
    for i = 1, #imports do
      code:write('using namespace ', table.concat(imports[i].namespace, '::'), ';\n')
    end
  end

  code:write('\n\n')
end


function openNamespace(header, ns)
  local indent = ''

  for i = 1, #ns do
    header:write(indent, "namespace ", ns[i], " {\n")
    indent = indent .. '  '
  end

  header:write('\n\n')
end


function closeNamespace(header, ns)
  for i = #ns, 1, -1 do
    for indent = 1, i - 1 do
      header:write('  ')
    end
    header:write('}\n')
  end
end

TypeMap = {
  u8   = 'std::uint8_t',
  i8   = 'std::int8_t',
  u16  = 'std::uint16_t',
  u16l = 'std::uint16_t',
  u16b = 'std::uint16_t',
  i16  = 'std::int16_t',
  i16l = 'std::int16_t',
  i16b = 'std::int16_t',
  u32  = 'std::uint32_t',
  u32l = 'std::uint32_t',
  u32b = 'std::uint32_t',
  i32  = 'std::int32_t',
  i32l = 'std::int32_t',
  i32b = 'std::int32_t',
  f32  = 'float',
  f32l = 'float',
  f32b = 'float',
  u64  = 'std::uint32_t',
  u64l = 'std::uint32_t',
  u64b = 'std::uint32_t',
  i64  = 'std::int32_t',
  i64l = 'std::int32_t',
  i64b = 'std::int32_t',
  f64  = 'double',
  f64l = 'double',
  f64b = 'double',
  string = 'std::string',
};


function findInPattern(name, pattern)
  for i = 1, #pattern do
    local pat = pattern[i]

    if pat["type"] == 'Group' then
      local t = findInPattern(name, pat)
      if t ~= nil then
        return t
      end
    elseif pat["type"] == 'Identifier' then
      if pat.ident == name then
        if TypeMap[pat.pattern] ~= nil then
          return TypeMap[pat.pattern]
        else
          return pat.pattern
        end
      end
    elseif pat["type"] == 'Numeric' then
      if pat.ident == name then
        if TypeMap[pat.pattern["type"]] ~= nil then
          return TypeMap[pat.pattern["type"]]
        else
          return pat.pattern["type"]
        end
      end
    end
  end

  return nil
end


function resolveType(storage, pattern)
  local tbl = storage["type"]

  if #tbl > 1 then
    return table.concat(tbl, '::')
  elseif #tbl == 1 then
    if tbl[1] == 'vector' then
      return 'std::vector<' .. findInPattern(storage.name, pattern) .. '>'
    elseif TypeMap[tbl[1]] ~= nil then
      return TypeMap[tbl[1]]
    end

    return tbl[1]
  else
    if TypeMap[storage.name] ~= nil then
      return TypeMap[storage.name]
    end

    return storage.name
  end

  return nil
end


function generateRuleStorage(header, storage, pattern)
  local map = {}
  header:write("\n\n");

  for i = 1, #storage do
    local entry = storage[i]
    local kind = resolveType(entry, pattern)

    header:write('    ', kind, ' ', entry.name, ';\n')
    map[entry.name] = { raw = entry["type"], resolved = kind }
  end

  return map
end


function sexprToCpp(code, decode)
  if decode["type"] == 'Sexpr' then
    local op = decode.value

    if op.mode ~= nil and op.mode == "BinOp" then
      sexprToCpp(code, decode[1])
      code:write(' ', op.value, ' ')
      if decode[2]["type"] == 'Sexpr' then
        code:write('(')
        sexprToCpp(code, decode[2])
        code:write(')')
      else
        sexprToCpp(code, decode[2])
      end
    elseif op["type"] == 'Identifier' then
      if #op.value == 1 and TypeMap[op.value[1]] ~= nil then
        code:write("static_cast<", TypeMap[op.value[1]], ">")
      else
        if #op.value == 1 and op.value[1] == 'lambda' then
          code:write('[&](')
          local args = decode[1]

          code:write('auto ', args.value.value[1])
          for i = 1, #args do
            code:write(', auto ', args[i].value[1])
          end

          code:write(') {\n')
          for i = 2, #decode do
            sexprToCpp(code, decode[i])
            code:write(";\n")
          end
          code:write('}')
          return
        else
          code:write(table.concat(op.value, '.'))
        end
      end
      code:write('(')
      for i = 1, #decode - 1, 1 do
        sexprToCpp(code, decode[i])
        code:write(", ")
      end
      sexprToCpp(code, decode[#decode])
      code:write(')')
    else
      print("Unhandled:",dump(decode.value))
    end
  elseif decode["type"] == 'Identifier' then
    code:write(table.concat(decode.value, '.'))
  elseif decode["type"] == 'BinaryLiteral' or
         decode["type"] == 'OctalLiteral' or
         decode["type"] == 'DecimalLiteral' or
         decode["type"] == 'HexadecimalLiteral' then
    code:write(decode.value)
  elseif decode["type"] == 'StringLiteral' then
    code:write('"', decode.value, '"')

  --    local sub = decode[2]["type"]
  --    if sub == 'Sexpr' then
  --      generateDecode(code, decode[2])
  --    elseif sub == 'DecimalLiteral' then
  --      code:write("  ", decode[1].value)
  --    end
  --  elseif op["type"] == 'Identifier' then
  --    if #op.value == 1 and TypeMap[op.value[1]] ~= nil then
  --      code:write("static_cast<", TypeMap[op.value[1]], ">(", decode[1].value, ")")
  --    else
  --      print("While:", dump(op))
  --    end
  --  end
  else
    print("Invalid Sexpr:",dump(decode.value))
		for i = 1, #decode do
    print("             ",dump(decode[i]))

		end
  end
end


function generateConsumeStage(code, stage, storage, final)
  --if stage["type"] == 'Identifier' and stage.ident ~= nil then
  --  if storage[stage.ident] ~= nil then
  --    local raw = storage[stage.ident].raw

  --    if #raw == 1 then
  --      if raw[1] == 'string' or raw[1] == 'vector' then
  --        if stage.minimum == stage.maximum then
  --          code:write("  ", stage.ident, ".assign(&_raw__[_idx__], &_raw__[_idx__ + ", stage.maximum, "]);\n",
  --                     "  _idx__ += ", stage.maximum, ";\n")
  --          return
  --        else
  --          code:write("  ", stage.ident, ".clear();\n")
  --        end
  --      elseif TypeMap[stage.pattern] ~= nil then
  --        if stage.pattern == 'i32b' then
  --          code:write("  ", stage.ident,
  --                     " = (static_cast<std::int32_t>(_raw__[_idx__]) << 24) |",
  --                       " (static_cast<std::int32_t>(_raw__[_idx__ + 1]) << 16) |",
  --                       " (static_cast<std::int32_t>(_raw__[_idx__ + 2]) << 8) |",
  --                        " static_cast<std::int32_t>(_raw__[_idx__ + 3]);\n",
  --                     "  _idx__ += 4;\n")
  --          return
  --        else
  --          code:write("  ", stage.ident, " = 0;\n")
  --        end
  --      end
  --    end
  --  elseif stage.minimum == stage.maximum and stage.maximum == 1 then
  --    if TypeMap[stage.pattern] ~= nil then
  --      if stage.pattern == 'i32b' then
  --        code:write("  ", stage.ident,
  --                   " = (static_cast<std::int32_t>(_raw__[_idx__]) << 24) |",
  --                     " (static_cast<std::int32_t>(_raw__[_idx__ + 1]) << 16) |",
  --                     " (static_cast<std::int32_t>(_raw__[_idx__ + 2]) << 8) |",
  --                      " static_cast<std::int32_t>(_raw__[_idx__ + 3]);\n",
  --                   "  _idx__ += 4;\n")
  --        return
  --      else
  --        code:write("  ", TypeMap[stage.pattern], " ", stage.ident, " = 0;\n")
  --      end
  --    end
  --  end
  --end

  if stage["type"] == 'Identifier' or
		stage["type"] == 'PatternMatch' or
		stage["type"] == 'Numeric' then
    if stage.ident ~= nil and storage[stage.ident] ~= nil then
      local raw = storage[stage.ident].raw

      if raw ~= nil and #raw == 1 then
        if raw[1] == 'string' or raw[1] == 'vector' then
          code:write("    ", stage.ident, ".clear();\n")
				elseif stage["type"] == 'Numeric' then
          code:write("    ", stage.ident, " = 0;\n")
        end
      end
		elseif stage.ident ~= nil and storage[stage.ident] == nil then
			if stage["type"] == 'Numeric' then
        code:write("    ", TypeMap[stage.pattern["type"]], ' ', stage.ident, " = 0;\n")
			end
    end
  end

  if((type(stage.maximum) == "number" and stage.maximum > 0) or
      type(stage.maximum) == "string") then
    code:write("    for(_rep__ = 0; _rep__ < ", stage.maximum, "; ++_rep__) {\n")
  else
    code:write("    for(_rep__ = 0; true; ++_rep__) {\n")
  end

  --if stage["type"] == 'Identifier' then
  --  if stage.ident ~= nil then
  --    if stage.pattern == 'u8' then

  --    else
  --    end
  --  end
  --end
  if stage["type"] == 'ExactMatch' then
    local arr = stage.pattern
    code:write("      if(_max__ - _idx__ < ", #arr, " ||\n")
    for i = 1, #arr - 1, 1 do
      code:write("         _raw__[_idx__ + ", i - 1, "] != ", arr[i], " ||\n")
    end
    code:write("         _raw__[_idx__ + ", #arr - 1, "] != ", arr[#arr], ") {\n",
               "        break;\n",
               "      }\n",
               "      else {\n")
    code:write("        _idx__ += ", #arr, ";\n",
               "      }\n")
  elseif stage["type"] == 'PatternMatch' then
    local pat = stage.pattern
    code:write("      if(_max__ - _idx__ < 1 ||\n",
               "         (_raw__[_idx__] & ", pat.mask, ") != ", pat.value, ") {\n",
               "        break;\n",
               "      }\n",
               "      else {\n")
    if stage.ident ~= nil then
      if stage.maximum ~= 1 then
        code:write("        ", stage.ident, ".append(1, static_cast<char>(_raw__[_idx__]));\n")
      else
        code:write("        ", stage.ident, " = _raw__[_idx__];\n")
      end
    end
    code:write("        ++_idx__;\n",
               "      }\n")
  elseif stage["type"] == 'Numeric' then
    local pat = stage.pattern
    code:write("      if(_max__ - _idx__ < ", pat.size, ") {\n",
               "        break;\n",
               "      }\n",
               "      else {\n")
    if stage.maximum ~= 1 then
      code:write("        ", TypeMap[pat["type"]], " _tmp__;\n")
      if pat.order == 'big' then
        code:write("        for(int i = 0; i < ", pat.size, "; ++i) {\n",
                   "          _tmp__ = (_tmp__ << 8) | _raw__[_idx__ + i];\n",
                   "        }\n",
                   "        ", stage.ident, ".emplace_back(_tmp__);\n")
      elseif pat.order == 'llittle' then
        code:write("        for(int i = 0; i < ", pat.size, "; ++i) {\n",
                   "          _tmp__ |= _raw__[_idx__ + i] << (i * 8);\n",
                   "        }\n",
                   "        ", stage.ident, ".emplace_back(_tmp__);\n")
      else
        code:write("        _tmp__ = *reinterpret_cast<const ", TypeMap[pat["type"]],
				                    " *>(&_raw__[_idx__]);\n",
                   "        ", stage.ident, ".emplace_back(_tmp__);\n")
      end
    else
      if pat.order == 'big' then
        code:write("        for(int i = 0; i < ", pat.size, "; ++i) {\n",
                   "          ", stage.ident, " = (", stage.ident, " << 8) | _raw__[_idx__ + i];\n",
                   "        }\n")
      elseif pat.order == 'little' then
        code:write("        for(int i = 0; i < ", pat.size, "; ++i) {\n",
                   "          ", stage.ident, " |= _raw__[_idx__ + i] << (i * 8);\n",
                   "        }\n")
      else
        code:write("        ", stage.ident,
                   " = *reinterpret_cast<", TypeMap[pat["type"]], " *>(&_raw__[_idx__]);\n")
      end
    end
    code:write("        _idx__ += ", pat.size, ";\n",
               "      }\n")
  elseif stage["type"] == 'Identifier' then
    if stage.ident ~= nil then
      if stage.maximum ~= 1 then
        code:write("      ", stage.ident, ".resize(", stage.ident, ".size() + 1);\n",
                   "      auto result = ", stage.ident, ".back().consume(&_raw__[_idx__], _max__ - _idx__);\n",
                   "      if(result < 0) {\n",
                   "        ", stage.ident, ".resize(", stage.ident, ".size() - 1);\n",
                   "        break;\n",
                   "      }\n",
                   "      _idx__ += result;\n")
      else
        code:write("      auto result = ", stage.ident, ".consume(&_raw__[_idx__], _max__ - _idx__);\n",
                   "      if(result < 0) {\n",
                   "        break;\n",
                   "      }\n",
                   "      _idx__ += result;\n")
      end
    else
      code:write("      ", stage.pattern, " _tmp__;\n",
                 "      auto result = _tmp__.consume(&_raw__[_idx__], _max__ - _idx__);\n",
                 "      if(result < 0) {\n",
                 "        break;\n",
                 "      }\n",
                 "      _idx__ += result;\n")
    end
  else
    io.write("Unhandled stage: ", dump(stage),'\n')
  end

  code:write("    }\n")
  if type(stage.minimum) == "string" or stage.minimum > 0 then
    code:write("    if(_rep__ < ", stage.minimum, ") {\n")
    code:write("      break;\n")
    code:write("    }\n")
  end
  code:write("\n")
end


function generateConsumeAlternate(code, pattern, storage, decode, validate, final)
  code:write("  do {\n")

  if pattern["type"] == "Group" then
    for i = 1, #pattern, 1 do
      generateConsumeStage(code, pattern[i], storage, decode)
    end
  else
    generateConsumeStage(code, pattern, storage, decode)
  end

  if decode ~= nil then
    for i = 1, #decode do
      sexprToCpp(code, decode[i])
      code:write(";\n")
    end
    code:write("\n")
  end

	if validate ~= nil then
		code:write("    if(!(")
    sexprToCpp(code, validate[1])
		code:write(")) {\n",
		           "      break;\n",
							 "    }\n\n")
	end

  code:write("    return _idx__;\n")
  code:write("  } while(false);\n\n")
end


function generateEmitAlternate(code, pattern, storage)
end


function generateRuleClass(header, code, rule, ns)
  header:write("class ", rule.name, "{\n",
               "  public:\n",
               "    std::ssize_t consume(const std::uint8_t *, std::size_t);\n",
               "    std::size_t size() const;\n",
               "    std::ssize_t emit(std::uint8_t *, std::size_t) const;\n")
  local storage = {}
  if rule.storage ~= nil then
    storage = generateRuleStorage(header, rule.storage, rule.pattern)
  end
  header:write("};\n\n\n")

  local namespace = table.concat(ns, '::')

  code:write("std::ssize_t ", rule.name,
	           "::consume(const std::uint8_t *_raw__, std::size_t _max__) {\n",
             "  int _rep__;\n",
             "  std::ssize_t _idx__ = 0;\n")
  if rule.decode ~= nil then
    code:write("  std::ssize_t _start__;\n")
  end
  code:write("\n")

  for i = 1, #rule.pattern - 1, 1 do
    generateConsumeAlternate(code, rule.pattern[i], storage, rule.decode, rule.validate)
  end
  generateConsumeAlternate(code, rule.pattern[#rule.pattern], storage, rule.decode, rule.validate)
  code:write("  return -1;\n}\n\n\n");

	code:write("std::size_t ", rule.name, "::size() const {\n")
  code:write("  return 0;\n}\n\n\n");

  code:write("std::ssize_t ", rule.name,
             "::emit(std::uint8_t *_raw__, std::size_t _max__) const {\n")
  for i = 1, #rule.pattern do
    generateEmitAlternate(code, rule.pattern[i], storage)
  end
  code:write("  return -1;\n}\n\n\n");
end


function execute(plan)
  local root = ''

  if plan.options.outdir ~= nil then
    root = plan.options.outdir .. '/'
  end

  for i = 1, #plan do
    local namespace = plan[i]
    local header, code = createNamespace(namespace.namespace, root)

    if header == nil then
      return -1
    elseif code == nil then
      header:close()
      return -1
    end

    includeImports(header, namespace.imports)
    openNamespace(header, namespace.namespace)

    generateUsing(code, namespace.imports)

    for j = 1, #namespace do
      generateRuleClass(header, code, namespace[j], namespace.namespace)
    end

    closeNamespace(header, namespace.namespace)
    header:close()
    code:close()
  end

  return 0
end

