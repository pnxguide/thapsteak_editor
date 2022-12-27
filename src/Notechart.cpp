#include <Notechart.hpp>

#include <algorithm>

Note::Note(long _tick, Lane _lane, Direction _direction, Side _side,
           bool _is_longnote)
    : tick(_tick),
      lane(_lane),
      direction(_direction),
      side(_side),
      is_longnote(_is_longnote) {}

bool Notechart::is_updated() { return this->updated; }

void Notechart::update() { this->updated = false; }

void Notechart::modify() { this->updated = true; }

void Notechart::add_note(Note note) {
    if (std::find_if(this->notes.begin(), this->notes.end(),
                     [note](const Note &n) {
                         return (n.tick == note.tick) && (n.lane == note.lane);
                     }) == this->notes.end()) {
        // Mark as modified
        this->modify();

        // Add and organize new note
        this->notes.push_back(note);
        std::sort(this->notes.begin(), this->notes.end(),
                  [](const Note &lhs, const Note &rhs) {
                      return lhs.tick < rhs.tick;
                  });
    }
}