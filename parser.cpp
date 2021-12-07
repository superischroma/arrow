#include <string>

#include "parser.h"
#include "assembler.h"

namespace arrow
{
    namespace evaluation_states
    {
        std::string name(evaluation_state es)
        {
            switch (es)
            {
                case FOUND: return "FOUND";
                case NEUTRAL: return "NEUTRAL";
                case SYNTAX_ERROR: return "SYNTAX_ERROR";
                default: return "UNKNOWN_EVAL_STATE_" + std::to_string(es);
            }
        }
    }

    namespace operating_systems
    {
        std::string name(operating_system os)
        {
            switch (os)
            {
                case WINDOWS: return "WINDOWS";
                case MAC: return "MAC";
                case LINUX: return "LINUX";
                default: return "UNKNOWN_OS_" + std::to_string(os);
            }
        }
    }

    bool check_eof(token*& t, bool msg)
    {
        if (t == nullptr)
        {
            if (msg) arrow::err("unexpectedly reached end of file");
            return true;
        }
        return false;
    }

    bool check_eof(token*& t)
    {
        return check_eof(t, true);
    }

    parser::parser(token* start, operating_system os)
    {
        current = start;
        current_scope = nullptr;
        this->os = os;
        literal_counter = 0;
        arguments = 0;
    }

    bool parser::good()
    {
        return current != nullptr;
    }

