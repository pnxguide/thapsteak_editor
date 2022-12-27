#include <Canvas.hpp>

#include <algorithm>
#include <iostream>
#include <thread>

BEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_MOTION(Canvas::mouseMove)
EVT_LEFT_DOWN(Canvas::mouseDown)
EVT_PAINT(Canvas::paintEvent)
END_EVENT_TABLE()

constexpr int COL_SIZE = 48;
constexpr int ROW_SIZE = 3;
constexpr int TICK_GRANULARITY = 4;

RenderTimer::RenderTimer(Canvas *pane) : wxTimer() { RenderTimer::pane = pane; }

void RenderTimer::Notify() { pane->Refresh(); }

void RenderTimer::start() { wxTimer::Start(10); }

Canvas::Canvas(wxFrame *parent) : wxPanel(parent) {
    this->chart = std::make_unique<Notechart>();
    this->chart->modify();

    this->chart->add_note(Note(0, LANE_H1, DIR_NONE, SIDE_NONE, false));
    this->chart->add_note(Note(0, LANE_H3, DIR_NONE, SIDE_RIGHT, false));
    this->chart->add_note(Note(16, LANE_H5, DIR_NONE, SIDE_RIGHT, true));
    this->chart->add_note(Note(0, LANE_N1, DIR_NONE, SIDE_LEFT, false));
    this->chart->add_note(Note(16, LANE_N1, DIR_NONE, SIDE_LEFT, true));
    this->chart->add_note(Note(0, LANE_E1, DIR_UP, SIDE_NONE, false));
}

void Canvas::mouseMove(wxMouseEvent &event) {
    wxPaintDC dc(this);

    wxPoint current_mouse_position = event.GetLogicalPosition(dc);
    this->current_x = current_mouse_position.x;
    this->current_y = current_mouse_position.y;
}

void Canvas::mouseDown(wxMouseEvent &event) {
    wxPaintDC dc(this);

    // Compute current tick
    // Assume the bottom line is 0 ticks
}

void Canvas::paintEvent(wxPaintEvent &evt) {
    wxPaintDC dc(this);
    render(dc);
}

void Canvas::paintNow() {
    wxClientDC dc(this);
    render(dc);
}

void Canvas::render(wxDC &dc) {
    std::chrono::duration<double> delta_time =
        std::chrono::steady_clock::now() - this->latest_update_time;
    // 1 frame per second = 1 second per frame
    this->update_frame(dc, (1.0 / 60.0) - delta_time.count());

    this->latest_update_time = std::chrono::steady_clock::now();
}

void Canvas::update_frame(wxDC &dc, double delta_time) {
    wxCoord width, height;
    dc.GetSize(&width, &height);

    // Scroll
    int current_tick = (int)current_tick_double;

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

    // 1 row = 1/192 room
    // 1 note = 1/32 room
    for (int screen_y = height; screen_y >= 0; screen_y -= ROW_SIZE) {
        int y = screen_y - current_tick;

        switch (TICK_GRANULARITY) {
            case 192: {
                if ((height - y) % (ROW_SIZE) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 96: {
                if ((height - y) % (ROW_SIZE * 2) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 64: {
                if ((height - y) % (ROW_SIZE * 3) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 48: {
                if ((height - y) % (ROW_SIZE * 4) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 32: {
                if ((height - y) % (ROW_SIZE * 6) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 24: {
                if ((height - y) % (ROW_SIZE * 8) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 16: {
                if ((height - y) % (ROW_SIZE * 12) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 12: {
                if ((height - y) % (ROW_SIZE * 16) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 8: {
                if ((height - y) % (ROW_SIZE * 24) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 6: {
                if ((height - y) % (ROW_SIZE * 32) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 4: {
                if ((height - y) % (ROW_SIZE * 48) == 0) {
                    dc.SetPen(wxPen(wxColor(0, 0, 255, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 2: {
                if ((height - y) % (ROW_SIZE * 96) == 0) {
                    dc.SetPen(wxPen(wxColor(0, 255, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
            case 1: {
                if ((height - y) % (ROW_SIZE * 192) == 0) {
                    dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                    dc.DrawLine(0, screen_y, width, screen_y);
                }
            }
        }
    }

    // Render notes
    for (Note note : this->chart->notes) {
        // The bottom line is (current_tick)
        int x_position = note.lane * COL_SIZE;
        int y_position = height - ((note.tick - current_tick + (6)) * ROW_SIZE);

        switch (note.side) {
            case SIDE_LEFT:
                dc.SetBrush(wxColor(192, 128, 128));
                break;
            case SIDE_RIGHT:
                dc.SetBrush(wxColor(128, 128, 192));
                break;
            default:
                dc.SetBrush(wxColor(128, 128, 128));
        }
        dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
        dc.DrawRectangle(x_position, y_position, COL_SIZE + 1,
                         (ROW_SIZE * 6) + 1);
    }

    // Draw hovered notes
    int cell_height = (ROW_SIZE * (192 / TICK_GRANULARITY));
    int current_x_cell = this->current_x / COL_SIZE;
    int current_y_cell =
        (this->current_y - (height % cell_height)) / cell_height + 1;
    if (this->current_y < (height % cell_height)) {
        current_y_cell = 0;
    }
    // Draw if in drawable x cells
    std::vector<int> drawable_x_cells = {1,  3,  4,  5,  6,  7, 9,
                                         10, 11, 12, 14, 15, 16};
    if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(),
                  current_x_cell) != drawable_x_cells.end()) {
        dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
        dc.SetBrush(wxColor(224, 224, 224));
        dc.DrawRectangle(current_x_cell * COL_SIZE,
                         (current_y_cell * cell_height) - (ROW_SIZE * 6) +
                             (height % cell_height),
                         COL_SIZE + 1, (ROW_SIZE * 6) + 1);
    }
}
