/*
    This is an application that was created by me Coga Fation but truth be told
    that without the help of ChatGpt it could have taken many many hours
    I'm taking for granted that I would have reached the same results which is a long shot
*/

#include "App.h"
#include "MainWindow.h"


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
