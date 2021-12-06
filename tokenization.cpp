#include "tokenization.h"

namespace arrow
{
    const std::vector<std::string> PUNCTUATORS = {
        ",",
        "*",
        "{",
        "}",
        ";"
    };

    const std::vector<std::string> MNEMONICS = {
        "ref",
        "copy",
        "set",
        "pass",
        "pull",
        "push",
        "pop",
        "call",
        "add",
        "del",
        "def",
        "ret"
    };

    namespace token_types
    {
        std::string name(token_type tt)
        {
            switch (tt)
            {
                case PROGRAM_START: return "PROGRAM_START";
                case IDENTIFIER: return "IDENTIFIER";
                case PUNCTUATOR: return "PUNCTUATOR";
                case NUMERIC_LITERAL: return "NUMERIC_LITERAL";
                case STRING_LITERAL: return "STRING_LITERAL";
                case TYPE_SPECIFIER: return "TYPE_SPECIFIER";
                case MNEMONIC: return "MNEMONIC";
                default: return "UNNAMED_TOKEN_TYPE_" + std::to_string(tt);
            }
        }
    }

    bool is_string_literal(std::string& t)
    {
        return t.length() >= 2 && t[0] == '\"' && t[t.length() - 1] == '\"';
    }

    bool is_type_specifier(std::string& t)
    {
        return t == "byte" || t == "short" || t == "int" || t == "long" || t == "float" || t == "double";
    }

    bool is_mnemonic(std::string& t)
    {
        return std::find(MNEMONICS.begin(), MNEMONICS.end(), t) != MNEMONICS.end(); 
    }

    bool is_numeric_literal(std::string& t)
    {
        if (t.length() == 0)
            return false;
        for (int i = 0; i < t.length(); i++)
        {
            char c = t[i];
            if ((c < '0' || c > '9') && c != '.')
                return false;
        }
        return true;
    }

    int ends_with_punctuator(std::string& t)
    {
        for (const auto& punctuator : PUNCTUATORS)
        {
            if (punctuator.length() > t.length())
                continue;
            bool a = true;
            for (int i = punctuator.length() - 1, j = t.length() - 1; i >= 0 && j >= 0; i--, j--)
            {
                if (punctuator[i] != t[j])
                {
                    a = false;
                    break;
                }
            }
            if (a) return punctuator.length();
        }
        return 0;
    }

    void free_tokens(token* t)
    {
        if (t == nullptr)
            return;
        free_tokens(t->next);
        delete t;
    }
}