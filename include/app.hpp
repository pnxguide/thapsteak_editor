#include <wx/wx.h>

#include "canvas.hpp"

class MyFrame;

class App : public wxApp {
    bool render_loop_on;
    void onIdle(wxIdleEvent &evt);
    virtual bool OnInit();

    MyFrame *frame;
    Canvas *drawPane;

   public:
    void activateRenderLoop(bool on);
};

class MyFrame : public wxFrame {
    RenderTimer *timer;
    Canvas *drawPane;

   public:
    MyFrame();
    ~MyFrame();
    void OnClose(wxCloseEvent &evt);

    DECLARE_EVENT_TABLE()

   private:
    void OnExit(wxCommandEvent &event);
};
