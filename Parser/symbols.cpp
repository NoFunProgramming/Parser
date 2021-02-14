#include "symbols.hpp"

/******************************************************************************/
Term::Term(const string& name, size_t rank):
    name(name),
    type(""),
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
Term::write_declare(ostream& out) const {
    out << "term" << rank;
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
    type(""),
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
Nonterm::Rule::write_action(ostream& out) const
{
    if (!nonterm->type.empty()) {
        out << "unique_ptr<" << nonterm->type << ">";
    } else {
        out << "void";
    }
    out << " " << action << "(";
    
    bool comma = false;
    for (auto sym : product) {
        Term* term = dynamic_cast<Term*>(sym);
        if (term && !term->type.empty()) {
            if (comma) {
                out << ", ";
            } else {
                comma = true;
            }
            out << term->type;
        }
        Nonterm* nonterm = dynamic_cast<Nonterm*>(sym);
        if (nonterm && !nonterm->type.empty()) {
            if (comma) {
                out << ", ";
            } else {
                comma = true;
            }
            out << "unique_ptr<" << nonterm->type << ">&";
        }
    }
    out << ");\n";
}
