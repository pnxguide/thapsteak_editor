#include <vector>

enum Direction { DIR_NONE = -1, DIR_RIGHT = 0, DIR_URIGHT = 45, DIR_UP = 90, DIR_ULEFT = 135, DIR_LEFT = 180 };
enum Side { SIDE_NONE, SIDE_LEFT, SIDE_RIGHT };
enum Lane {
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

class Note {
   public:
    long tick;
    Lane lane;
    Direction direction{DIR_NONE};
    Side side{SIDE_NONE};
    bool is_longnote{false};

    Note(long _tick, Lane _lane, Direction _direction, Side _side, bool _is_longnote);
};

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
