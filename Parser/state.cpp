#include "state.hpp"

using std::make_unique;

State::State(size_t id):
    id(id),
    name("state"){}

const string&
State::get_ident() const {
    return name;
}

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
            for (auto& rule : nonterm->rules) {
                vector<Symbol*> product;
                vector<Symbol*> check = item.advance().rest();
                product.insert(product.end(), check.begin(), check.end());
                product.push_back(item.ahead);

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

unique_ptr<State>
State::solve_next(Symbol* symbol)
{
    // TODO Set the identifier.
    unique_ptr<State> state = make_unique<State>(0);
    for (auto item : items) {
        if (item.is_next(symbol))
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
        Term* term = item.next_term();
        if (term) {
            auto found = nexts.find(term);
            if (found != nexts.end()) {
                actions->shift[term] = found->second;
            }
        }
        else if (item.is_end()) {
            if (item == accept) {
                actions->accept[&Symbol::Endmark] = item.rule;
            } else {
                actions->reduce[item.ahead] = item.rule;
            }
        }
    }

    //actions->reduce();
    //    auto found = previous.insert(std::move(actions));
    //    this->actions = found.first->get();
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

void
State::write(ostream& out) const
{
    out << "state" << id;
}

void
State::write_declare(ostream& out) const
{
    out << "extern State state"  << id << ";\n";
}

void
State::write_define(ostream& out) const
{
//    if (actions->shift.size() > 0) {
//        out << "Shift shift" << id << "[];\n";
//    }
//    if (actions->accept.size() > 0) {
//        out << "Rule* accept" << id << "(Symbol* sym);\n";
//    }
//    if (actions->reduce.size() > 0) {
//        out << "Rule* reduce" << id << "(Symbol* sym);\n";
//    }
//    if (nexts.size() > 0) {
//        out << ", &next" << id;
//    }
//
    out << "State state"  << id << " = {";
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
    if (nexts.size() > 0) {
        out << ", next" << id;
    } else {
        out << ", nullptr";
    }
    out << "};\n";
}


void
State::firsts(const vector<Symbol*>& symbols, set<Symbol*>* firsts)
{
    for (Symbol* sym : symbols) {
        Nonterm* nonterm = dynamic_cast<Nonterm*>(sym);
        if (nonterm) {
            firsts->insert(nonterm->firsts.begin(), nonterm->firsts.end());
            if (!nonterm->has_empty) {
                return;
            }
        } else {
            firsts->insert(sym);
            return;
        }
    }
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
        out << "},\n";
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
        out << ", &";
        act.second->write(out);
        out << "},\n";
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
        out << ", &";
        act.second->write(out);
        out << "},\n";
    }
    out << "};\n";
}

void
State::write_next(ostream& out) const
{
    if (nexts.size() ==  0)
        return;

    out << "Next next" << id << "[] = {\n";
    for (auto& next : nexts) {
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
    if (mark == rule->product.size()) {
        throw std::exception();
    }
    return Item(rule, mark + 1, ahead);
}

vector<Symbol*>
State::Item::rest() {
    vector<Symbol*> result;
    result.insert(result.end(),
                  rule->product.begin() + mark,
                  rule->product.end());
    return result;
}

bool
State::Item::is_end() {
    return rule->product.size() == mark;
}

bool
State::Item::is_next(Symbol* symbol) {
    if (mark < rule->product.size()) {
        return symbol == rule->product[mark];
    } else {
        return false;
    }
}

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
State::Item::operator==(const Item& other) const
{
    if (rule != other.rule) {
        return false;
    }
    else if (ahead != other.ahead) {
        return false;
    }
    else {
        return mark == other.mark;
    }
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

