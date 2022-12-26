#include <Notechart.hpp>

bool Notechart::is_updated() {
    return this->updated;
}

void Notechart::update() {
    this->updated = false;
}

void Notechart::modify() {
    this->updated = true;
}