#include "symbols.hpp"

/******************************************************************************/
Symbol::Symbol(const string& name):
    id      (10),
    name    (name){}

Symbol Symbol::Endmark("endmark");

/******************************************************************************/
Term::Term(const string& name):
    Symbol  (name),
    id      (0){}

void
Term::print(ostream& out) const {
    out << "'" << name << "'";
}

void
Term::write(ostream& out) const {
    out << "term" << id;
}

/******************************************************************************/
Nonterm::Nonterm(const string& name):
    id      (0),
    Symbol  (name),
    has_empty(false){}

void
Nonterm::add_rule(const vector<Symbol*>& syms)
{
    Rule* rule = add_rule();
    for (auto sym : syms) {
        rule->product.push_back(sym);
    }
}

Nonterm::Rule*
Nonterm::add_rule() {
    rules.emplace_back(std::make_unique<Rule>(this));
    return rules.back().get();
}

void Nonterm::print(ostream& out) const { out << name; }
void Nonterm::write(ostream& out) const { out << "nonterm" << id; }

void
Nonterm::print_rules(ostream& out) const
{
    for (auto& rule : rules) {
        if (rule.get() == rules.begin()->get()) {
            out << name << ": ";
        } else {
            out << "  | ";
        }
        rule->print(out);
        out << "\n";
    }
}

void
Nonterm::print_firsts(ostream& out) const
{
    print(out); out << ": ";
    for (auto sym : firsts) {
        sym->print(out);
    }
}

void
Nonterm::print_follows(ostream& out) const
{
    print(out); out << ": ";
    for (auto sym : follows) {
        sym->print(out);
    }
}

void
Nonterm::solve_first(bool *found)
{
    size_t before = firsts.size();
    bool before_eps = has_empty;
    for (auto& rule : rules) {
        insert_firsts(rule->product.begin(), rule->product.end());
    }
    if ((firsts.size() > before) || (has_empty && !before_eps)) {
        *found = true;
    }
}

void
Nonterm::solve_follows(bool *found)
{
    for (auto& rule : rules) {
        auto sym = rule->product.begin();
        for (; sym < rule->product.end(); sym++ ) {
            Nonterm* nonterm = dynamic_cast<Nonterm*>(*sym);
            if (nonterm) {
                size_t before = nonterm->follows.size();

                bool epsilon = false;
                nonterm->insert_follows(sym + 1, rule->product.end(), &epsilon);
                if (epsilon) {
                    nonterm->follows.insert(rule->nonterm->follows.begin(),
                                            rule->nonterm->follows.end());
                }
                if (nonterm->follows.size() > before) {
                    *found = true;
                }
            }
        }
    }
}

void
Nonterm::insert_follows(Symbol* symbol) {
    follows.insert(symbol);
}

void
Nonterm::insert_firsts(vector<Symbol*>::iterator symbol,
                       vector<Symbol*>::iterator end)
{
    for (; symbol < end; symbol++) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(*symbol);
        if (nonterm) {
            firsts.insert(nonterm->firsts.begin(), nonterm->firsts.end());
            if (!nonterm->has_empty)
                return;
        } else {
            firsts.insert(*symbol);
            return;
        }
    }
    has_empty = true;
}

void
Nonterm::insert_follows(vector<Symbol*>::iterator symbol,
                        vector<Symbol*>::iterator end,
                        bool* epsilon)
{
    for (; symbol < end; symbol++) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(*symbol);
        if (nonterm) {
            follows.insert(nonterm->firsts.begin(), nonterm->firsts.end());
            if (!nonterm->has_empty)
                return;
        } else {
            follows.insert(*symbol);
            return;
        }
    }
    *epsilon = true;
}

/******************************************************************************/
Nonterm::Rule::Rule(Nonterm* nonterm):
    id(0),
    nonterm(nonterm){}

void
Nonterm::Rule::add(Symbol* sym) {
    product.push_back(sym);
}

void
Nonterm::Rule::print(ostream& out) const
{
    for (auto sym : product) {
        if (sym != product.front()) {
            out << " ";
        }
        sym->print(out);
    }
}

void
Nonterm::Rule::write(ostream& out) const {
    out << "rule" << id;
}
