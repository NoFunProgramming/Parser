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
    for (auto nonterm : grammar.all) {
        nonterm->rank = id++;
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

    write(grammar.lexer, out);
    
    for (auto& nonterm : grammar.nonterms) {
        declare_nonterm(nonterm.second.get(), out);
        out << std::endl;
    }
    out << std::endl;
    
    for (auto rule : grammar.all_rules) {
        define_action(rule, out);
        define_action_cast(rule, out);
        define_action_call(rule, out);
    }
    
    declare_rules(grammar, out);
        
    for (auto& s : grammar.states) {
        define_actions(s.get(), out);
    }
    out << std::endl;
    
    for (auto& s : grammar.states) {
        define_gotos(s.get(), out);
    }
    out << std::endl;
    
    declare_states(grammar, out);
}

/**
 * Writes the source code for a lexer.  The source code will define a structure
 * for each state in DFA. This structure contains a method that take a character
 * and returns either a new state in the DFA or a null pointer.  The null
 * indicates that the pattern matching is complete and the accept value, if any,
 * for the current state is the type of token identified.
 */
void
Code::write(const Lexer& lexer, std::ostream& out)
{
    std::vector<Node*> sorted;
    for (auto& state : lexer.nodes) {
        sorted.push_back(state.get());
    }
    struct {
        bool operator()(Node* a, Node* b) const { return a->id < b->id; }
    } Compare;
    
    std::sort(sorted.begin(), sorted.end(), Compare);

    for (auto& state : sorted) {
        define_scan(state, out);
    }
    out << "\n";
    
    out << "Node nodes[] = {\n";
    for (auto& state : sorted) {
        define_node(state, out);
    }
    out << "};\n";
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

/**
 * Writes the source code for a single state of the lexer.  The source code
 * defines a structure and a method for each state.  The method takes an input
 * character and returns either a new state in the DFA or a null pointer.
 */
void
Code::define_scan(Node* node, std::ostream& out)
{
    if (node->nexts.size() == 0) {
        return;
    }
    
    out << "int\n";
    out << "scanX" << node->id << "(int c) {\n";
    for (auto next : node->nexts) {
        out << "    if (";
        next.first.write(out);
        out << ") { return " << next.second->id << "; }\n";
    }
    out << "    return -1;\n";
    out << "}\n\n";
}

void
Code::define_node(Node* node, std::ostream& out)
{
    if (node->nexts.size() > 0) {
        out << "    {&scanX" << node->id;
    } else {
        out << "    {nullptr";
    }

    if (node->accept) {
        out << ", &term" << node->accept->rank;
        if (node->accept->scan.size() > 0) {
            out << ", &scan" << node->accept->rank << "";
        } else {
            out << ", nullptr";
        }
    } else {
        out << ", nullptr";
        out << ", nullptr";
    }
    out << "},\n";
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

/******************************************************************************/
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
Code::define_actions(State* s, std::ostream& out)
{
    out << "struct Act act" << s->id << "[] = {";

    for (auto& act : s->actions->shift) {
        out << "{&"; act.first->write(out);
        out << ", 'S', " << act.second->id; out << "}, ";
    }

    for (auto& act : s->actions->reduce) {
        out << "{&"; act.first->write(out);
        out << ", 'R', " << act.second->id << "}, ";
    }

    for (auto& act : s->actions->accept) {
        out << "{&"; act.first->write(out);
        out << ", 'A', " << act.second->id << "}, ";
    }

    out << "{nullptr, 0, 0}";
    out << "};\n";
}

void
Code::define_gotos(State* s, std::ostream& out)
{
    if (!s->gotos.size())
        return;
    
    out << "struct Go go" << s->id << "[] = {";

    for (auto& g : s->gotos) {
        out << "{&";
        g.first->write(out);
        out << ", " << g.second->id << "}, ";
    }
    
    out << "{nullptr, 0}";
    out << "};\n";
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

