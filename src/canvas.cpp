#define MINIAUDIO_IMPLEMENTATION
#include "../include/canvas.hpp"

#include <fmt/format.h>
#include <wx/numdlg.h>

#include <nlohmann/json.hpp>

#include "../third_party/miniaudio.h"

using json = nlohmann::json;

constexpr int COL_SIZE = 48;
constexpr int NOTE_SIZE = 3;

RenderTimer::RenderTimer(Canvas *pane) : wxTimer() { RenderTimer::pane = pane; }

void RenderTimer::Notify() { pane->Refresh(); }

void RenderTimer::start() { wxTimer::Start(10); }

BEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_MOTION(Canvas::mouseMove)
EVT_LEFT_DOWN(Canvas::mouseDown)
EVT_LEFT_UP(Canvas::mouseUp)
EVT_MOUSEWHEEL(Canvas::mouseWheel)
EVT_KEY_DOWN(Canvas::keyDown)
EVT_KEY_UP(Canvas::keyUp)
EVT_PAINT(Canvas::paintEvent)
END_EVENT_TABLE()

std::vector<int> drawable_x_cells = {1,  3,  4,  5,  6,  7, 9,
                                     10, 11, 12, 14, 15, 16};

std::vector<int> tick_granularity = {1,  2,  4,  6,  8,  12, 16,
                                     24, 32, 48, 64, 96, 192};

Canvas::Canvas(wxFrame *parent) : wxPanel(parent) {
    this->chart = std::make_unique<Notechart>();
    this->current_tick_double = 0;

    ma_result result;

    engine_config = ma_engine_config_init();

    result = ma_engine_init(&engine_config, &engine);
    if (result != MA_SUCCESS) {
        return;
    }

    result = ma_sound_init_from_file(
        &engine,
        "/Users/pnx/Documents/Project/Thapsteak/thapsteak_editor/audio/"
        "story_to_gaslight_your_child.mp3",
        0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        return;
    }

    this->is_init = true;
}

void Canvas::mouseMove(wxMouseEvent &event) {
    this->current_x = event.GetX();
    this->current_y = event.GetY();
}

