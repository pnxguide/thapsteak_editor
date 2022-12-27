#include <App.hpp>

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_CLOSE(MyFrame::OnClose)
END_EVENT_TABLE()

bool App::OnInit() {
    frame = new MyFrame();
    frame->Show();
    return true;
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

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    drawPane = new Canvas(this);
    sizer->Add(drawPane, 1, wxEXPAND);
    SetSizer(sizer);

    timer = new RenderTimer(drawPane);
    Show();
    timer->start();
}

MyFrame::~MyFrame() {
    delete timer;
}

void MyFrame::OnExit(wxCommandEvent &event) { Close(true); }

void MyFrame::OnAbout(wxCommandEvent &event) {
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello World",
                 wxOK | wxICON_INFORMATION);
}

void MyFrame::OnClose(wxCloseEvent &evt) {
    timer->Stop();
    evt.Skip();
}
