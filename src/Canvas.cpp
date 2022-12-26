#include <Canvas.hpp>

#include <algorithm>
#include <iostream>

BEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_MOTION(Canvas::mouseMove)
EVT_LEFT_DOWN(Canvas::mouseDown)
EVT_PAINT(Canvas::paintEvent)
END_EVENT_TABLE()

Canvas::Canvas(wxFrame *parent) : wxPanel(parent) {
    this->chart = std::make_unique<Notechart>();
    this->chart->modify();
}

void Canvas::mouseMove(wxMouseEvent &event) {
    wxPaintDC dc(this);

    wxPoint current_mouse_position = event.GetLogicalPosition(dc);
    this->current_x = current_mouse_position.x;
    this->current_y = current_mouse_position.y;
}

void Canvas::mouseDown(wxMouseEvent &event) { wxPaintDC dc(this); }

void Canvas::paintEvent(wxPaintEvent &evt) {
    wxPaintDC dc(this);
    render(dc);
}

void Canvas::paintNow() {
    wxClientDC dc(this);
    render(dc);
}

void Canvas::render(wxDC &dc) {
    wxCoord width, height;
    dc.GetSize(&width, &height);

    constexpr int COL_SIZE = 48;
    constexpr int ROW_SIZE = 16;

    // Clear the previous frame
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    // Draw lanes
    // BPM
    dc.SetPen(wxPen(wxColor(0, 0, 0, 0), 1));
    dc.SetBrush(wxColor(255, 224, 255));
    dc.DrawRectangle(COL_SIZE, 0, COL_SIZE + 1, height + 1);
    // Hard
    dc.SetPen(wxPen(wxColor(0, 0, 0, 0), 1));
    dc.SetBrush(wxColor(255, 224, 224));
    dc.DrawRectangle(COL_SIZE * 3, 0, (COL_SIZE * 5) + 1, height + 1);
    // Normal
    dc.SetPen(wxPen(wxColor(0, 0, 0, 0), 1));
    dc.SetBrush(wxColor(255, 255, 224));
    dc.DrawRectangle(COL_SIZE * 9, 0, (COL_SIZE * 4) + 1, height + 1);
    // Easy
    dc.SetPen(wxPen(wxColor(0, 0, 0, 0), 1));
    dc.SetBrush(wxColor(224, 255, 224));
    dc.DrawRectangle(COL_SIZE * 14, 0, (COL_SIZE * 3) + 1, height + 1);

    // Draw tables
    dc.SetPen(wxPen(wxColor(192, 192, 192), 1));
    for (int col = 0; col < 19; col++) {
        dc.DrawLine(col * COL_SIZE, 0, col * COL_SIZE, height);
    }
    for (int y = 0; y < height; y += ROW_SIZE) {
        dc.DrawLine(0, y, width, y);
    }

    // Draw hovered notes
    int current_x_cell = (this->current_x / COL_SIZE);
    int current_y_cell = (this->current_y / ROW_SIZE);
    // Draw if in drawable x cells
    std::vector<int> drawable_x_cells = {1,  3,  4,  5,  6,  7, 9,
                                         10, 11, 12, 14, 15, 16};
    if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(),
                           current_x_cell) != drawable_x_cells.end()) {
        dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
        dc.SetBrush(wxColor(224, 224, 224));
        dc.DrawRectangle(current_x_cell * COL_SIZE, current_y_cell * ROW_SIZE,
                         COL_SIZE + 1, ROW_SIZE + 1);
    }
}
