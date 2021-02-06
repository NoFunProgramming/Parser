#include "symbols.hpp"

/******************************************************************************/
Symbol::Symbol(const string& name):
    id      (10),
    name    (name){}

Symbol Symbol::Endmark("endmark");

void
Symbol::print(ostream& out) const {
    out << "'" << name << "'";
}

void
Symbol::write(ostream& out) const {
    out << "term" << id;
}

/******************************************************************************/
Nonterm::Nonterm(const string& name):
    Symbol  (name),
    has_empty(false){}

void
Nonterm::add_rule(const vector<Symbol*>& syms, const string& reduce)
{
    rules.emplace_back(std::make_unique<Rule>(this));
    Rule* rule = rules.back().get();
    for (auto sym : syms) {
        rule->product.push_back(sym);
    }
    rule->reduce = reduce;
}

void Nonterm::print(ostream& out) const { out << name; }
void Nonterm::write(ostream& out) const { out << "nonterm" << id; }

void
Nonterm::write_rules(ostream& out) const
{
    for (auto& rule : rules) {
        rule->write_rule(out);
    }
}

void
Nonterm::print_rules(ostream& out) const
{
    for (auto& rule : rules) {
        if (rule.get() == rules.begin()->get()) {
            out << name << ": ";
        } else {
            out << "  | ";
        }
        for (auto sym : rule->product) {
            if (sym != rule->product.front()) {
                out << " ";
            }
            sym->print(out);
        }
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
    nonterm(nonterm), id(0){}

void
Nonterm::Rule::write_rule(ostream& out) const
{
    out << "unique_ptr<" << nonterm->type << "> " << reduce << "(";
    
    bool first = true;
    for (auto sym : product) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(sym);
        if (nonterm) {
            if (!first) {
                out << ", ";
            }
            out << "unique_ptr<" << nonterm->type << ">&";
            first = false;
        } else {
            if (!sym->type.empty()) {
                if (!first) {
                    out << ", ";
                }
                out << sym->type;
                first = false;
            }
        }
    }
    out << ");\n";
}
