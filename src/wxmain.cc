#include <wx/wx.h>
#include "help.h"

extern "C"
{
extern bool apme_init(int argc, char* argv[]);
}

class ApmePoll : public wxTimer
{
    void Notify(void)
    {
        FILE *f;

        f = fopen("./bla.txt", "a+");
        fprintf(f, "periodic\n");
        fclose(f);
    }
};

class ApmeApp : public wxApp
{
    private:
        wxTimer *periodic;

    public:
        ApmeApp() : wxApp(), periodic(NULL)
        {
        }

        virtual bool OnInit(); 
};

DECLARE_APP(ApmeApp)

IMPLEMENT_APP(ApmeApp)

bool ApmeApp::OnInit()
{
    wxFrame *frame = new wxFrame((wxFrame*) NULL, -1, _T("Hello wxzWidgets World"));
    frame->CreateStatusBar();
    frame->SetStatusText(_T("Hello World"));
    frame->Show(true);
    SetTopWindow(frame);

    /* Pop up stuff */
    wxMessageBox(help_chatlog_warning, "Enable the chatlog?", wxYES_NO, frame);

    /* Periodic stuff */
    periodic = new ApmePoll();
    periodic->Start(1000, wxTIMER_CONTINUOUS);

    return true;
}
