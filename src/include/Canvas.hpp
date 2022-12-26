#include <memory>
#include <wx/wx.h>

#include <Notechart.hpp>

class Canvas : public wxPanel {
   public:
    Canvas(wxFrame *parent);

    void paintEvent(wxPaintEvent& evt);
    void paintNow();
    void render(wxDC &dc);

    void mouseDown(wxMouseEvent &event);
    void mouseMove(wxMouseEvent &event);

    std::unique_ptr<Notechart> chart;

    int current_x{0}, current_y{0};

    DECLARE_EVENT_TABLE()
};
