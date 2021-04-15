#include "code.hpp"

void
Code::write(Generator& gen, std::vector<State*>& states, std::ostream& out)
{
    out << "#include \"calculator.hpp\"\n\n";
    out << "using std::unique_ptr;\n\n";
    out << "using std::vector;\n\n";
    
    write_struct(out);
    write_terms(gen, out);
    
    gen.lexer.write(out);
    
    size_t id = 0;
    for (auto& nonterm : gen.nonterms) {
        nonterm.second->rank = id++;
        nonterm.second->write_declare(out);
    }
    out << std::endl;

    for (auto& nonterm : gen.all) {
        for (auto& rule : nonterm->rules) {
            rule->write_proto(out);
        }
    }
    out << std::endl;
    
    id = 0;
    for (auto& nonterm : gen.nonterms) {
        for (auto& rule : nonterm.second->rules) {
            rule->id = id++;
            rule->write_declare(out);
        }
    }
    out << std::endl;

    
    for (auto& state : gen.states) {
        state->write_declare(out);
    }
    out << std::endl;
    
    for (auto& state : states) {
        //state->write_shift(out);
        state->write_accept(out);
        state->write_reduce(out);
        state->write_goto(out);
    }
    out << std::endl;

//    for (auto& state : states) {
//        state->write_define(out);
//    }
//    out << std::endl;


    
    for (auto s : states) {
        write_state(s, out);
    }
    
    for (auto& nonterm : gen.all) {
        for (auto& rule : nonterm->rules) {
            rule->write_action(out);
        }
    }
    out << std::endl;
    
    for (auto& nonterm : gen.all) {
        for (auto& rule : nonterm->rules) {
            rule->write_define(out);
        }
    }
    out << std::endl;
    
    for (auto s : states) {
        write_define(s, out);
    }
    
    write_functions(out);

}

void
Code::write_define(State* state, std::ostream& out)
{
    out << "State ";
    state->write(out);
    out << " = {" << state->id;

    if (state->actions->shift.size() > 0) {
        out << ", shift" << state->id;
    } else {
        out << ", nullptr";
    }
    if (state->actions->accept.size() > 0) {
        out << ", accept" << state->id;
    } else {
        out << ", nullptr";
    }
    if (state->actions->reduce.size() > 0) {
        out << ", reduce" << state->id;
    } else {
        out << ", nullptr";
    }
    if (state->gotos.size() > 0) {
        out << ", go" << state->id;
    } else {
        out << ", nullptr";
    }
    out << "};\n";
}


void
Code::write_state(State* state, std::ostream& out)
{
    if (state->actions->shift.size() ==  0)
        return;

    out << "Shift shift" << state->id << "[] = {\n";
    for (auto& act : state->actions->shift) {
        out << "    {&";
        act.first->write(out);
        out << ", &";
        act.second->write(out);
        out << "}, // ";
        act.first->print(out);
        out << " -> ";
        act.second->print(out);
        out << "\n";
    }
    out << "    {nullptr, nullptr}};\n";
}

void
Code::write_terms(Generator& gen, std::ostream& out)
{
    for (auto& term : gen.terms) {
        term.second->write_proto(out);
    }
    out << std::endl;
    
    for (auto& term : gen.terms) {
        term.second->write_define(out);
    }
    out << std::endl;
    
    for (auto& term : gen.terms) {
        term.second->write_declare(out);
    }
    out << "Symbol endmark;\n";
    out << std::endl;
}

void
Code::write_struct(std::ostream& out)
{
    // TODO Remove the symbol.
    out << "struct Symbol {\n";
    out << "    const char* name;\n";
    out << "};\n";

    
    out << "struct Node {\n";
    out << "    Node* (*scan)(int c);\n";
    out << "    Accept* accept;\n";
    out << "};\n\n";
    
    out << "class State;\n\n";
    
    out << "struct Shift {\n";
    out << "    Symbol* sym;\n";
    out << "    State*  state;\n";
    out << "};\n\n";

    out << "struct Reduce {\n";
    out << "    Symbol* sym;\n";
    out << "    Rule*   rule;\n";
    out << "};\n\n";

    out << "struct Go {\n";
    out << "    Symbol* sym;\n";
    out << "    State*  state;\n";
    out << "};\n\n";
    
    out << "struct State {\n";
    out << "    int     id;\n";
    out << "    Shift*  shift;\n";
    out << "    Reduce* accept;\n";
    out << "    Reduce* reduce;\n";
    out << "    Go*     next;\n";
    out << "};\n\n";
}

void
Code::write_functions(std::ostream& out)
{
    out << "Node*\n";
    out << "next_node(Node* node, int c)\n";
    out << "{\n";
    out << "    return node->scan(c);\n";
    out << "}\n";

    out << "Accept*\n";
    out << "find_term(Node* node) {\n";
    out << "    return node->accept;\n";
    out << "}\n";

    out << "const char* symbol_name(Symbol* sym) { return sym->name;}\n";
    
    out << "Node*  node_start() { return &node0; }\n";
    out << "State* state_start() { return &state0; }\n";
    out << "Symbol* symbol_end() { return &endmark; }\n";

    
    out << "State*\n";
    out << "find_shift(State* state, Symbol* sym) {\n";
    out << "    if (!state->shift)\n";
    out << "        return nullptr;\n";
    out << "    for (Shift* s = state->shift; s->sym; s++) {\n";
    out << "        if (s->sym == sym) {\n";
    out << "            return s->state;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return nullptr;\n";
    out << "}\n\n";

    out << "Rule*\n";
    out << "find_accept(State* state, Symbol* sym) {\n";
    out << "    if (!state->accept)\n";
    out << "        return nullptr;\n";
    out << "    for (Reduce* r = state->accept; r->sym; r++) {\n";
    out << "        if (r->sym == sym) {\n";
    out << "            return r->rule;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return nullptr;\n";
    out << "}\n\n";

    out << "Rule*\n";
    out << "find_reduce(State* state, Symbol* sym) {\n";
    out << "    if (!state->reduce)\n";
    out << "        return nullptr;\n";
    out << "   for (Reduce* r = state->reduce; r->sym; r++) {\n";
    out << "        if (r->sym == sym) {\n";
    out << "            return r->rule;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return nullptr;\n";
    out << "}\n\n";

    out << "State*\n";
    out << "find_goto(State* state, Symbol* sym) {\n";
    out << "   if (!state->next)\n";
    out << "       return nullptr;\n";
    out << "   for (Go* g = state->next; g->sym; g++) {\n";
    out << "       if (g->sym == sym) {\n";
    out << "           return g->state;\n";
    out << "       }\n";
    out << "   }\n";
    out << "   return nullptr;\n";
    out << "}\n\n";
}

