#pragma once
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
