#include "../include/App.h"
#include "../include/MainWindow.h"


IMPLEMENT_APP(App);//Main

bool App::OnInit() {


    //    wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);
    //    wxUpdateUIEvent::SetMode(wxUPDATE_UI_PROCESS_SPECIFIED);
    //    wxUpdateUIEvent::SetUpdateInterval(1500);

    if (!wxApp::OnInit()) {
        return false;
    }
    else {

        MainWindow* frame = new MainWindow();
        frame->Show(true);
        return true;
    }
}

App::App()
{
    //ctor
}

App::~App()
{
    //ctor
}
