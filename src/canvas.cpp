#include "../include/canvas.hpp"

#include <fmt/format.h>

constexpr int COL_SIZE = 48;
constexpr int ROW_SIZE = 3;
constexpr int TICK_GRANULARITY = 4;

enum Mode { MODE_POINTER, MODE_CREATE, MODE_DELETE };

RenderTimer::RenderTimer(Canvas *pane) : wxTimer() { RenderTimer::pane = pane; }

void RenderTimer::Notify() { pane->Refresh(); }

void RenderTimer::start() { wxTimer::Start(10); }

BEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_MOTION(Canvas::mouseMove)
EVT_LEFT_DOWN(Canvas::mouseDown)
EVT_MOUSEWHEEL(Canvas::mouseWheel)
EVT_PAINT(Canvas::paintEvent)
END_EVENT_TABLE()

std::vector<int> drawable_x_cells = {1, 3, 4, 5, 6, 7, 9, 10, 11, 12, 14, 15, 16};

Canvas::Canvas(wxFrame *parent) : wxPanel(parent) {
    this->chart = std::make_unique<Notechart>();
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
    wxCoord width, height;
    dc.GetSize(&width, &height);

    int current_tick = (int)this->current_tick_double;

    int cell_range_in_ticks = 192 / TICK_GRANULARITY;

    int current_mouse_column = this->current_x / COL_SIZE;

    if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(), current_mouse_column) != drawable_x_cells.end()) {
        int absolute_ticks = (height - this->current_y) / ROW_SIZE + current_tick;
        int absolute_ticks_with_granularity = (absolute_ticks / cell_range_in_ticks) * cell_range_in_ticks;
        this->chart->add_note(
            Note(absolute_ticks_with_granularity, (Lane)current_mouse_column, DIR_NONE, SIDE_NONE, false));
    }
}

void Canvas::mouseWheel(wxMouseEvent &event) {
    wxPaintDC dc(this);

    this->current_tick_double += event.GetWheelRotation();
    if (this->current_tick_double < 0) this->current_tick_double = 0;
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
    std::chrono::duration<double> delta_time = std::chrono::steady_clock::now() - this->latest_update_time;
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
        int y = screen_y - (current_tick * ROW_SIZE);

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

                if ((height - y - 96) % (ROW_SIZE * 192) == 0) {
                    dc.SetFont(wxFont{128, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD});
                    dc.SetTextForeground(wxColor(224, 224, 224));
                    dc.DrawText(wxT("#" + fmt::format("{:03d}", (height - y) / (ROW_SIZE * 192))), width - 320,
                                screen_y - 128 + 96);
                } else if ((height - y) % (ROW_SIZE * 192) == 0) {
                    dc.SetFont(wxFont{128, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD});
                    dc.SetTextForeground(wxColor(224, 224, 224));
                    dc.DrawText(wxT("#" + fmt::format("{:03d}", (height - y) / (ROW_SIZE * 192))), width - 320,
                                screen_y - 128);
                }
            }
        }
    }

    // Render notes
    for (Note note : this->chart->notes) {
        int y_position = height - ((note.tick - current_tick + (6)) * ROW_SIZE);
        if (y_position >= 0 && y_position < height) {
            // The bottom line is (current_tick)
            int x_position = note.lane * COL_SIZE;

            switch (note.side) {
                case SIDE_LEFT:
                    dc.SetBrush(wxColor(255, 191, 191));
                    break;
                case SIDE_RIGHT:
                    dc.SetBrush(wxColor(191, 191, 255));
                    break;
                default:
                    dc.SetBrush(wxColor(191, 191, 191));
            }
            dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
            dc.DrawRectangle(x_position, y_position, COL_SIZE + 1, (ROW_SIZE * 6) + 1);
        }
    }

    // Draw hovered notes
    int cell_height = 192 / TICK_GRANULARITY * ROW_SIZE;
    int current_mouse_column = this->current_x / COL_SIZE;

    // Draw if in drawable x cells
    if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(), current_mouse_column) != drawable_x_cells.end()) {
        dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
        dc.SetBrush(wxColor(224, 224, 224, 127));
        // Calculate y-position of the hovered note
        //  - make it stick with the highest lower line
        dc.DrawRectangle(current_mouse_column * COL_SIZE, this->current_y - (ROW_SIZE * 3),
                         // ((this->current_y - ((current_tick % (192 / TICK_GRANULARITY)) * ROW_SIZE)) /
                         //  cell_height * cell_height) +
                         //     (cell_height - (ROW_SIZE * 6)) +
                         //     ((height - (cell_height - ((current_tick % (192 / TICK_GRANULARITY)) * ROW_SIZE))) %
                         //      cell_height),
                         COL_SIZE + 1, ROW_SIZE * 6);
    }
}
