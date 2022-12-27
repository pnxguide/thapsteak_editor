#include <vector>

#include <Note.hpp>

class Notechart {
   public:
    bool is_updated();
    void modify();
    void update();
    void add_note(Note note);

    std::vector<Note> notes;


   private:
    bool updated{false};
};