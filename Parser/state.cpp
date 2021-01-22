#include "state.hpp"

using std::make_unique;

void
State::add(State::Item item) {
    items.insert(item);
}

void
State::add_next(Symbol* symbol, State* next) {
    nexts[symbol] = next;
}


void
State::closure()
{
    vector<Item> found;
    std::copy(items.begin(), items.end(), std::back_inserter(found));

    while (found.size() > 0)
    {
        Item item = found.back();
        found.pop_back();

        Nonterm* nonterm = item.next_nonterm();
        if (nonterm) {
//            for (auto& rule : nonterm->rules) {
//                vector<Symbol> product;
//                vector<Symbol> check = item.advance().rest();
//                product.insert(product.end(), check.begin(), check.end());
//                product.push_back(item.get_ahead());
//
//                set<Symbol> terms;
//                Symbol::firsts(product, &terms);
//                for (auto term : terms) {
//                    Item next = Item(rule.get(), 0, term);
//                    auto result = items.insert(next);
//                    if (result.second) {
//                        found.push_back(next);
//                    }
//                }
//            }
        }
    }
}

unique_ptr<State>
State::solve_next(Symbol* symbol)
{
    unique_ptr<State> state = make_unique<State>();
    for (auto item : items) {
//        if (item.is_next(symbol))
//            state->items.insert(item.advance());
    }
    state->closure();
    if (state->items.size() == 0) {
        state.reset();
    }
    return state;
}

/******************************************************************************/
State::Item::Item(Nonterm::Rule* rule, size_t mark, Symbol* ahead):
    rule    (rule),
    mark    (mark),
    ahead   (ahead){}

Term*
State::Item::next_term()
{
    if (mark < rule->product.size()) {
        Symbol* symbol = rule->product[mark];
        return dynamic_cast<Term*>(symbol);
    }
    return nullptr;
}

Nonterm*
State::Item::next_nonterm()
{
    if (mark < rule->product.size()) {
        Symbol* symbol = rule->product[mark];
        return dynamic_cast<Nonterm*>(symbol);
    }
    return nullptr;
}


bool
State::Item::operator<(const Item& other) const
{
    if (rule != other.rule) {
        return rule < other.rule;
    }
    else if (ahead != other.ahead) {
        return ahead < other.ahead;
    }
    else {
        return mark < other.mark;
    }
}
