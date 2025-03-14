/*
	This is an application that was created by me Coga Fation but truth be told
	that without the help of ChatGpt it could have taken many many hours
	I'm taking for granted that I would have reached the same results which is a long shot
*/


#pragma once
#ifdef WX_PRECOMP
#include <wx/wxprec.h>
#else
#include <wx/wx.h>
#endif // WX_PRECOMP


#include "id.h"
class App : public wxApp
{
public:
    App();
    bool OnInit();
    ~App();

protected:

private:
};

DECLARE_APP(App); //wxGetApp
