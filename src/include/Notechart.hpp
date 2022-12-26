#include <vector>

#include <Note.hpp>

class Notechart {
   public:
    bool is_updated();
    void modify();
    void update();

   private:
    std::vector<Note> notes;
    bool updated{false};
};