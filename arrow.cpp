#include <iostream>
#include <fstream>
#include <vector>

#include "logger.h"
#include "assembler.h"
#include "tokenization.h"
#include "parser.h"

int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        arrow::err("no input file");
        return -1;
    }
    arrow::operating_system os = arrow::operating_systems::WINDOWS;
    std::ifstream fis = std::ifstream(argv[1]);
    if (fis.fail())
    {
        arrow::err("something happened while trying to open " + std::string(argv[1]));
        return -1;
    }
    arrow::token* first_token = new arrow::token{ "arrow", arrow::token_types::PROGRAM_START, nullptr, 0 };
    arrow::token* current = first_token;
    bool in_comment = false, in_string_literal = false;
    int line_tracker = 0;
    for (std::string t; fis.good();)
    {
        char c = fis.get();
        if (c == '"' && (t.length() == 0 || t[t.length() - 1] != '\\'))
            in_string_literal = !in_string_literal;
        if (c == '#')
            in_comment = true;
        if (c == '\n')
            line_tracker++;
        if (in_comment)
        {
            if (c == '\n')
                in_comment = false;
            continue;
        }
        if (c != '\r' && c != '\n' && (c != ' ' || in_string_literal))
            t += c;
        if (t.length() == 0)
            continue;
        if (int len = arrow::ends_with_punctuator(t) && !in_string_literal)
        {
            std::string fp = t.substr(0, t.length() - len);
            if (fp.length() != 0)
            {
                if (arrow::is_numeric_literal(fp))
                    current = current->next = new arrow::token{ fp, arrow::token_types::NUMERIC_LITERAL, nullptr, line_tracker };
                else if (arrow::is_string_literal(fp))
                    current = current->next = new arrow::token{ fp, arrow::token_types::STRING_LITERAL, nullptr, line_tracker };
                else
                    current = current->next = new arrow::token{ fp, arrow::token_types::IDENTIFIER, nullptr, line_tracker };
            }
            current = current->next = new arrow::token{ t.substr(t.length() - len), arrow::token_types::PUNCTUATOR, nullptr, line_tracker };
            t.erase();
            continue;
        }
        if (c != '\n' && c != ' ')
            continue;
        if (in_string_literal)
            continue;
        if (arrow::is_string_literal(t))
            current = current->next = new arrow::token{ t, arrow::token_types::STRING_LITERAL, nullptr, line_tracker };
        else if (arrow::is_type_specifier(t))
            current = current->next = new arrow::token{ t, arrow::token_types::TYPE_SPECIFIER, nullptr, line_tracker };
        else if (arrow::is_mnemonic(t))
            current = current->next = new arrow::token{ t, arrow::token_types::MNEMONIC, nullptr, line_tracker };
        else if (arrow::is_numeric_literal(t))
            current = current->next = new arrow::token{ t, arrow::token_types::NUMERIC_LITERAL, nullptr, line_tracker };
        else
            current = current->next = new arrow::token{ t, arrow::token_types::IDENTIFIER, nullptr, line_tracker };
        t.erase();
    }
    for (arrow::token* c = first_token; c != nullptr; c = c->next)
        std::cout << c->content << ", " << arrow::token_types::name(c->type) << std::endl;
    fis.close();
    current = first_token->next;
    arrow::parser parser = arrow::parser(current, os);
    while (parser.good())
    {
        arrow::evaluation_state ls = parser.label_start();
        std::cout << "ls: " << arrow::evaluation_states::name(ls) << std::endl;
        if (ls == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (ls == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state le = parser.label_end();
        std::cout << "le: " << arrow::evaluation_states::name(le) << std::endl;
        if (le == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (le == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state ila = parser.il_asm();
        std::cout << "ila: " << arrow::evaluation_states::name(ila) << std::endl;
        if (ila == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (ila == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state st = parser.store();
        std::cout << "st: " << arrow::evaluation_states::name(st) << std::endl;
        if (st == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (st == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state pa = parser.pass();
        std::cout << "pa: " << arrow::evaluation_states::name(pa) << std::endl;
        if (pa == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (pa == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state pu = parser.pull();
        std::cout << "pu: " << arrow::evaluation_states::name(pu) << std::endl;
        if (pu == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (pu == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state del = parser.del();
        std::cout << "del: " << arrow::evaluation_states::name(del) << std::endl;
        if (del == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (del == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state def = parser.define();
        std::cout << "def: " << arrow::evaluation_states::name(def) << std::endl;
        if (def == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (def == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state cp = parser.copy();
        std::cout << "cp: " << arrow::evaluation_states::name(cp) << std::endl;
        if (cp == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (cp == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state add = parser.add();
        std::cout << "add: " << arrow::evaluation_states::name(add) << std::endl;
        if (add == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (add == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state re = parser.ret();
        std::cout << "re: " << arrow::evaluation_states::name(re) << std::endl;
        if (re == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (re == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state se = parser.set();
        std::cout << "se: " << arrow::evaluation_states::name(se) << std::endl;
        if (se == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (se == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state rf = parser.reference();
        std::cout << "rf: " << arrow::evaluation_states::name(rf) << std::endl;
        if (rf == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (rf == arrow::evaluation_states::FOUND)
            continue;
        arrow::evaluation_state ca = parser.call();
        std::cout << "ca: " << arrow::evaluation_states::name(ca) << std::endl;
        if (ca == arrow::evaluation_states::SYNTAX_ERROR)
            return -1;
        if (ca == arrow::evaluation_states::FOUND)
            continue;
        break;
    }
    if (!parser.has_symbol("main"))
    {
        arrow::err("no entry point found for application. define a label named 'main'");
        return -1;
    }
    std::ofstream fos = std::ofstream(std::string(argv[1]) + ".asm");
    std::string out = parser.result();
    fos.write(out.c_str(), out.length());
    fos.close();
    free_tokens(first_token);
}