#include "symbols.hpp"

/******************************************************************************/
Term::Term(const string& name, size_t rank):
    name(name),
    rank(rank){}

void
Term::print(ostream& out) const {
    out << "'" << name << "'";
}

void
Term::write(ostream& out) const {
    out << "term" << rank;
}

void
Term::write_declare(ostream& out) const
{
    out << "Term term" << rank << " = {\"" << name << "\", ";
    if (!action.empty()) {
        out << "&term" << rank << "_scan";
    } else {
        out << "nullptr";
    }
    out << "};\n";
}

void
Term::write_proto(ostream& out) const
{
    if (action.empty())
        return;
    out << "unique_ptr<" << type << "> ";
    out << action << "(const std::string& s);\n";
}

void
Term::write_define(ostream& out) const
{
    if (action.empty())
        return;
    out << "Value*\n";
    out << "term" << rank << "_scan(const std::string& s) {\n";
    out << "    unique_ptr<" << type << "> value = " << action << "(s);\n";
    out << "    return value.release();\n";
    out << "}\n";
}

/******************************************************************************/
void
Endmark::print(ostream& out) const {
    out << "$";
}

void
Endmark::write(ostream& out) const {
    out << "endmark";
}

/******************************************************************************/
Nonterm::Nonterm(const string& name):
    name(name),
    rank(0),
    empty_first(false){}


void
Nonterm::print(ostream& out) const {
    out << name;
}

void
Nonterm::write(ostream& out) const {
    out << "nonterm" << rank;
}

void
Nonterm::add_rule(const vector<Symbol*>& syms, const string& action)
{
    rules.emplace_back(std::make_unique<Rule>(this, action));
    Rule* rule = rules.back().get();
    
    rule->product.insert(rule->product.end(), syms.begin(), syms.end());
}

/******************************************************************************/
void
Nonterm::solve_first(bool *found)
{
    for (auto& rule : rules) {
        insert_firsts(rule.get(), found);
    }
}

void
Nonterm::insert_firsts(Rule* rule, bool* found)
{
    for (auto sym : rule->product) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(sym);
        if (nonterm) {
            for (auto first : nonterm->firsts) {
                auto inserted = firsts.insert(first);
                if (inserted.second) {
                    *found = true;
                }
            }
            if (!nonterm->empty_first) {
                return;
            }
        } else {
            auto inserted = firsts.insert(sym);
            if (inserted.second) {
                *found = true;
            }
            return;
        }
    }
    if (!empty_first) {
        empty_first = true;
        *found = true;
    }
}

/******************************************************************************/
void
Nonterm::solve_follows(bool *found)
{
    for (auto& rule : rules) {
        auto sym = rule->product.begin();
        auto end = rule->product.end();
        
        for (; sym < end; sym++) {
            Nonterm* nonterm = dynamic_cast<Nonterm*>(*sym);
            if (nonterm) {
                bool empty = false;
                nonterm->insert_follows(sym + 1, end, &empty, found);
                if (empty) {
                    nonterm->insert_follows(rule->nonterm->follows, found);
                }
            }
        }
    }
}

void
Nonterm::insert_follows(const set<Symbol*>& syms, bool* found)
{
    for (Symbol* sym : syms) {
        auto in = follows.insert(sym);
        if (in.second) {
            *found = true;
        }
    }
}

void
Nonterm::insert_follows(vector<Symbol*>::iterator sym,
                        vector<Symbol*>::iterator end,
                        bool* epsilon, bool* found)
{
    for (; sym < end; sym++) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(*sym);
        if (nonterm) {
            insert_follows(nonterm->firsts, found);
            if (!nonterm->empty_first) {
                *epsilon = false;
                return;
            }
        } else {
            auto in = follows.insert(*sym);
            if (in.second) {
                *found = true;
            }
            *epsilon = false;
            return;
        }
    }
    *epsilon = true;
}

/******************************************************************************/
void
Nonterm::print_rules(ostream& out) const
{
    bool bar = false;
    for (auto& rule : rules) {
        if (!bar) {
            out << name << ": ";
            bar = true;
        } else {
            out << "  | ";
        }
        
        bool space = false;
        for (auto sym : rule->product) {
            if (space) {
                out << " ";
            } else {
                space = true;
            }
            sym->print(out);
        }
        out << "\n";
    }
}

void
Nonterm::print_firsts(ostream& out) const
{
    out << "  "; print(out); out << ": ";
    bool space = false;
    for (auto sym : firsts) {
        if (space) {
            out << " ";
        } else {
            space = true;
        }
        sym->print(out);
    }
}

void
Nonterm::print_follows(ostream& out) const
{
    out << "  "; print(out); out << ": ";
    bool space = false;
    for (auto sym : follows) {
        if (space) {
            out << " ";
        } else {
            space = true;
        }
        sym->print(out);
    }
}

/******************************************************************************/
Nonterm::Rule::Rule(Nonterm* nonterm, const string& action):
    nonterm (nonterm),
    action  (action),
    id      (0){}

void
Nonterm::Rule::print(ostream& out) const
{
    out << nonterm->name << ":";

    bool space = false;
    for (auto sym : product) {
        if (space) {
            out << " ";
        } else {
            space = true;
        }
        sym->print(out);
    }
}

void
Nonterm::Rule::write(ostream& out) const {
    out << "rule" << id;
}

void
Nonterm::Rule::write_declare(ostream& out) const
{
    out << "Rule rule" << id;
    out << " = {" << product.size() << ", &";
    nonterm->write(out);
    
    out << ", ";
    if (!action.empty()) {
        out << "&" << action;
    } else {
        out << "nullptr";
    }
    out << "};\n";
}

void
Nonterm::Rule::write_proto(ostream& out) const
{
    out << "Value* ";
    out << action << "(vector<Value*>&);\n";
}

void
Nonterm::Rule::write_action(ostream& out) const
{
    if (!nonterm->type.empty()) {
        out << "unique_ptr<" << nonterm->type << "> ";
    } else {
        out << "void ";
    }
    
    out << action << "(";

    bool comma = false;
    for (auto sym : product) {
        if (!sym->type.empty()) {
            if (comma) {
                out << ", ";
            } else {
                comma = true;
            }
            out << "unique_ptr<" << sym->type << ">&";
        }
    }
    
    out << ");\n";
}

void
Nonterm::Rule::write_define(ostream& out) const
{
    out << "Value*\n";
    out << action << "(vector<Value*>& values) {\n";
    
    for (int i = 0; i < product.size(); i++) {
        int index = i - (int)product.size();
        if (!product[i]->type.empty()) {
            out << "    unique_ptr<" << product[i]->type << "> ";
            out << "E" << i;
            out << "(dynamic_cast<" << product[i]->type << "*>";
            out << "(values.end()[" << index << "]));\n";
        }
    }
    
    out << "    unique_ptr<" << nonterm->type << "> ";
    out << "R = "<< action << "(";

    bool comma = false;
    for (int i = 0; i < product.size(); i++) {
        if (!product[i]->type.empty()) {
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
