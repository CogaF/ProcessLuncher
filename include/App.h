/*!
 * \file App.h
 * \brief The wxWidgets application object of Process Launcher.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */
#pragma once

#include <wx/app.h>

/*!
 * \brief Application object.
 *
 * Process Launcher is a small tool without a theme setting, so it always starts in dark mode:
 * OnInit() calls wxApp::SetAppearance() (wxWidgets 3.3+) before the first window exists, and
 * FilterEvent() colours every top-level window the first time it is shown and gives it the
 * application icon.
 */
class App : public wxApp
{
public:
    /*!
     * \brief Selects the dark appearance, then creates and shows the main window.
     * \return false to abort the start-up.
     */
    bool OnInit() override;

    /*!
     * \brief Sees every event before the window it is meant for.
     *
     * Used to theme each top-level window (frame, dialog, message box) and set its icon when it is
     * shown for the first time.
     * \param event the event being dispatched.
     * \return Event_Skip (-1) so that normal processing continues.
     */
    int FilterEvent(wxEvent& event) override;
};

wxDECLARE_APP(App);
