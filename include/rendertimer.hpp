#include <wx/wx.h>

class Canvas;

class RenderTimer : public wxTimer {
    Canvas *pane;

   public:
    RenderTimer(Canvas *pane);
    void Notify();
    void start();
};