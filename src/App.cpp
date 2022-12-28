#include <fmt/format.h>
#include <wx/wx.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>

class Canvas;
class MyFrame;
class Note;
class Notechart;
class RenderTimer;

/**
 * TODO: Command stacks (for undo-ing)
 * TODO: Set BPM
 * TODO: Set granularity
 * TODO: Import/Export charts
 * TODO: Auto-play with spacebar
 * TODO: Add music (with single BPM)
 * TODO: Add music (with multiple BPM)
 */

class App : public wxApp {
    bool render_loop_on;
    void onIdle(wxIdleEvent &evt);
    virtual bool OnInit();

    MyFrame *frame;
    Canvas *drawPane;

   public:
    void activateRenderLoop(bool on);
};

wxIMPLEMENT_APP(App);

class MyFrame : public wxFrame {
    RenderTimer *timer;
    Canvas *drawPane;

   public:
    MyFrame();
    ~MyFrame();
    void OnClose(wxCloseEvent &evt);

    DECLARE_EVENT_TABLE()

   private:
    void OnHello(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
};

class Canvas : public wxPanel {
   public:
    Canvas(wxFrame *parent);

    void paintEvent(wxPaintEvent &evt);
    void paintNow();
    void render(wxDC &dc);
    void update_frame(wxDC &dc, double delta_time);

    void mouseDown(wxMouseEvent &event);
    void mouseMove(wxMouseEvent &event);
    void mouseWheel(wxMouseEvent &event);

    std::unique_ptr<Notechart> chart;

    int current_x{0}, current_y{0};
    double current_tick_double{12.0};

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

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_CLOSE(MyFrame::OnClose)
END_EVENT_TABLE()

bool App::OnInit() {
    frame = new MyFrame();
    frame->Show();
    return true;
}

MyFrame::MyFrame() : wxFrame(NULL, wxID_ANY, "Thapsteak Notecharter", wxPoint(50, 50), wxSize(640, 480)) {
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    drawPane = new Canvas(this);
    sizer->Add(drawPane, 1, wxEXPAND);
    SetSizer(sizer);

    timer = new RenderTimer(drawPane);
    Show();
    timer->start();
}

MyFrame::~MyFrame() { delete timer; }

void MyFrame::OnExit(wxCommandEvent &event) { Close(true); }

void MyFrame::OnAbout(wxCommandEvent &event) {
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnClose(wxCloseEvent &evt) {
    timer->Stop();
    evt.Skip();
}

BEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_MOTION(Canvas::mouseMove)
EVT_LEFT_DOWN(Canvas::mouseDown)
EVT_MOUSEWHEEL(Canvas::mouseWheel)
EVT_PAINT(Canvas::paintEvent)
END_EVENT_TABLE()

constexpr int COL_SIZE = 48;
constexpr int ROW_SIZE = 3;
constexpr int TICK_GRANULARITY = 4;

std::vector<int> drawable_x_cells = {1, 3, 4, 5, 6, 7, 9, 10, 11, 12, 14, 15, 16};

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

Note::Note(long _tick, Lane _lane, Direction _direction, Side _side, bool _is_longnote)
    : tick(_tick), lane(_lane), direction(_direction), side(_side), is_longnote(_is_longnote) {}

bool Notechart::is_updated() { return this->updated; }

void Notechart::update() { this->updated = false; }

void Notechart::modify() { this->updated = true; }

void Notechart::add_note(Note note) {
    if (std::find_if(this->notes.begin(), this->notes.end(), [note](const Note &n) {
            return (n.tick == note.tick) && (n.lane == note.lane);
        }) == this->notes.end()) {
        // Mark as modified
        this->modify();

        // Add and organize new note
        this->notes.push_back(note);
        std::sort(this->notes.begin(), this->notes.end(),
                  [](const Note &lhs, const Note &rhs) { return lhs.tick < rhs.tick; });
    }
}