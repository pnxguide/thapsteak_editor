#include <wx/wx.h>

#include <set>
#include "notechart.hpp"

enum Mode { MODE_POINTER, MODE_CREATE };
const std::vector<std::string> ModeStr{ "MODE_POINTER", "MODE_CREATE" };

class Canvas : public wxPanel {
   public:
    Canvas(wxFrame *parent);

    void paintEvent(wxPaintEvent &evt);
    void paintNow();
    void render(wxDC &dc);
    void update_frame(wxDC &dc, double delta_time);

    void mouseUp(wxMouseEvent &event);
    void mouseDown(wxMouseEvent &event);
    void mouseMove(wxMouseEvent &event);
    void mouseWheel(wxMouseEvent &event);
    void keyDown(wxKeyEvent &event);
    void keyUp(wxKeyEvent &event);

    std::unique_ptr<Notechart> chart;

    bool is_highlighted{false};
    int highlight_x{0}, highlight_y{0};
    std::set<int> highlighted_notes;

    int tick_granularity_index{0};
    Mode mode{Mode::MODE_POINTER};

    int current_x{0}, current_y{0};
    double current_tick_double{12.0};

    Side current_side{SIDE_NONE};

    bool is_long_note{false};

    std::chrono::time_point<std::chrono::steady_clock> latest_update_time;

    DECLARE_EVENT_TABLE()
};

class RenderTimer : public wxTimer {
    Canvas *pane;

   public:
    RenderTimer(Canvas *pane);
    void Notify();
    void start();
};