void Canvas::keyDown(wxKeyEvent &event) {
    wxChar uc = event.GetUnicodeKey();

    if (uc != WXK_NONE) {
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
            // Change row size
            case ']': {
                if (this->current_row_size < 8) {
                    this->current_row_size++;
                }
                break;
            }
            case '[': {
                if (this->current_row_size > 1) {
                    this->current_row_size--;
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
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->direction = DIR_RIGHT;
                }
                break;
            }
            case 'X': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->direction = DIR_URIGHT;
                }
                break;
            }
            case 'C': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->direction = DIR_UP;
                }
                break;
            }
            case 'V': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->direction = DIR_ULEFT;
                }
                break;
            }
            case 'B': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->direction = DIR_LEFT;
                }
                break;
            }
            case 'N': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->direction = DIR_NONE;
                }
                break;
            }
            // Side
            case ',': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->side = SIDE_NONE;
                }
                this->current_side = SIDE_NONE;
                break;
            }
            case '.': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->side = SIDE_LEFT;
                }
                this->current_side = SIDE_LEFT;
                break;
            }
            case '/': {
                for (int note_id : this->highlighted_notes) {
                    std::shared_ptr<Note> ptr_to_note =
                        this->chart->note_index[note_id];
                    ptr_to_note->side = SIDE_RIGHT;
                }
                this->current_side = SIDE_RIGHT;
                break;
            }
            // Import
            case 'I': {
                wxFileDialog import_dialog(
                    this, _("Import JSON"), "", "",
                    "Thapsteak files (*.thapsteak)|*.thapsteak",
                    wxFD_OPEN | wxFD_FILE_MUST_EXIST);

                if (import_dialog.ShowModal() == wxID_CANCEL) {
                    break;
                }

                std::string file_path(import_dialog.GetPath());
                std::FILE *imported_file = std::fopen(file_path.c_str(), "rb");
                std::fseek(imported_file, 0, SEEK_END);
                long imported_file_size = std::ftell(imported_file);

                std::string buffer;
                buffer.resize(imported_file_size);

                std::rewind(imported_file);
                std::fread(buffer.data(), sizeof(uint32_t), imported_file_size,
                           imported_file);

                this->chart->notes.clear();

                json data = json::parse(buffer);
                for (auto &e : data["events"]) {
                    Note new_note(0, LANE_NONE, DIR_NONE, SIDE_NONE, false);

                    for (auto &[key, value] : e.items()) {
                        if (key == "row") {
                            new_note.tick = value;
                        } else if (key == "channel") {
                            if (value == "BPM") {
                                new_note.lane = LANE_BPM;
                            } else if (value == "H1") {
                                new_note.lane = LANE_H1;
                            } else if (value == "H2") {
                                new_note.lane = LANE_H2;
                            } else if (value == "H3") {
                                new_note.lane = LANE_H3;
                            } else if (value == "H4") {
                                new_note.lane = LANE_H4;
                            } else if (value == "H5") {
                                new_note.lane = LANE_H5;
                            } else if (value == "N1") {
                                new_note.lane = LANE_N1;
                            } else if (value == "N2") {
                                new_note.lane = LANE_N2;
                            } else if (value == "N3") {
                                new_note.lane = LANE_N3;
                            } else if (value == "N4") {
                                new_note.lane = LANE_N4;
                            } else if (value == "E1") {
                                new_note.lane = LANE_E1;
                            } else if (value == "E2") {
                                new_note.lane = LANE_E2;
                            } else if (value == "E3") {
                                new_note.lane = LANE_E3;
                            }
                        } else if (key == "longNote") {
                            new_note.is_longnote = value;
                        } else if (key == "angle") {
                            new_note.direction = value;
                        } else if (key == "side") {
                            if (value == "left") {
                                new_note.side = SIDE_LEFT;
                            } else if (value == "right") {
                                new_note.side = SIDE_RIGHT;
                            }
                        } else if (key == "value") {
                            new_note.value = value;
                        }
                    }

                    this->chart->add_note(new_note);
                }

                std::fclose(imported_file);
                break;
            }
            case 'S': {
                wxFileDialog export_dialog(
                    this, _("Export JSON"), "", "",
                    "Thapsteak files (*.thapsteak)|*.thapsteak",
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

                if (export_dialog.ShowModal() == wxID_CANCEL) {
                    break;
                }

                std::string content = this->chart->to_string();

                std::string file_path(export_dialog.GetPath());
                std::FILE *exported_file = std::fopen(file_path.c_str(), "w");
                std::fwrite(content.data(), sizeof(char), content.size(),
                            exported_file);
                std::fclose(exported_file);

                break;
            }
        }
    }

    switch (event.GetKeyCode()) {
        // Delete notes
        case WXK_BACK:
        case WXK_DELETE: {
            for (int note_id : this->highlighted_notes) {
                this->chart->notes.erase(
                    std::remove_if(this->chart->notes.begin(),
                                   this->chart->notes.end(),
                                   [note_id](const std::shared_ptr<Note> &n) {
                                       return note_id == n->id;
                                   }),
                    this->chart->notes.end());
            }
            break;
        }
        // Long note
        case WXK_SHIFT: {
            this->is_long_note = true;
            break;
        }
        // Autoplay
        case WXK_SPACE: {
            this->is_autoplay = !this->is_autoplay;

            if (this->is_autoplay) {
                // Play audio
                double beats = this->current_tick_double / 192 * 4;
                double milliseconds = (beats * 60.0 * 1000.0) / this->BPM;
                milliseconds -= offset * 1000.0;

                unsigned int sample_rate = ma_engine_get_sample_rate(&engine);
                double fpms = (double)sample_rate / (double)1000.0;

                ma_sound_seek_to_pcm_frame(&sound, (int)(fpms * milliseconds));

                // float seconds = 0.0;
                // ma_sound_get_cursor_in_seconds(&sound, &seconds);
                // printf("%f %f\n", milliseconds / 1000.0, seconds);

                ma_sound_start(&sound);
            } else {
                ma_sound_stop(&sound);
            }

            break;
        }
    }
}

