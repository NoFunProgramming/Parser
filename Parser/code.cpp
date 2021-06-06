#include "code.hpp"

/******************************************************************************/
void
Code::write(const Grammar& grammar, std::ostream& out)
{
    for (auto include : grammar.includes) {
        out << include << std::endl;
    }
    out << "using std::unique_ptr;\n";
    out << "using std::vector;\n\n";
        
    // TODO Number nonterm not in a rule.
    size_t id = 0;
    size_t rule_id = 0;
    for (auto nonterm : grammar.all) {
        nonterm->rank = id++;
        for (auto& rule : nonterm->rules) {
            rule->id = rule_id++;
        }
    }

    out << "Symbol endmark;\n";
    out << "Symbol* Endmark = &endmark;\n\n";
    
    for (auto& term : grammar.terms) {
        declare_terms(term.second.get(), out);
    }
    out << "\n";
    
    for (auto& term : grammar.terms) {
        define_term_actions(term.second.get(), out);
    }

    grammar.lexer.write(out);
    
    declare_structs(out);

    for (auto& nonterm : grammar.nonterms) {
        declare_nonterm(nonterm.second.get(), out);
        out << std::endl;
    }
    out << std::endl;
    
    for (auto& nonterm : grammar.all) {
        for (auto& rule : nonterm->rules) {
            define_action(rule.get(), out);
            define_action_cast(rule.get(), out);
            define_action_call(rule.get(), out);
        }
    }
    
    declare_rules(grammar, out);
        
    for (auto& s : grammar.states) {
        define_actions(s.get(), out);
    }
    out << std::endl;
    
    define_gotos(grammar, out);
    declare_states(grammar, out);
    define_functions(out);
}

void
Code::define_actions(State* s, std::ostream& out)
{
    out << "struct Act act" << s->id << "[] = {";

    bool comma = false;
    for (auto& act : s->actions->shift) {
        if (comma) { out << ", "; } else { comma = true; }
        out << "{&"; act.first->write(out);
        out << ", 'S', " << act.second->id; out << "}";
    }

    for (auto& act : s->actions->reduce) {
        if (comma) { out << ", "; } else { comma = true; }
        out << "{&"; act.first->write(out);
        out << ", 'R', " << act.second->id << "}";
    }

    for (auto& act : s->actions->accept) {
        if (comma) { out << ", "; } else { comma = true; }
        out << "{&"; act.first->write(out);
        out << ", 'A', " << act.second->id << "}";
    }

    if (comma) { out << ", "; }
    out << "{nullptr, 0, 0}";
    out << "};\n";
}

/******************************************************************************/
void
Code::declare_structs(std::ostream& out)
{
}

/******************************************************************************/
void
Code::declare_terms(Term* term, std::ostream& out)
{
    out << "Symbol term" << term->rank;
    out << " = {\"" << term->name << "\"};\n";
}

void
Code::define_term_actions(Term* term, std::ostream& out)
{
    if (term->action.empty())
        return;
    
    out << "unique_ptr<" << term->type << ">\n";
    out << term->action << "(Table*, const std::string&);\n\n";

    out << "Value*\n";
    out << "scan" << term->rank << "(Table* t, const std::string& s) {\n";
    out << "    unique_ptr<" << term->type << "> value = ";
    out << term->action << "(t, s);\n";
    out << "    return value.release();\n";
    out << "}\n\n";
}

/******************************************************************************/
void
Code::declare_nonterm(Nonterm* nonterm, std::ostream& out)
{
    out << "Symbol nonterm" << nonterm->rank;
    out << " = {\"" << nonterm->name << "\"};";
}

/******************************************************************************/
void
Code::define_action(Nonterm::Rule* rule, std::ostream& out)
{
    if (!rule->nonterm->type.empty()) {
        out << "unique_ptr<" << rule->nonterm->type << ">\n";
    } else {
        out << "void\n";
    }

    out << rule->action << "(";
    out << "Table*";

    bool comma = true;
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
Code::define_action_cast(Nonterm::Rule* rule, std::ostream& out)
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
}

void
Code::define_action_call(Nonterm::Rule* rule, std::ostream& out)
{
    out << "    unique_ptr<" << rule->nonterm->type << "> ";
    out << "R = "<< rule->action << "(";
    out << "table";

    bool comma = true;
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


void
Code::declare_rules(const Grammar& grammar, std::ostream& out)
{
    out << "Rule rules[] = {\n";
    for (auto& nonterm : grammar.all) {
        for (auto& rule : nonterm->rules) {
            out << "  {&nonterm" << rule->nonterm->rank << ", ";
            out << rule->product.size() << ", ";
            if (!rule->action.empty()) {
                out << "&" << rule->action;
            } else {
                out << "nullptr";
            }
            out << "},\n";
        }
    }
    out << "};\n\n";
}

void
Code::declare_states(const Grammar& grammar, std::ostream& out)
{
    std::vector<State*> states;
    for (auto& state : grammar.states) {
        states.push_back(state.get());
    }
    struct {
        bool operator()(State* a, State* b) const { return a->id < b->id; }
    } Compare;
    
    std::sort(states.begin(), states.end(), Compare);
    
    out << "State states[] = {\n";
    for (auto& state : states) {
        out << "    {act" << state->id << ", ";
        if (state->gotos.size() > 0) {
            out << "go" << state->id;
        } else {
            out << "nullptr";
        }
        out << "},\n";
    }
    out << "};\n\n";
}

void
Code::define_gotos(const Grammar& grammar, std::ostream& out)
{
    for (auto& s : grammar.states) {
        if (s->gotos.size() == 0) {
            continue;
        }
        
        out << "struct Go go" << s->id << "[] = {";

        bool comma = false;
        for (auto& g : s->gotos) {
            if (comma) { out << ", "; } else { comma = true; }
            out << "{&"; g.first->write(out);
            out << ", " << g.second->id; out << "}";
        }
        
        if (comma) { out << ", "; }
        out << "{nullptr, 0}";
        out << "};\n";
    }
    out << std::endl;
}

/******************************************************************************/
void
Code::define_functions(std::ostream& out)
{
//    out << "char\n";
//    out << "find_action(int state, Symbol* sym, int* next) {\n";
//    out << "    for (Act* s = states[state].act; s->sym; s++) {\n";
//    out << "        if (s->sym == sym) {\n";
//    out << "            *next = s->next;\n";
//    out << "            return s->type;\n";
//    out << "        }\n";
//    out << "    }\n";
//    out << "    return -1;\n";
//    out << "}\n\n";
//
//    out << "int\n";
//    out << "find_goto(int state, Symbol* sym) {\n";
//    out << "   if (!states[state].go)\n";
//    out << "        return -1;\n";
//    out << "   for (Go* g = states[state].go; g->sym; g++) {\n";
//    out << "       if (g->sym == sym) {\n";
//    out << "           return g->state;\n";
//    out << "       }\n";
//    out << "   }\n";
//    out << "   return -1;\n";
//    out << "}\n\n";
}
