enum Direction {
    DIR_NONE = -1,
    DIR_RIGHT = 0,
    DIR_URIGHT = 45,
    DIR_UP = 90,
    DIR_ULEFT = 135,
    DIR_LEFT = 180
};
enum Side { SIDE_NONE, SIDE_LEFT, SIDE_RIGHT };
enum Lane {
    LANE_BPM,
    LANE_H1,
    LANE_H2,
    LANE_H3,
    LANE_H4,
    LANE_H5,
    LANE_N1,
    LANE_N2,
    LANE_N3,
    LANE_N4,
    LANE_E1,
    LANE_E2,
    LANE_E3
};

class Note {
   public:
    long tick;
    Lane lane;
    Direction direction{DIR_NONE};
    Side side{SIDE_NONE};
    bool is_longnote{false};
};