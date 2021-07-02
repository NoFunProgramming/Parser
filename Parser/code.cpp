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
    
    for (auto& term : grammar.terms) {
        write_terms(term.second.get(), out);
    }
    out << "\n";
    
    for (auto& term : grammar.terms) {
        write_eval(term.second.get(), out);
    }

    write(grammar.lexer, out);
    
    out << "Symbol endmark;\n";
    out << "Symbol* Endmark = &endmark;\n\n";
    
    for (auto& nonterm : grammar.nonterms) {
        write_nonterm(nonterm.second.get(), out);
        out << std::endl;
    }
    out << std::endl;
    
    /** Sort the states so the data can be access by an array index. */
    std::vector<State*> states;
    for (auto& state : grammar.states) {
        states.push_back(state.get());
    }
    
    struct {
        bool operator()(State* a, State* b) const {
            return a->id < b->id;
        }
    } compare;
    
    std::sort(states.begin(), states.end(), compare);
    
    /** Methods that cast value pointers to user define types. */
    for (auto rule : grammar.all_rules) {
        write_rule_action(rule, out);
        write_call_action(rule, out);
    }
    
    write_rules(grammar, out);
        
    write_actions(states, out);
    write_gotos(states, out);
    write_states(states, out);
}

/*******************************************************************************
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
        bool operator()(Node* a, Node* b) const {
            return a->id < b->id;
        }
    } compare;
    
    std::sort(sorted.begin(), sorted.end(), compare);

    for (auto state : sorted) {
        write_scan(state, out);
    }
    
    out << "Node nodes[] = {\n";
    for (auto state : sorted) {
        write_node(state, out);
    }
    out << "};\n";
    out << "\n";
}

/******************************************************************************/
void
Code::write_terms(Term* term, std::ostream& out)
{
    out << "Symbol term" << term->rank;
    out << " = {\"" << term->name << "\"};\n";
}

void
Code::write_eval(Term* term, std::ostream& out)
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


/**
 * Writes the source code for a single state of the lexer.  The source code
 * defines a structure and a method for each state.  The method takes an input
 * character and returns either a new state in the DFA or a null pointer.
 */
void
Code::write_scan(Node* node, std::ostream& out)
{
    if (node->nexts.size() == 0) {
        return;
    }
    
    out << "int\n";
    out << "next" << node->id << "(int c) {\n";
    for (auto next : node->nexts) {
        out << "    if (";
        next.first.write(out);
        out << ") { return " << next.second->id << "; }\n";
    }
    out << "    return -1;\n";
    out << "}\n\n";
}

void
Code::write_node(Node* node, std::ostream& out)
{
    if (node->nexts.size() > 0) {
        out << "    {&next" << node->id;
    } else {
        out << "    {nullptr";
    }

    if (node->accept) {
        out << ", &term" << node->accept->rank;
        if (node->accept->action.size() > 0) {
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
Code::write_nonterm(Nonterm* nonterm, std::ostream& out)
{
    out << "Symbol nonterm" << nonterm->rank;
    out << " = {\"" << nonterm->name << "\"};";
}

void
Code::write_rule_action(Nonterm::Rule* rule, std::ostream& out)
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
Code::write_call_action(Nonterm::Rule* rule, std::ostream& out)
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

void
Code::write_rules(const Grammar& grammar, std::ostream& out)
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

/******************************************************************************/
void
Code::write_actions(std::vector<State*> states, std::ostream& out)
{
    for (auto s : states) {
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
    out << "\n";
}

void
Code::write_gotos(std::vector<State*> states, std::ostream& out)
{
    for (auto s : states) {
        if (!s->gotos.size()) {
            continue;
        }
        out << "struct Go go" << s->id << "[] = {";
        for (auto& g : s->gotos) {
            out << "{&";
            g.first->write(out);
            out << ", " << g.second->id << "}, ";
        }
        out << "{nullptr, 0}";
        out << "};\n";
    }
    out << "\n";
}

void
Code::write_states(std::vector<State*> states, std::ostream& out)
{
    out << "State states[] = {\n";
    for (auto& s : states) {
        out << "    {act" << s->id << ", ";
        if (s->gotos.size() > 0) {
            out << "go" << s->id;
        } else {
            out << "nullptr";
        }
        out << "},\n";
    }
    out << "};\n\n";
}