void Canvas::keyUp(wxKeyEvent &event) {
    wxChar uc = event.GetUnicodeKey();

    if (uc != WXK_NONE) {
    }

    switch (event.GetKeyCode()) {
        // Long note
        case WXK_SHIFT: {
            this->is_long_note = false;
            break;
        }
    }
}

void Canvas::mouseDown(wxMouseEvent &event) {
    if (mode == Mode::MODE_CREATE) {
        int current_tick = (int)this->current_tick_double;

        int cell_range_in_ticks =
            192 / tick_granularity[tick_granularity_index];

        int current_mouse_column = this->current_x / COL_SIZE;

        if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(),
                      current_mouse_column) != drawable_x_cells.end()) {
            int absolute_ticks =
                (this->height - this->current_y) / this->current_row_size +
                current_tick;
            int absolute_ticks_with_granularity =
                (absolute_ticks / cell_range_in_ticks) * cell_range_in_ticks;
            Note new_note = Note(absolute_ticks_with_granularity,
                                 (Lane)current_mouse_column, DIR_NONE,
                                 this->current_side, false);

            new_note.is_longnote =
                (this->current_side != SIDE_NONE) && this->is_long_note;

            if ((Lane)current_mouse_column == LANE_BPM) {
                wxString prompt = wxGetTextFromUser("BPM", "", "");
                std::string str_prompt(prompt);
                float bpm = std::atof(str_prompt.c_str());
                new_note.value = bpm;
            }

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
    }
}

void Canvas::mouseWheel(wxMouseEvent &event) {
    if (event.GetWheelRotation() != 0) {
        this->is_autoplay = false;
        ma_sound_stop(&sound);
        this->current_tick_double += event.GetWheelRotation();
        if (this->current_tick_double < 0) this->current_tick_double = 0;
    }
}

