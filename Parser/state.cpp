#include "state.hpp"

using std::make_unique;

State::State(size_t id):
    id(id){}

void
State::add(State::Item item) {
    items.insert(item);
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
            vector<Symbol*> product = item.advance().rest();
            product.push_back(item.ahead);

            // TODO Check the calculation of terms.
            for (auto& rule : nonterm->rules) {
                set<Symbol*> terms;
                firsts(product, &terms);
                for (auto term : terms) {
                    Item next = Item(rule.get(), 0, term);
                    auto result = items.insert(next);
                    if (result.second) {
                        found.push_back(next);
                    }
                }
            }
        }
    }
}

void
State::firsts(const vector<Symbol*>& symbols, set<Symbol*>* firsts)
{
    for (Symbol* sym : symbols) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(sym);
        if (nonterm) {
            firsts->insert(nonterm->firsts.begin(), nonterm->firsts.end());
            if (!nonterm->empty_first) {
                return;
            }
        } else {
            firsts->insert(sym);
            return;
        }
    }
}

unique_ptr<State>
State::solve_next(Symbol* symbol)
{
    unique_ptr<State> state = make_unique<State>(0);
    for (auto item : items) {
        if (item.next() == symbol)
            state->items.insert(item.advance());
    }
    state->closure();
    if (state->items.size() == 0) {
        state.reset();
    }
    return state;
}

void
State::add_next(Symbol* symbol, State* next) {
    nexts[symbol] = next;
}

void
State::solve_actions(Item accept)
{
    // TODO Look for shift reduce conflicts.
    
    actions = make_unique<Actions>();

    for (auto item : items) {
        Symbol* term = item.next();
        if (term && !dynamic_cast<Nonterm*>(term)) {
            auto found = nexts.find(term);
            if (found != nexts.end()) {
                actions->shift[term] = found->second;
            }
        }
        else if (!item.next()) {
            if (item == accept) {
                actions->accept[item.ahead] = item.rule;
            } else {
                actions->reduce[item.ahead] = item.rule;
            }
        }
    }

    //  actions->reduce();
    //  auto found = previous.insert(std::move(actions));
    //  this->actions = found.first->get();
}

void
State::solve_gotos()
{
    for (auto next : nexts) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(next.first);
        if (nonterm) {
            gotos[nonterm] = next.second;
        }
    }
}

bool
State::operator<(const State& other) const {
    return items < other.items;
}

/******************************************************************************/
void
State::print(ostream& out) const
{
    out << "State " << id << "\n";
    for (auto& item : items) {
        item.print(out);
        out << "\n";
    }
    for (auto next : nexts) {
        next.first->print(out);
        out <<  " -> " << next.second->id << "\n";
    }
}

/******************************************************************************/
void
State::write(ostream& out) const {
    out << "state" << id;
}

void
State::write_declare(ostream& out) const {
    out << "extern State "; write(out); out << ";\n";
}

void
State::write_define(ostream& out) const
{
    out << "State "; write(out); out << " = {";
    if (actions->shift.size() > 0) {
        out << "shift" << id;
    } else {
        out << "nullptr";
    }
    if (actions->accept.size() > 0) {
        out << ", accept" << id;
    } else {
        out << ", nullptr";
    }
    if (actions->reduce.size() > 0) {
        out << ", reduce" << id;
    } else {
        out << ", nullptr";
    }
    if (gotos.size() > 0) {
        out << ", next" << id;
    } else {
        out << ", nullptr";
    }
    out << "};\n";
}

void
State::write_shift(ostream& out) const
{
    if (actions->shift.size() ==  0)
        return;

    out << "Shift shift" << id << "[] = {\n";
    for (auto& act : actions->shift) {
        out << "    {&";
        act.first->write(out);
        out << ", &";
        act.second->write(out);
        out << "},";
        out << " \\\\ ";
        act.first->print(out);
        out << " -> State " << id;
        out << "\n";
    }
    out << "};\n";
}

void
State::write_accept(ostream& out) const
{
    if (actions->accept.size() ==  0)
        return;
    
    out << "Accept accept" << id << "[] = {\n";
    for (auto& act : actions->accept) {
        out << "    {&";
        act.first->write(out);
        out << ", &rule" << act.second->id << "},";
        out << " \\\\ ";
        act.first->print(out);
        out << " -> Rule " << id;
        out << "\n";
    }
    out << "};\n";
}

void
State::write_reduce(ostream& out) const
{
    if (actions->reduce.size() ==  0)
        return;
    
    out << "Reduce reduce" << id << "[] = {\n";
    for (auto& act : actions->reduce) {
        out << "    {&";
        act.first->write(out);        
        out << ", &rule" << act.second->id << "},";
        out << " \\\\ ";
        act.first->print(out);
        out << " -> Rule " << id;
        out << "\n";
    }
    out << "};\n";
}

void
State::write_next(ostream& out) const
{
    if (gotos.size() ==  0)
        return;

    out << "Next next" << id << "[] = {\n";
    for (auto& next : gotos) {
        out << "    {&";
        next.first->write(out);
        out << ", &";
        next.second->write(out);
        out << "},\n";
    }
    out << "};\n";
}

/******************************************************************************/
State::Item::Item(Nonterm::Rule* rule, size_t mark, Symbol* ahead):
    rule    (rule),
    mark    (mark),
    ahead   (ahead){}

State::Item
State::Item::advance() {
    if (mark < rule->product.size()) {
        return Item(rule, mark + 1, ahead);
    } else {
        return Item(rule, mark, ahead);
    }
}

vector<Symbol*>
State::Item::rest() {
    vector<Symbol*> result;
    result.insert(result.end(),
                  rule->product.begin() + mark,
                  rule->product.end());
    return result;
}

Symbol*
State::Item::next() {
    if (mark < rule->product.size()) {
        return rule->product[mark];
    } else {
        return nullptr;
    }
}

Nonterm*
State::Item::next_nonterm() {
    if (mark < rule->product.size()) {
        return dynamic_cast<Nonterm*>(rule->product[mark]);
    } else {
        return nullptr;
    }
}

bool
State::Item::operator==(const Item& other) const {
    if (rule != other.rule) {
        return false;
    } else if (ahead != other.ahead) {
        return false;
    } else {
        return mark == other.mark;
    }
}

bool
State::Item::operator<(const Item& other) const {
    if (rule != other.rule) {
        return rule < other.rule;
    } else if (ahead != other.ahead) {
        return ahead < other.ahead;
    } else {
        return mark < other.mark;
    }
}

void
State::Item::print(ostream& out) const
{
    bool first = true;    
    for (size_t i = 0; i < rule->product.size(); i++) {
        if (i == mark) {
            if (!first) {
                out << " ";
            } else {
                first = false;
            }
            out << ".";
        }
        if (!first) {
            out << " ";
        } else {
            first = false;
        }
        rule->product[i]->print(out);
    }
    
    if (mark == rule->product.size()) {
        if (!first) {
            out << " ";
        } else {
            first = false;
        }
        out << ".";
    }
    
    out << " , ";
    ahead->print(out);
}
