#include "code.hpp"

void
Code::write(std::vector<State*>& states, std::ostream& out)
{
    write_struct(out);
    write_functions(out);
    for (auto s : states) {
        write_state(s, out);
    }
    for (auto s : states) {
        write_define(s, out);
    }
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
Code::write_struct(std::ostream& out)
{
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