void Canvas::paintEvent(wxPaintEvent &evt) {
    wxPaintDC dc(this);
    dc.GetSize(&(this->width), &(this->height));
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
    for (int screen_y = height; screen_y >= 0;
         screen_y -= this->current_row_size) {
        int y = screen_y - (current_tick * this->current_row_size);

        if (tick_granularity[tick_granularity_index] % 192 == 0) {
            if ((height - y) % (this->current_row_size) == 0) {
                dc.SetPen(wxPen(wxColor(0, 255, 0, 127), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 96 == 0) {
            if ((height - y) % (this->current_row_size * 2) == 0) {
                dc.SetPen(wxPen(wxColor(0, 255, 0, 127), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 64 == 0) {
            if ((height - y) % (this->current_row_size * 3) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 127), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 48 == 0) {
            if ((height - y) % (this->current_row_size * 4) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 127), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 32 == 0) {
            if ((height - y) % (this->current_row_size * 6) == 0) {
                dc.SetPen(wxPen(wxColor(0, 0, 255, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 24 == 0) {
            if ((height - y) % (this->current_row_size * 8) == 0) {
                dc.SetPen(wxPen(wxColor(0, 0, 255, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 16 == 0) {
            if ((height - y) % (this->current_row_size * 12) == 0) {
                dc.SetPen(wxPen(wxColor(0, 255, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 12 == 0) {
            if ((height - y) % (this->current_row_size * 16) == 0) {
                dc.SetPen(wxPen(wxColor(0, 255, 0, 255), 1));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 8 == 0) {
            if ((height - y) % (this->current_row_size * 24) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 2));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 6 == 0) {
            if ((height - y) % (this->current_row_size * 32) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 2));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 4 == 0) {
            if ((height - y) % (this->current_row_size * 48) == 0) {
                dc.SetPen(wxPen(wxColor(0, 0, 255, 255), 3));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 2 == 0) {
            if ((height - y) % (this->current_row_size * 96) == 0) {
                dc.SetPen(wxPen(wxColor(0, 255, 0, 255), 3));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }
        if (tick_granularity[tick_granularity_index] % 1 == 0) {
            if ((height - y) % (this->current_row_size * 192) == 0) {
                dc.SetPen(wxPen(wxColor(255, 0, 0, 255), 4));
                dc.DrawLine(0, screen_y, width, screen_y);
            }
        }

        if ((height - y - 96) % (this->current_row_size * 192) == 0) {
            dc.SetFont(wxFont{128, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD});
            dc.SetTextForeground(wxColor(224, 224, 224));
            dc.DrawText(
                wxT("#" +
                    fmt::format("{:03d}",
                                (height - y) / (this->current_row_size * 192))),
                width - 320, screen_y - 128 + 96);
        } else if ((height - y) % (this->current_row_size * 192) == 0) {
            dc.SetFont(wxFont{128, wxFONTFAMILY_SWISS, wxNORMAL, wxBOLD});
            dc.SetTextForeground(wxColor(224, 224, 224));
            dc.DrawText(
                wxT("#" +
                    fmt::format("{:03d}",
                                (height - y) / (this->current_row_size * 192))),
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
    for (int idx = 0; idx < this->chart->notes.size(); idx++) {
        std::shared_ptr<Note> note(this->chart->notes[idx]);

        int y_position =
            height - ((note->tick - current_tick) * this->current_row_size) -
            (NOTE_SIZE * 6);

        if (y_position + ((NOTE_SIZE * 6) + 1) >= 0 && y_position < height) {
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

            if (this->is_highlighted && x_position > x1 - (COL_SIZE + 1) &&
                x_position < x2 && y_position > y1 - ((NOTE_SIZE * 6) + 1) &&
                y_position < y2) {
                this->highlighted_notes.insert(note->id);
            }

            if (this->highlighted_notes.contains(note->id)) {
                dc.SetBrush(wxColor(128, 192, 128));
            }

            dc.SetPen(wxPen(wxColor(128, 128, 128), 1));
            dc.DrawRectangle(x_position, y_position, COL_SIZE + 1,
                             (NOTE_SIZE * 6) + 1);

            // Overlay
            if (note->lane == LANE_BPM) {
                dc.SetFont(wxFont{12, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL});
                dc.SetTextForeground(wxColor(255, 255, 255));
                dc.DrawText(wxT("" + fmt::format("{:.3f}", note->value)),
                            x_position, y_position + 3);
            } else {
                if (note->is_longnote) {
                    dc.SetFont(
                        wxFont{12, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL});
                    dc.SetTextForeground(wxColor(255, 255, 255));
                    dc.DrawText(wxT("DRAG"), x_position, y_position + 3);

                    // Draw LN line
                    // Find the previous connector (note with the same side)
                    for (int j = idx - 1; j >= 0; j--) {
                        std::shared_ptr<Note> prev(this->chart->notes[j]);

                        if (prev->side == note->side &&
                            this->chart->is_same_lane_group(prev, note)) {
                            int prev_y_position = height -
                                                  ((prev->tick - current_tick) *
                                                   this->current_row_size) -
                                                  (NOTE_SIZE * 6);
                            int prev_x_position = prev->lane * COL_SIZE;

                            dc.SetPen(wxPen(wxColor(128, 128, 128), 5));
                            dc.DrawLine(
                                x_position + ((COL_SIZE + 1) / 2),
                                y_position + (((NOTE_SIZE * 6) + 1) / 2),
                                prev_x_position + ((COL_SIZE + 1) / 2),
                                prev_y_position + (((NOTE_SIZE * 6) + 1) / 2));

                            break;
                        }
                    }
                }

                if (note->side != SIDE_NONE) {
                    // Check whether the next connector is out-of-screen
                    for (int j = idx + 1; j < this->chart->notes.size(); j++) {
                        std::shared_ptr<Note> next(this->chart->notes[j]);

                        if (next->side == note->side &&
                            this->chart->is_same_lane_group(next, note)) {
                            if (next->is_longnote) {
                                int next_y_position =
                                    height -
                                    ((next->tick - current_tick) *
                                     this->current_row_size) -
                                    (NOTE_SIZE * 6);

                                if (next_y_position < 0) {
                                    int next_x_position = next->lane * COL_SIZE;

                                    dc.SetPen(wxPen(wxColor(128, 128, 128), 5));
                                    dc.DrawLine(
                                        x_position + ((COL_SIZE + 1) / 2),
                                        y_position +
                                            (((NOTE_SIZE * 6) + 1) / 2),
                                        next_x_position + ((COL_SIZE + 1) / 2),
                                        next_y_position +
                                            (((NOTE_SIZE * 6) + 1) / 2));
                                }
                            }

                            break;
                        }
                    }
                }

                if (note->direction != DIR_NONE) {
                    dc.SetFont(
                        wxFont{32, wxFONTFAMILY_SWISS, wxNORMAL, wxNORMAL});
                    dc.SetTextForeground(wxColor(128, 128, 128));

                    switch (note->direction) {
                        case DIR_LEFT: {
                            dc.DrawRotatedText(wxT("➔"), x_position + 24,
                                               y_position - 9,
                                               180 - note->direction);
                            break;
                        }
                        case DIR_ULEFT: {
                            dc.DrawRotatedText(wxT("➔"), x_position + 15,
                                               y_position - 2,
                                               180 - note->direction);
                            break;
                        }
                        case DIR_UP: {
                            dc.DrawRotatedText(wxT("➔"), x_position + 10,
                                               y_position + 8,
                                               180 - note->direction);
                            break;
                        }
                        case DIR_URIGHT: {
                            dc.DrawRotatedText(wxT("➔"), x_position + 17,
                                               y_position + 17,
                                               180 - note->direction);
                            break;
                        }
                        case DIR_RIGHT: {
                            dc.DrawRotatedText(wxT("➔"), x_position + 24,
                                               y_position + 18,
                                               180 - note->direction);
                            break;
                        }
                    }
                }
            }
        }
    }

    if (mode == Mode::MODE_CREATE) {
        // Draw hovered notes
        int cell_height = 192 / tick_granularity[tick_granularity_index] *
                          this->current_row_size;
        int current_mouse_column = this->current_x / COL_SIZE;

        // Draw if in drawable x cells
        if (std::find(drawable_x_cells.begin(), drawable_x_cells.end(),
                      current_mouse_column) != drawable_x_cells.end()) {
            dc.SetPen(wxPen(wxColor(128, 128, 128), 1));

            // Note color
            switch (this->current_side) {
                case SIDE_LEFT:
                    dc.SetBrush(wxColor(255, 191, 191, 127));
                    break;
                case SIDE_RIGHT:
                    dc.SetBrush(wxColor(191, 191, 255, 127));
                    break;
                default:
                    dc.SetBrush(wxColor(191, 191, 191, 127));
            }

            // Calculate y-position of the hovered note
            //  - make it stick with the highest lower line
            dc.DrawRectangle(current_mouse_column * COL_SIZE,
                             this->current_y - (NOTE_SIZE * 3), COL_SIZE + 1,
                             NOTE_SIZE * 6);
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
    dc.DrawText(wxT("" + fmt::format("Mode: {:<}", ModeStr[this->mode])),
                width - 290, 20);
    dc.DrawText(
        wxT("" + fmt::format("Tick Granularity: {:3d}",
                             tick_granularity[this->tick_granularity_index])),
        width - 290, 40);
    dc.DrawText(
        wxT("" + fmt::format("Side: {:<}", side_text[this->current_side])),
        width - 290, 60);
    dc.DrawText(wxT("" + fmt::format("Row Size: {:d}", this->current_row_size)),
                width - 290, 80);

    // Compute time
    double beats = this->current_tick_double / 192 * 4;
    int milliseconds = (int)((1.0 / this->BPM) * beats * 60.0 * 1000.0);

    dc.DrawText(wxT("" + fmt::format("Current Time (ms): {:d}", milliseconds)),
                width - 290, 100);

    if (this->is_init) {
        float seconds = 0.0;
        ma_sound_get_cursor_in_seconds(&sound, &seconds);
        seconds += offset;

        if (this->is_autoplay) {
            current_tick_double = ((seconds / 60.0) * this->BPM) * (192.0 / 4.0);
        }

        dc.DrawText(
            wxT("" + fmt::format("Current Time (ms): {:d}", (int)(seconds * 1000.0))),
            width - 290, 120);
    }

    dc.SetPen(wxPen(wxColor(255, 255, 255, 127), 3));
    dc.DrawLine(0, height, width, height);
}
