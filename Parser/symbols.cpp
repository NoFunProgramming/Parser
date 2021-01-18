#include "symbols.hpp"

/******************************************************************************/
Term::Term(const string& name):
    name(name){}

void
Term::print(ostream& out) const {
    out << "'" << name << "'";
}

/******************************************************************************/
Nonterm::Nonterm(const string& name):
    name(name) {}

Nonterm::Rule*
Nonterm::add_rule() {
    rules.emplace_back(std::make_unique<Rule>());
    return rules.back().get();
}

void
Nonterm::print(ostream& out) const
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

/******************************************************************************/
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
