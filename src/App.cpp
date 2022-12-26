#include <App.hpp>

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_CLOSE(MyFrame::OnClose)
END_EVENT_TABLE()

bool App::OnInit() {
    render_loop_on = false;

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    frame = new MyFrame();

    drawPane = new Canvas(frame);
    sizer->Add(drawPane, 1, wxEXPAND);

    frame->SetSizer(sizer);
    frame->Show();

    activateRenderLoop(true);
    return true;
}

void App::activateRenderLoop(bool on) {
    if (on && !render_loop_on) {
        Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(App::onIdle));
        render_loop_on = true;
    } else if (!on && render_loop_on) {
        Disconnect(wxEVT_IDLE, wxIdleEventHandler(App::onIdle));
        render_loop_on = false;
    }
}

void App::onIdle(wxIdleEvent &evt) {
    if (render_loop_on) {
        drawPane->paintNow();
        evt.RequestMore();  // render continuously, not only
                            // once on idle
    }
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Thapsteak Notecharter", wxPoint(50, 50),
              wxSize(400, 200)) {
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::OnExit(wxCommandEvent &event) { Close(true); }

void MyFrame::OnAbout(wxCommandEvent &event) {
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello World",
                 wxOK | wxICON_INFORMATION);
}

void MyFrame::OnClose(wxCloseEvent &evt) {
    wxGetApp().activateRenderLoop(false);
    evt.Skip();
}
