#include <wx/wx.h>

#include <Canvas.hpp>

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