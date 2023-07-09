#include "../include/notechart.hpp"

#include <memory>

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

bool Notechart::is_same_lane_group(std::shared_ptr<Note> a,
                                   std::shared_ptr<Note> b) {
    if (a->lane >= 3 && a->lane <= 7) {
        return b->lane >= 3 && b->lane <= 7;
    }

    else if (a->lane >= 9 && a->lane <= 12) {
        return b->lane >= 9 && b->lane <= 12;
    }

    else if (a->lane >= 14 && a->lane <= 16) {
        return b->lane >= 14 && b->lane <= 16;
    }

    return false;
}

void Notechart::add_note(Note note) {
    // Deduplicate
    if (std::find_if(this->notes.begin(), this->notes.end(),
                     [note](const std::shared_ptr<Note> &n) {
                         return (n->tick == note.tick) &&
                                (n->lane == note.lane);
                     }) == this->notes.end()) {
        // Mark as modified
        this->modify();

        // Assign note ID
        note.id = current_sequence++;
        // Index the note
        this->note_index[note.id] = std::make_shared<Note>(note);

        // Add and organize new note
        this->notes.push_back(this->note_index[note.id]);
        std::sort(this->notes.begin(), this->notes.end(),
                  [](const std::shared_ptr<Note> &lhs,
                     const std::shared_ptr<Note> &rhs) {
                      return lhs->tick < rhs->tick;
                  });
    }
}