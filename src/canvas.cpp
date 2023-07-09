#include "../include/canvas.hpp"

#include <fmt/format.h>
#include <wx/numdlg.h>

constexpr int COL_SIZE = 48;
constexpr int ROW_SIZE = 3;

RenderTimer::RenderTimer(Canvas *pane) : wxTimer() { RenderTimer::pane = pane; }

void RenderTimer::Notify() { pane->Refresh(); }

void RenderTimer::start() { wxTimer::Start(10); }

BEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_MOTION(Canvas::mouseMove)
EVT_LEFT_DOWN(Canvas::mouseDown)
EVT_LEFT_UP(Canvas::mouseUp)
EVT_MOUSEWHEEL(Canvas::mouseWheel)
EVT_KEY_DOWN(Canvas::keyDown)
EVT_PAINT(Canvas::paintEvent)
END_EVENT_TABLE()

std::vector<int> drawable_x_cells = {1,  3,  4,  5,  6,  7, 9,
                                     10, 11, 12, 14, 15, 16};

std::vector<int> tick_granularity = {1,  2,  4,  6,  8,  12, 16,
                                     24, 32, 48, 64, 96, 192};

Canvas::Canvas(wxFrame *parent) : wxPanel(parent) {
    this->chart = std::make_unique<Notechart>();
}

void Canvas::mouseMove(wxMouseEvent &event) {
    wxPaintDC dc(this);

    wxPoint current_mouse_position = event.GetLogicalPosition(dc);
    this->current_x = current_mouse_position.x;
    this->current_y = current_mouse_position.y;
}

void Canvas::keyDown(wxKeyEvent &event) {
    wxChar uc = event.GetUnicodeKey();
    if (uc != WXK_NONE) {
        printf("%c\n", uc);

        switch (uc) {
            // Change mode
            case 'Q': {
                mode = Mode::MODE_POINTER;
                break;
            }
            case 'W': {
                mode = Mode::MODE_CREATE;
                break;
            }
            // Change tick granularity
            case '=': {
                if (tick_granularity_index < tick_granularity.size() - 1) {
                    tick_granularity_index++;
                }
                break;
            }
            case '-': {
                if (tick_granularity_index > 0) {
                    tick_granularity_index--;
                }
                break;
            }
            // Warp
            case 'F': {
                long prompt =
                    wxGetNumberFromUser("Warp to", "", "", 0, 0, INT_MAX);
                this->current_tick_double = prompt * 192.0;
                break;
            }
            // Flick
            case 'Z': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note = this->chart->note_index[note_id];
                    ptr_to_note->direction = DIR_LEFT;
                    printf("%p %d %d\n", ptr_to_note.get(), ptr_to_note->id, ptr_to_note->direction);
                }
            }
        }
    }
}

void Canvas::mouseDown(wxMouseEvent &event) {
    if (mode == Mode::MODE_CREATE) {
        wxPaintDC dc(this);

        // Compute current tick
        // Assume the bottom line is 0 ticks
        wxCoord width, height;
        dc.GetSize(&width, &height);

        int current_tick = (int)this->current_tick_double;

        int cell_range_in_ticks =
            192 / tick_granularity[tick_granularity_index];

        int current_mouse_column = this->current_x / COL_SIZE;

        if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(),
                      current_mouse_column) != drawable_x_cells.end()) {
            int absolute_ticks =
                (height - this->current_y) / ROW_SIZE + current_tick;
            int absolute_ticks_with_granularity =
                (absolute_ticks / cell_range_in_ticks) * cell_range_in_ticks;
            Note new_note =
                Note(absolute_ticks_with_granularity,
                     (Lane)current_mouse_column, DIR_NONE, SIDE_NONE, false);

            if ((Lane)current_mouse_column == LANE_BPM) {
                wxString prompt = wxGetTextFromUser("BPM", "", "");
                std::string str_prompt(prompt);
                float bpm = std::atof(str_prompt.c_str());
                new_note.value = bpm;
            }

            printf("%p\n", &new_note);

            this->chart->add_note(new_note);
        }
    } else if (mode == Mode::MODE_POINTER) {
        this->is_highlighted = true;
        this->highlight_x = this->current_x;
        this->highlight_y = this->current_y;
    }
}

