#ifndef ARROW_PARSER_H
#define ARROW_PARSER_H

#include <map>
#include <stack>

#include "tokenization.h"
#include "logger.h"
#include "assembler.h"

namespace arrow
{
    typedef unsigned int evaluation_state;
    namespace evaluation_states
    {
        const evaluation_state FOUND = 0x00;
        const evaluation_state NEUTRAL = 0x01;
        const evaluation_state SYNTAX_ERROR = 0x02;

        std::string name(evaluation_state es);
    }

    typedef unsigned int operating_system;
    namespace operating_systems
    {
        const operating_system WINDOWS = 0x00;
        const operating_system MAC = 0x01;
        const operating_system LINUX = 0x02;

        std::string name(operating_system os);
    }

    typedef struct symbol {
        token* t;
        symbol* scope;
        int offset;
    } symbol;

    class parser
    {
    private:
        std::map<std::string, symbol> symbols;
        token* current;
        symbol* current_scope;
        assembler as;
        operating_system os;
        std::stack<token*> local_push_stack;
        int literal_counter;
        int arguments;
    public:
        parser(token* start, operating_system os);
        bool good();
        evaluation_state label_start(token*& t);
        evaluation_state label_start();
        evaluation_state label_end(token*& t);
        evaluation_state label_end();
        evaluation_state define(token*& t);
        evaluation_state define();
        evaluation_state reference(token*& t);
        evaluation_state reference();
        evaluation_state evaluate(token*& t, symbol* dest, bool validate = false, bool mutilating = false);
        evaluation_state copy(token*& t);
        evaluation_state copy();
        evaluation_state add(token*& t);
        evaluation_state add();
        evaluation_state set(token*& t);
        evaluation_state set();
        evaluation_state pass(token*& t);
        evaluation_state pass();
        evaluation_state pull(token*& t);
        evaluation_state pull();
        evaluation_state ret(token*& t);
        evaluation_state ret();
        evaluation_state call(token*& t);
        evaluation_state call();
        evaluation_state del(token*& t);
        evaluation_state del();
        evaluation_state il_asm(token*& t);
        evaluation_state il_asm();
        std::string result();
    };
}

#endif