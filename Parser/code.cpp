#include "code.hpp"

/******************************************************************************/
void
Code::write(Grammar& grammar, std::ostream& out)
{
    out << "#include \"calculator.hpp\"\n\n";
    out << "using std::unique_ptr;\n\n";
    out << "using std::vector;\n\n";
    
    size_t id = 0;
    size_t rule_id = 0;
    for (auto& nonterm : grammar.nonterms) {
        nonterm.second->rank = id++;
        for (auto& rule : nonterm.second->rules) {
            rule->id = rule_id++;
        }
    }
    
    write_struct(out);

    out << "Symbol endmark;\n\n";
    for (auto& term : grammar.terms) {
        write_declare(term.second.get(), out);
    }
    for (auto& term : grammar.terms) {
        write_define(term.second.get(), out);
    }

    grammar.lexer.write(out);
    
    for (auto& nonterm : grammar.nonterms) {
        write_declare(nonterm.second.get(), out);
        for (auto& rule : nonterm.second->rules) {
            write_proto(rule.get(), out);
        }
        for (auto& rule : nonterm.second->rules) {
            write_declare(rule.get(), out);
        }
        out << std::endl;
    }
    
    for (auto& nonterm : grammar.all) {
        for (auto& rule : nonterm->rules) {
            write_action(rule.get(), out);
            write_define(rule.get(), out);
        }
    }
    
    for (auto& state : grammar.states) {
        out << "extern State state" << state->id << ";\n";
    }
    out << std::endl;
    
    for (auto& s : grammar.states) {
        write_shift(s.get(), out);
        write_accept(s.get(), out);
        write_reduce(s.get(), out);
        write_goto(s.get(), out);
    }
    out << std::endl;
    
    for (auto& s : grammar.states) {
        write_define(s.get(), out);
    }

    write_functions(out);
}

