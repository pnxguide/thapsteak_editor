#include "../include/app.hpp"

#include <wx/wx.h>
#include <wx/window.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>

/**
 * TODO: (Urgent) Flickering screens
 * TODO: Command stacks (for undo-ing)
 * TODO: Add music (with multiple BPM)
 * TODO: Drag existing notes
 */

wxIMPLEMENT_APP(App);

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
              wxSize(640, 480)) {
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("v.0.1 (pre-alpha) | Created by pnx (for Thapsteak)");

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

void MyFrame::OnClose(wxCloseEvent &evt) {
    timer->Stop();
    evt.Skip();
}
