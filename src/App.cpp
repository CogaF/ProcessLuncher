/*!
 * \file App.cpp
 * \brief Implementation of App.h: start-up, dark mode and the application icon.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */

#include "App.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/datectrl.h>
#include <wx/icon.h>
#include <wx/toplevel.h>

#include "AppInfo.h"
#include "BuildInfo.h"
#include "DataDir.h"
#include "LicenseManager.h"
#include "Log.h"
#include "MainWindow.h"

wxIMPLEMENT_APP(App);

namespace {

/*! \brief Background used for dark windows. */
const wxColour kDarkBackground(32, 32, 32);
/*! \brief Text colour used for dark windows. */
const wxColour kDarkForeground(230, 230, 230);

/*!
 * \brief Recursively applies the dark colours to \p win and its children.
 *
 * Buttons, check boxes, choices and date pickers are skipped on purpose: the system draws them dark
 * by itself (wxApp::SetAppearance) and forced colours would hide their disabled look.
 * \param win window to colour (may be null).
 */
void ApplyDarkColours(wxWindow* win)
{
    if (win == nullptr) return;
    const bool drawnBySystem = wxDynamicCast(win, wxButton) != nullptr ||
                               wxDynamicCast(win, wxCheckBox) != nullptr ||
                               wxDynamicCast(win, wxChoice) != nullptr ||
                               wxDynamicCast(win, wxDatePickerCtrl) != nullptr;
    if (!drawnBySystem) {
        win->SetBackgroundColour(kDarkBackground);
        win->SetForegroundColour(kDarkForeground);
    }
    for (wxWindow* child : win->GetChildren()) ApplyDarkColours(child);
    win->Refresh();
}

} // namespace

bool App::OnInit()
{
    if (!wxApp::OnInit()) return false;

#if wxCHECK_VERSION(3, 3, 0)
    // Must happen before the first window is created; wxWidgets applies it completely only then.
    SetAppearance(wxApp::Appearance::Dark);
#endif

    SetAppDisplayName(AppInfo::kName);

    // Log first, so everything after it is recorded (<data folder>/log.txt).
    Log::setLevel(LogLevel::Info);
    Log::enableFile(DataDir::file("log.txt"));
    Log::info(std::string(AppInfo::kName) + " " + GetCompactVersion() + " starting, data folder: " + DataDir::path().string());

    // License and trial state, before the first window (which shows them in its status bar).
    Licensing::initialize();

    auto* frame = new MainWindow();
    frame->Show(true);
    return true;
}

int App::OnExit()
{
    Log::info("Exiting.");
    Log::enableFile({});
    return wxApp::OnExit();
}

int App::FilterEvent(wxEvent& event)
{
    if (event.GetEventType() == wxEVT_SHOW && static_cast<wxShowEvent&>(event).IsShown()) {
        if (auto* tlw = wxDynamicCast(event.GetEventObject(), wxTopLevelWindow)) {
            // "appicon" is the first icon of the .rc file (art/app.ico).
            tlw->SetIcons(wxIconBundle("appicon", nullptr));
            ApplyDarkColours(tlw);
        }
    }
    return wxApp::FilterEvent(event);
}
