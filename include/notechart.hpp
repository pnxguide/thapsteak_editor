#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum Direction {
    DIR_NONE = -1,
    DIR_RIGHT = 0,
    DIR_URIGHT = 45,
    DIR_UP = 90,
    DIR_ULEFT = 135,
    DIR_LEFT = 180
};
enum Side { SIDE_NONE, SIDE_LEFT, SIDE_RIGHT };
const std::vector<std::string> side_text{"null", "left", "right"};

enum Lane {
    LANE_NONE = 0,
    LANE_BPM = 1,
    LANE_H1 = 3,
    LANE_H2 = 4,
    LANE_H3 = 5,
    LANE_H4 = 6,
    LANE_H5 = 7,
    LANE_N1 = 9,
    LANE_N2 = 10,
    LANE_N3 = 11,
    LANE_N4 = 12,
    LANE_E1 = 14,
    LANE_E2 = 15,
    LANE_E3 = 16
};
const std::vector<std::string> lane_text{"",   "BPM", "",   "H1", "H2", "H3",
                                         "H4", "H5",  "",   "N1", "N2", "N3",
                                         "N4", "",    "E1", "E2", "E3"};

class Note {
   public:
    long id;
    long tick;
    Lane lane;
    Direction direction{DIR_NONE};
    Side side{SIDE_NONE};
    bool is_longnote{false};
    float value{0.0};

    Note(long _tick, Lane _lane, Direction _direction, Side _side,
         bool _is_longnote);

    std::string to_string();
};

class Notechart {
   public:
    bool is_updated();
    void modify();
    void update();
    void add_note(Note note);

    std::vector<std::shared_ptr<Note>> notes;
    std::unordered_map<int, std::shared_ptr<Note>> note_index;

    bool is_same_lane_group(std::shared_ptr<Note> a, std::shared_ptr<Note> b);

    std::string to_string();

   private:
    int current_sequence{0};
    bool updated{false};
};