void Canvas::mouseUp(wxMouseEvent &event) {
    if (mode == Mode::MODE_CREATE) {
    } else if (mode == Mode::MODE_POINTER) {
        this->is_highlighted = false;
        // TODO: Compute all highlighted notes
        printf("(%d,%d) to (%d,%d)\n", this->highlight_x, this->highlight_y,
               this->current_x, this->current_y);
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
        int y = screen_y - (current_tick * ROW_SIZE);

        if (tick_granularity[tick_granularity_index] % 192 == 0) {
            if ((height - y) % (ROW_SIZE) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 96 == 0) {
            if ((height - y) % (ROW_SIZE * 2) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 64 == 0) {
            if ((height - y) % (ROW_SIZE * 3) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 48 == 0) {
            if ((height - y) % (ROW_SIZE * 4) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 32 == 0) {
            if ((height - y) % (ROW_SIZE * 6) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 24 == 0) {
            if ((height - y) % (ROW_SIZE * 8) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 16 == 0) {
            if ((height - y) % (ROW_SIZE * 12) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 12 == 0) {
            if ((height - y) % (ROW_SIZE * 16) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 8 == 0) {
            if ((height - y) % (ROW_SIZE * 24) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 6 == 0) {
            if ((height - y) % (ROW_SIZE * 32) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 4 == 0) {
            if ((height - y) % (ROW_SIZE * 48) == 0) {
                dc.SetPen(wxPen(wxColor(0, 0, 255, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 2 == 0) {
            if ((height - y) % (ROW_SIZE * 96) == 0) {
                dc.SetPen(wxPen(wxColor(0, 255, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 1 == 0) {
            if ((height - y) % (ROW_SIZE * 192) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }

        if ((height - y - 96) % (ROW_SIZE * 192) == 0) {
            dc.SetFont(wxFont{128, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD});
            dc.SetTextForeground(wxColor(224, 224, 224));
            dc.DrawText(wxT("#" + fmt::format("{:03d}",
                                              (height - y) / (ROW_SIZE * 192))),
                        width - 320, screen_y - 128 + 96);
        } else if ((height - y) % (ROW_SIZE * 192) == 0) {
            dc.SetFont(wxFont{128, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD});
            dc.SetTextForeground(wxColor(224, 224, 224));
            dc.DrawText(wxT("#" + fmt::format("{:03d}",
                                              (height - y) / (ROW_SIZE * 192))),
                        width - 320, screen_y - 128);
        }
    }

    // Reset the highlight set
    int x1, x2, y1, y2;
    if (this->is_highlighted) {
        this->highlighted_notes = std::set<int>();
        x1 = this->highlight_x;
        x2 = this->current_x;
        y1 = this->highlight_y;
        y2 = this->current_y;

        if (x1 > x2) {
            x2 = this->highlight_x;
            x1 = this->current_x;
        }

        if (y1 > y2) {
            y2 = this->highlight_y;
            y1 = this->current_y;
        }
    }

    // Render notes
    for (std::shared_ptr<Note> note : this->chart->notes) {
        int y_position = height - ((note->tick - current_tick + (6)) * ROW_SIZE);
        if (y_position >= 0 && y_position < height) {
            // The bottom line is (current_tick)
            int x_position = note->lane * COL_SIZE;

            // Note color
            switch (note->side) {
                case SIDE_LEFT:
                    dc.SetBrush(wxColor(255, 191, 191));
                    break;
                case SIDE_RIGHT:
                    dc.SetBrush(wxColor(191, 191, 255));
                    break;
                default:
                    dc.SetBrush(wxColor(191, 191, 191));
            }

            if (note->lane == LANE_BPM) {
                dc.SetBrush(wxColor(128, 128, 128));
            }

            if (this->is_highlighted &&
                x_position > x1 - (COL_SIZE + 1) && x_position < x2 &&
                y_position > y1 - ((ROW_SIZE * 6) + 1) && y_position < y2) {
                this->highlighted_notes.insert(note->id);
            }

            if (this->highlighted_notes.contains(note->id)) {
                dc.SetBrush(wxColor(192, 128, 128));
            }

            dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
            dc.DrawRectangle(x_position, y_position, COL_SIZE + 1,
                             (ROW_SIZE * 6) + 1);

            // Overlay
            if (note->lane == LANE_BPM) {
                dc.SetFont(wxFont{12, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL});
                dc.SetTextForeground(wxColor(255, 255, 255));
                dc.DrawText(wxT("" + fmt::format("{:.3f}", note->value)),
                            x_position, y_position + 3);
            } else {
                dc.SetFont(wxFont{12, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL});
                dc.SetTextForeground(wxColor(255, 255, 255));
                dc.DrawText(wxT("" + fmt::format("{:d} {:d}", note->id, note->direction)),
                            x_position, y_position + 3);
            }
        }
    }

    if (mode == Mode::MODE_CREATE) {
        // Draw hovered notes
        int cell_height =
            192 / tick_granularity[tick_granularity_index] * ROW_SIZE;
        int current_mouse_column = this->current_x / COL_SIZE;

        // Draw if in drawable x cells
        if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(),
                      current_mouse_column) != drawable_x_cells.end()) {
            dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
            dc.SetBrush(wxColor(224, 224, 224, 127));
            // Calculate y-position of the hovered note
            //  - make it stick with the highest lower line
            dc.DrawRectangle(current_mouse_column * COL_SIZE,
                             this->current_y - (ROW_SIZE * 3), COL_SIZE + 1,
                             ROW_SIZE * 6);
        }
    } else if (mode == Mode::MODE_POINTER) {
        // Highlight
        if (this->is_highlighted) {
            // Draw Highlighter
            dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
            dc.SetBrush(wxColor(255, 255, 224, 127));

            dc.DrawRectangle(x1, y1, x2 - x1, y2 - y1);
        }
    }

    // Draw GUI
    dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
    dc.SetBrush(wxColor(224, 224, 224, 127));
    dc.DrawRectangle(width - 300, 10, 290, 200);

    dc.SetFont(wxFont{16, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL});
    dc.SetTextForeground(wxColor(0, 0, 0));
    dc.DrawText(wxT("" + fmt::format("Mode: {:<}", ModeStr[mode])), width - 290,
                20);
    dc.DrawText(wxT("" + fmt::format("Tick Granularity: {:3d}",
                                     tick_granularity[tick_granularity_index])),
                width - 290, 40);

    int index = 0;
    for (std::shared_ptr<Note> note : this->chart->notes) {
        dc.DrawText(wxT("" + fmt::format("({:d} : {:d})",
                                     note->id, note->direction)),
                width - 290, 60 + 20 * (index++));
    }
}
