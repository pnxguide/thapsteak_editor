#include <chrono>
#include <memory>
#include <wx/wx.h>

#include <Notechart.hpp>

class Canvas : public wxPanel {
   public:
    Canvas(wxFrame *parent);

    void paintEvent(wxPaintEvent& evt);
    void paintNow();
    void render(wxDC &dc);
    void update_frame(wxDC &dc, double delta_time);

    void mouseDown(wxMouseEvent &event);
    void mouseMove(wxMouseEvent &event);

    std::unique_ptr<Notechart> chart;

    int current_x{0}, current_y{0};
    double current_tick_double{0.0};

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