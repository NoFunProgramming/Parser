#include "code.hpp"

/******************************************************************************/
void
Code::write(const Grammar& grammar, std::ostream& out)
{
    for (auto include : grammar.includes) {
        out << include << std::endl;
    }
    out << "using std::unique_ptr;\n\n";
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
    
    out << "struct Node {\n";
    out << "    Node* (*scan)(int c);\n";
    out << "    Accept* accept;\n";
    out << "};\n\n";
    
//    out << "struct N {\n";
//    out << "    int (*next)(int c);\n";
//    out << "    Symbol* term;\n";
//    out << "    Value* (*scan)(Table*, const std::string &);\n";
//    out << "};\n\n";


    out << "Symbol endmark;\n\n";
    for (auto& term : grammar.terms) {
        declare_terms(term.second.get(), out);
    }
    for (auto& term : grammar.terms) {
        define_term_actions(term.second.get(), out);
    }

    grammar.lexer.write(out);
    
    out << "Node*\n";
    out << "next_node(Node* node, int c) {\n";
    out << "    return node->scan(c);\n";
    out << "}\n\n";

    out << "Accept*\n";
    out << "is_accept(Node* node) {\n";
    out << "    return node->accept;\n";
    out << "}\n\n";

    out << "Node*   Start_Node  = &node0;\n";
    //out << "State*  Start_State = &state0;\n";
    out << "Symbol* Endmark     = &endmark;\n";

    
    out << "//***************************************************************\n";
    declare_structs(out);

    
    for (auto& nonterm : grammar.nonterms) {
        declare_nonterm(nonterm.second.get(), out);
        out << std::endl;
    }
    out << std::endl;
    
    for (auto& nonterm : grammar.all) {
        for (auto& rule : nonterm->rules) {
            define_action(rule.get(), out);
            define_action_call(rule.get(), out);
        }
    }
    
    declare_rules(grammar, out);
        
    for (auto& s : grammar.states) {
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
    out << std::endl;
    
    define_gotos(grammar, out);
    
    declare_states(grammar, out);

    define_functions(out);
}

/******************************************************************************/
void
Code::declare_structs(std::ostream& out)
{
        
    out << "struct Act {\n";
    out << "    Symbol* sym;\n";
    out << "    char    type;\n";
    out << "    int     next;\n";
    out << "};\n\n";
    
    out << "struct Gos {\n";
    out << "    Symbol* sym;\n";
    out << "    int  state;\n";
    out << "};\n\n";
    
    out << "struct St {\n";
    out << "    struct Act* act;\n";
    out << "    struct Gos* gos;\n";
    out << "};\n\n";
}

/******************************************************************************/
void
Code::declare_terms(Term* term, std::ostream& out)
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

//void
//Code::declare_action(Nonterm::Rule* rule, std::ostream& out)
//{
//    out << "Value* ";
//    out << rule->action << "(Table*, vector<Value*>&);\n";
//}

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
Code::define_action_call(Nonterm::Rule* rule, std::ostream& out)
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
/******************************************************************************/
void
Code::define_functions(std::ostream& out)
{
    out << "char\n";
    out << "find_action(int state, Symbol* sym, int* next) {\n";
    out << "    for (Act* s = sts[state].act; s->sym; s++) {\n";
    out << "        if (s->sym == sym) {\n";
    out << "            *next = s->next;\n";
    out << "            return s->type;\n";
    out << "        }\n";
    out << "    }\n";
    out << "    return -1;\n";
    out << "}\n\n";

    out << "int\n";
    out << "find_goto2(int state, Symbol* sym) {\n";
    out << "   if (!sts[state].gos)\n";
    out << "        return -1;\n";
    out << "   for (Gos* g = sts[state].gos; g->sym; g++) {\n";
    out << "       if (g->sym == sym) {\n";
    out << "           return g->state;\n";
    out << "       }\n";
    out << "   }\n";
    out << "   return -1;\n";
    out << "}\n\n";

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
    
    out << "St sts[] = {\n";
    for (auto& state : states) {
        out << "    {act" << state->id << ", ";
        if (state->gotos.size() > 0) {
            out << "gos" << state->id;
        } else {
            out << "nullptr";
        }
        out << "},\n";
    }
    out << "};\n\n";
}

void
Code::declare_rules(const Grammar& grammar, std::ostream& out)
{
    out << "Rs rs[] = {\n";
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
Code::define_gotos(const Grammar& grammar, std::ostream& out)
{
    for (auto& s : grammar.states) {
        if (s->gotos.size() == 0) {
            continue;
        }
        
        out << "struct Gos gos" << s->id << "[] = {";

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
