#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <stdexcept>

namespace arrow
{
    typedef std::string register_resolvable;

    extern const char* ARG_REGISTER_SEQUENCE[4];

    register_resolvable resolve_register(register_resolvable& identifier, int size);
    register_resolvable resolve_register(register_resolvable&& identifier, int size);

    class subroutine
    {
    public:
        std::string name;
        std::string instructions;
        int stackalloc;
        int offset_mutilator;
        std::string ending;
        subroutine* parent;
        std::vector<subroutine*>* children;
        bool preserve_ret_value;

        subroutine(std::string name, subroutine* parent);
        subroutine& alloc_delta(int bs);
        subroutine& add_child(subroutine* sr);
        std::string construct();
        ~subroutine();
    };

    class assembler
    {
    private:
        std::string data;
        std::string bss;
        std::set<std::string> ext;
        std::map<std::string, subroutine*> subroutines;
    public:
        std::string entry;
        unsigned char write_mode;

        assembler(std::string entry = "main");
        assembler& enter(std::string& subroutine);
        assembler& enter(std::string&& subroutine);
        assembler& instruct(std::string& subroutine, std::string instruction);
        assembler& instruct(std::string&& subroutine, std::string instruction);
        assembler& external(std::string identifier);
        arrow::subroutine*& sr(std::string& name, subroutine* parent);
        arrow::subroutine*& sr(std::string& name);
        assembler& operator<<(std::string& line);
        assembler& operator<<(std::string&& line);
        assembler& operator<<(assembler& (*mod)(assembler& as));
        std::string construct();
    };

    assembler& data(assembler& as);
    assembler& bss(assembler& as);
}

#endif