    evaluation_state parser::label_start(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (check_eof(t->next, false)) return evaluation_states::NEUTRAL;
        if (t->next->content != "{") return evaluation_states::NEUTRAL;
        if (symbols.find(t->content) != symbols.end())
        {
            arrow::err("symbol '" + t->content + "' is already defined", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        symbol& label = symbols[t->content] = { t, current_scope, 0 };
        t = t->next->next;
        current_scope = &label;
        return evaluation_states::FOUND;
    }

    evaluation_state parser::label_start()
    {
        if (current != nullptr)
            std::cout << current->content << std::endl;
        token*& c = current;
        return label_start(c);
    }

    evaluation_state parser::label_end(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "}") return evaluation_states::NEUTRAL;
        if (current_scope == nullptr)
        {
            arrow::err("unexpected right curly brace", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        t = t->next;
        current_scope = current_scope->scope; // scope out
        return evaluation_states::FOUND;
    }

    evaluation_state parser::label_end()
    {
        token*& c = current;
        return label_end(c);
    }

    evaluation_state parser::define(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "def") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next))
            return evaluation_states::SYNTAX_ERROR;
        if (t->type != token_types::IDENTIFIER)
        {
            arrow::err("identifier expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (symbols.find(t->content) != symbols.end())
        {
            arrow::err("symbol '" + t->content + "' is already defined", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        symbols[t->content] = { t, current_scope, 0 };
        as.external(t->content);
        t = t->next;
        return evaluation_states::FOUND;
    }

    evaluation_state parser::define()
    {
        token*& c = current;
        return define(c);
    }

    evaluation_state parser::reference(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "ref") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to identifier and check eof
        if (t->content == "*")
        {
            arrow::err("dereference operator not allowed here", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (t->type != token_types::IDENTIFIER)
        {
            arrow::err("identifier expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        token* ref_token = t;
        std::string& identifier = t->content;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to comma
        if (t->content != ",")
        {
            arrow::err("comma expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to allocation quantity
        if (os == operating_systems::WINDOWS)
        {
            as.sr(current_scope->t->content)->alloc_delta(32);
            as.external("GetProcessHeap");
            as.external("HeapAlloc");
            as.instruct(current_scope->t->content, "call GetProcessHeap");
            as.instruct(current_scope->t->content, "mov rcx, rax");
            as.instruct(current_scope->t->content, "mov rdx, 8");
            evaluation_state e = evaluate(t, nullptr, false);
            if (e == evaluation_states::SYNTAX_ERROR)
                return evaluation_states::SYNTAX_ERROR;
            as.instruct(current_scope->t->content, "mov r8, rax");
            as.instruct(current_scope->t->content, "call HeapAlloc");
            int& mutilator = as.sr(current_scope->t->content)->offset_mutilator;
            symbol& sym = symbols[identifier] = { ref_token, current_scope, mutilator -= 8 };
            as.instruct(current_scope->t->content, "mov qword [rbp + " + std::to_string(mutilator) + "], rax");
            return evaluation_states::FOUND;
        }
        arrow::err("unsupported operation for output operating system " + operating_systems::name(os));
        return evaluation_states::SYNTAX_ERROR;
    }

    evaluation_state parser::reference()
    {
        token*& c = current;
        return reference(c);
    }

    evaluation_state parser::copy(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "copy") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to identifier and check eof
        if (t->content == "*")
        {
            arrow::err("dereference operator not allowed here", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (t->type != token_types::IDENTIFIER)
        {
            arrow::err("identifier expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        std::string& identifier = t->content;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to comma
        if (t->content != ",")
        {
            arrow::err("comma expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to value to copy
        evaluation_state e = evaluate(t, nullptr, false);
        if (e == evaluation_states::SYNTAX_ERROR)
            return evaluation_states::SYNTAX_ERROR;
        if (symbols.find(identifier) == symbols.end())
        {
            arrow::err("symbol '" + identifier + "' is not defined", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        symbol& sym = symbols[identifier];
        as.instruct(current_scope->t->content, "mov rbx, qword [rbp + " + std::to_string(sym.offset) + ']');
        as.instruct(current_scope->t->content, "mov qword [rbx], rax");
        return evaluation_states::FOUND;
    }

    evaluation_state parser::copy()
    {
        token*& c = current;
        return copy(c);
    }

    evaluation_state parser::add(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "add") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to identifier and check eof
        evaluation_state e_left = evaluate(t, nullptr, false, true);
        if (t->content != ",")
        {
            arrow::err("comma expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to value to copy
        as.instruct(current_scope->t->content, "mov rbx, rax");
        evaluation_state e_right = evaluate(t, nullptr, false);
        if (e_right == evaluation_states::SYNTAX_ERROR)
            return evaluation_states::SYNTAX_ERROR;
        as.instruct(current_scope->t->content, "add [rbx], rax");
        return evaluation_states::FOUND;
    }

    evaluation_state parser::add()
    {
        token*& c = current;
        return add(c);
    }

    evaluation_state parser::set(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "set") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to identifier and check eof
        if (t->content == "*")
        {
            arrow::err("dereference operator not allowed here", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (t->type != token_types::IDENTIFIER)
        {
            arrow::err("identifier expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        std::string& identifier = t->content;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to comma
        if (t->content != ",")
        {
            arrow::err("comma expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR; // skip to value to copy
        evaluation_state e = evaluate(t, nullptr, false);
        if (e == evaluation_states::SYNTAX_ERROR)
            return evaluation_states::SYNTAX_ERROR;
        if (symbols.find(identifier) == symbols.end())
        {
            arrow::err("symbol '" + identifier + "' is not defined", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        symbol& sym = symbols[identifier];
        as.instruct(current_scope->t->content, "mov qword [rbp + " + std::to_string(sym.offset) + "], rax");
        return evaluation_states::FOUND;
    }

    evaluation_state parser::set()
    {
        token*& c = current;
        return set(c);
    }

    evaluation_state parser::pass(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "pass") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR;
        token* c = t;
        local_push_stack.push(c);
        evaluation_state e = evaluate(t, nullptr, true);
        if (e == evaluation_states::NEUTRAL)
        {
            arrow::err("expression expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (e == evaluation_states::SYNTAX_ERROR) return evaluation_states::SYNTAX_ERROR;
        return evaluation_states::FOUND;
    }

    evaluation_state parser::pass()
    {
        token*& c = current;
        return pass(c);
    }

    evaluation_state parser::pull(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "pull") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR;
        evaluation_state e = evaluate(t, nullptr, false, true);
        as.instruct(current_scope->t->content, "mov rbx, qword [rbp + " + std::to_string(((as.sr(current_scope->t->content)->pulls++) * 8) + 16) + ']');
        as.instruct(current_scope->t->content, "mov [rax], rbx");
        return evaluation_states::FOUND;
    }

    evaluation_state parser::pull()
    {
        token*& c = current;
        return pull(c);
    }

    evaluation_state parser::ret(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "ret") return evaluation_states::NEUTRAL;
        evaluation_state e = evaluate(t = t->next, nullptr);
        as.instruct(current_scope->t->content, "push rax");
        as.sr(current_scope->t->content)->preserve_ret_value = true;
        return e;
    }

    evaluation_state parser::ret()
    {
        token*& c = current;
        return ret(c);
    }

    evaluation_state parser::call(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "call") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next))
            return evaluation_states::SYNTAX_ERROR;
        if (t->type != token_types::IDENTIFIER)
        {
            arrow::err("identifier expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (symbols.find(t->content) == symbols.end())
        {
            arrow::err("symbol '" + t->content + "' is not defined", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        std::string& identifier = t->content;
        as.sr(current_scope->t->content)->alloc_delta(32);
        symbol& sym = symbols[t->content];
        while (!local_push_stack.empty())
        {
            token* et = local_push_stack.top();
            evaluation_state e = evaluate(et, nullptr, false);
            if (local_push_stack.size() > X64_CALLING_CONVENTION_REGISTERS.size())
                as.instruct(current_scope->t->content, "push rax");
            else
                as.instruct(current_scope->t->content, "mov " + X64_CALLING_CONVENTION_REGISTERS[local_push_stack.size() - 1] + ", rax");
            local_push_stack.pop();
        }
        as.instruct(current_scope->t->content, "call " + identifier);
        t = t->next;
        return evaluation_states::FOUND;
    }

    evaluation_state parser::call()
    {
        token*& c = current;
        return call(c);
    }

    evaluation_state parser::del(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "del") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next))
            return evaluation_states::SYNTAX_ERROR;
        if (t->type != token_types::IDENTIFIER)
        {
            arrow::err("identifier expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (symbols.find(t->content) == symbols.end())
        {
            arrow::err("symbol '" + t->content + "' is not defined", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        as.sr(current_scope->t->content)->alloc_delta(32);
        symbol& sym = symbols[t->content];
        if (os == operating_systems::WINDOWS)
        {
            as.external("GetProcessHeap");
            as.external("HeapFree");
            as.instruct(current_scope->t->content, "call GetProcessHeap");
            as.instruct(current_scope->t->content, "mov rcx, rax");
            as.instruct(current_scope->t->content, "mov rdx, 0");
            as.instruct(current_scope->t->content, "mov r8, qword [rbp + " + std::to_string(sym.offset) + ']');
            as.instruct(current_scope->t->content, "call HeapFree");
        }
        else
        {
            arrow::err("unsupported operation for output operating system " + operating_systems::name(os));
            return evaluation_states::SYNTAX_ERROR;
        }
        symbols.erase(t->content);
        t = t->next;
        return evaluation_states::FOUND;
    }

    evaluation_state parser::del()
    {
        token*& c = current;
        return del(c);
    }

    evaluation_state parser::evaluate(token*& t, symbol* dest, bool validate, bool mutilating)
    {
        std::string location = dest != nullptr ? "[rbp + " + std::to_string(dest->offset) + ']' : "rax";
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content == "*")
        {
            if (check_eof(t = t->next))
                return evaluation_states::SYNTAX_ERROR;
            if (t->type != token_types::IDENTIFIER)
            {
                arrow::err("attempt to dereference non-symbol", t->line);
                return evaluation_states::SYNTAX_ERROR;
            }
            if (symbols.find(t->content) == symbols.end())
            {
                arrow::err("symbol '" + t->content + "' is not defined", t->line);
                return evaluation_states::SYNTAX_ERROR;
            }
            symbol& sym = symbols[t->content];
            if (!validate)
            {
                if (location == "rax")
                {
                    as.instruct(current_scope->t->content, "mov rax, qword [rbp + " + std::to_string(sym.offset) + ']');
                    if (!mutilating)
                        as.instruct(current_scope->t->content, "mov rax, qword [rax]");
                }
                else
                {
                    as.instruct(current_scope->t->content, "push rax");
                    as.instruct(current_scope->t->content, "mov rax, qword [rbp + " + std::to_string(sym.offset) + ']');
                    if (!mutilating)
                        as.instruct(current_scope->t->content, "mov rax, qword [rax]");
                    as.instruct(current_scope->t->content, "mov qword " + location + ", rax");
                    as.instruct(current_scope->t->content, "pop rax");
                }
            }
            t = t->next;
            return evaluation_states::FOUND;
        }
        if (!validate)
        {
            switch (t->type)
            {
                case token_types::NUMERIC_LITERAL:
                {
                    as.instruct(current_scope->t->content, "mov " + location + ", " + t->content);
                    break;
                }
                case token_types::STRING_LITERAL:
                {
                    as << arrow::data << 'L' + std::to_string(++literal_counter) + " db " + t->content + ", 0";
                    as.instruct(current_scope->t->content, "mov " + location + ", " + 'L' + std::to_string(literal_counter));
                    break;
                }
                case token_types::IDENTIFIER:
                {
                    if (symbols.find(t->content) == symbols.end())
                    {
                        arrow::err("symbol '" + t->content + "' is not defined", t->line);
                        return evaluation_states::SYNTAX_ERROR;
                    }
                    symbol& sym = symbols[t->content];
                    if (location == "rax")
                        as.instruct(current_scope->t->content, std::string(!mutilating ? "mov" : "lea") + " rax, [rbp + " + std::to_string(sym.offset) + ']');
                    else
                    {
                        as.instruct(current_scope->t->content, "push rax");
                        as.instruct(current_scope->t->content, std::string(!mutilating ? "mov" : "lea") + " rax, [rbp + " + std::to_string(sym.offset) + ']');
                        as.instruct(current_scope->t->content, "mov qword " + location + ", rax");
                        as.instruct(current_scope->t->content, "pop rax");
                    }
                    break;
                }
                default:
                {
                    arrow::err("expected a symbol, string literal, or number literal", t->line);
                    return evaluation_states::SYNTAX_ERROR;
                }
            }
        }
        t = t->next;
        return evaluation_states::FOUND;
    }

    evaluation_state parser::il_asm(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "asm") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR;
        if (t->type != token_types::STRING_LITERAL)
        {
            arrow::err("string literal expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        as.instruct(current_scope->t->content, t->content.substr(1, t->content.length() - 2));
        t = t->next;
        return evaluation_states::FOUND;
    }

    evaluation_state parser::il_asm()
    {
        token*& c = current;
        return il_asm(c);
    }

    evaluation_state parser::store(token*& t)
    {
        if (check_eof(t, false)) return evaluation_states::NEUTRAL;
        if (t->content != "store") return evaluation_states::NEUTRAL;
        if (check_eof(t = t->next)) return evaluation_states::SYNTAX_ERROR;
        as.instruct(current_scope->t->content, "mov rbx, rax");
        evaluation_state e = evaluate(t, nullptr, false, true);
        if (e == evaluation_states::NEUTRAL)
        {
            arrow::err("expression expected", t->line);
            return evaluation_states::SYNTAX_ERROR;
        }
        if (e == evaluation_states::SYNTAX_ERROR) return e;
        as.instruct(current_scope->t->content, "mov [rax], rbx");
        return evaluation_states::FOUND;
    }

    evaluation_state parser::store()
    {
        token*& c = current;
        return store(c);
    }

    std::string parser::result()
    {
        return as.construct();
    }

    bool parser::has_symbol(std::string name)
    {
        return symbols.find(name) != symbols.end();
    }
}