/******************************************************************************/
void
Code::write_struct(std::ostream& out)
{
    out << "struct Symbol {\n";
    out << "    const char* name;\n";
    out << "};\n\n";
    
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

/******************************************************************************/
void
Code::write_declare(Term* term, std::ostream& out)
{
    out << "Value* scan" << term->rank << "(Table*, const std::string&);\n";
    
    out << "Symbol term" << term->rank;
    out << " = {\"" << term->name << "\"};\n";

    out << "Accept term" << term->rank << "_accept = {&term" << term->rank << ", ";
    if (!term->action.empty()) {
        out << "&scan" << term->rank << "";
    } else {
        out << "nullptr";
    }
    out << "};\n\n";
}

void
Code::write_define(Term* term, std::ostream& out)
{
    if (term->action.empty())
        return;
    
    out << "unique_ptr<" << term->type << ">\n";
    out << term->action << "(Table*, const std::string&);\n\n";

    out << "Value*\n";
    out << "scan" << term->rank << "(Table* table, const std::string& s) {\n";
    out << "    unique_ptr<" << term->type << "> value = ";
    out << term->action << "(table, s);\n";
    out << "    return value.release();\n";
    out << "}\n\n";
}

/******************************************************************************/
void
Code::write_declare(Nonterm* nonterm, std::ostream& out)
{
    out << "Symbol nonterm" << nonterm->rank;
    out << " = {\"" << nonterm->name << "\"};\n";
}

void
Code::write_declare(Nonterm::Rule* rule, std::ostream& out)
{
    out << "Rule rule" << rule->id << " = ";
    out << "{&nonterm" << rule->nonterm->rank << ", ";
    if (!rule->action.empty()) {
        out << "&" << rule->action;
    } else {
        out << "nullptr";
    }
    out << ", " << rule->product.size() << "};\n";
}

void
Code::write_proto(Nonterm::Rule* rule, std::ostream& out)
{
    out << "Value* ";
    out << rule->action << "(Table*, vector<Value*>&);\n";
}

/******************************************************************************/
void
Code::write_action(Nonterm::Rule* rule, std::ostream& out)
{
    if (!rule->nonterm->type.empty()) {
        out << "unique_ptr<" << rule->nonterm->type << ">\n";
    } else {
        out << "void\n";
    }

    out << rule->action << "(";
    out << "Table*, ";

    bool comma = false;
    for (auto sym : rule->product) {
        if (!sym->type.empty()) {
            if (comma) {
                out << ", ";
            } else {
                comma = true;
            }
            out << "unique_ptr<" << sym->type << ">&";
        }
    }
    out << ");\n\n";
}

void
Code::write_define(Nonterm::Rule* rule, std::ostream& out)
{
    out << "Value*\n";
    out << rule->action << "(Table* table, vector<Value*>& values) {\n";

    for (int i = 0; i < rule->product.size(); i++) {
        Symbol* sym = rule->product[i];
        int index = i - (int)rule->product.size();
        if (!sym->type.empty()) {
            out << "    unique_ptr<" << sym->type << "> ";
            out << "E" << i;
            out << "(dynamic_cast<" << sym->type << "*>";
            out << "(values.end()[" << index << "]));\n";
        }
    }

    out << "    unique_ptr<" << rule->nonterm->type << "> ";
    out << "R = "<< rule->action << "(";
    out << "table, ";

    bool comma = false;
    for (int i = 0; i < rule->product.size(); i++) {
        if (!rule->product[i]->type.empty()) {
            if (comma) {
                out << ", ";
            } else {
                comma = true;
            }
            out << "E" << i;
        }
    }

    out << ");\n";
    out << "    return R.release();\n";
    out << "}\n\n";
}

/******************************************************************************/
void
Code::write_shift(State* state, std::ostream& out)
{
    if (state->actions->shift.size() ==  0)
        return;

    out << "Shift shift" << state->id << "[] = {\n";
    for (auto& act : state->actions->shift) {
        out << "    {&"; act.first->write(out);
        out << ", &state" << act.second->id << "},\n";
    }
    out << "    {nullptr, nullptr}};\n";
}

void
Code::write_accept(State* state, std::ostream& out)
{
    if (state->actions->accept.size() ==  0)
        return;
    
    out << "Reduce accept" << state->id << "[] = {\n";
    for (auto& act : state->actions->accept) {
        out << "    {&"; act.first->write(out);
        out << ", &"; act.second->write(out); out << "},\n";
    }
    out << "    {nullptr, nullptr}};\n";
}

void
Code::write_reduce(State* state, std::ostream& out)
{
    if (state->actions->reduce.size() ==  0)
        return;
    
    out << "Reduce reduce" << state->id << "[] = {\n";
    for (auto& act : state->actions->reduce) {
        out << "    {&"; act.first->write(out);
        out << ", &"; act.second->write(out); out << "},\n";
    }
    out << "    {nullptr, nullptr}};\n";
}

void
Code::write_goto(State* state, std::ostream& out)
{
    if (state->gotos.size() ==  0)
        return;

    out << "Go go" << state->id << "[] = {\n";
    for (auto& go : state->gotos) {
        out << "    {&"; go.first->write(out);
        out << ", &state" << go.second->id << "},\n";
    }
    out << "    {nullptr, nullptr}};\n";
}

void
Code::write_define(State* state, std::ostream& out)
{
    out << "State ";
    out << "state" << state->id;
    out << " = {" << state->id;

    if (state->actions->shift.size()) {
        out << ", shift" << state->id;
    } else {
        out << ", nullptr";
    }
    if (state->actions->accept.size()) {
        out << ", accept" << state->id;
    } else {
        out << ", nullptr";
    }
    if (state->actions->reduce.size()) {
        out << ", reduce" << state->id;
    } else {
        out << ", nullptr";
    }
    if (state->gotos.size()) {
        out << ", go" << state->id;
    } else {
        out << ", nullptr";
    }
    out << "};\n";
}

/******************************************************************************/
void
Code::write_functions(std::ostream& out)
{
    out << "\nNode*\n";
    out << "next_node(Node* node, int c) {\n";
    out << "    return node->scan(c);\n";
    out << "}\n\n";

    out << "Accept*\n";
    out << "find_term(Node* node) {\n";
    out << "    return node->accept;\n";
    out << "}\n\n";

    out << "const char* symbol_name(Symbol* sym) { return sym->name;}\n\n";
    
    out << "Node*  node_start() { return &node0; }\n";
    out << "State* state_start() { return &state0; }\n";
    out << "Symbol* symbol_end() { return &endmark; }\n\n";
    
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
