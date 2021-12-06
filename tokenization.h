#ifndef ARROW_TOKENIZATION_H
#define ARROW_TOKENIZATION_H

#include <string>
#include <vector>
#include <algorithm>

namespace arrow
{
    typedef unsigned int token_type;
    namespace token_types
    {
        const token_type PROGRAM_START = 0x00;
        const token_type IDENTIFIER = 0x01;
        const token_type PUNCTUATOR = 0x02;
        const token_type NUMERIC_LITERAL = 0x03;
        const token_type STRING_LITERAL = 0x04;
        const token_type TYPE_SPECIFIER = 0x05;
        const token_type MNEMONIC = 0x06;

        std::string name(token_type tt);
    }

    typedef struct token {
        std::string content;
        token_type type;
        token* next;
        int line;
    } token;

    bool is_string_literal(std::string& t);
    bool is_type_specifier(std::string& t);
    bool is_mnemonic(std::string& t);
    bool is_numeric_literal(std::string& t);
    int ends_with_punctuator(std::string& t);
    void free_tokens(token* t);
}

#endif