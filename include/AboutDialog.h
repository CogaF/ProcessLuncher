/*!
 * \file AboutDialog.h
 * \brief Info > About: a structured window with the application, its license, the version history
 * and the technical details of this installation.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 */
#pragma once

#include <wx/dialog.h>

class wxNotebook;
class wxWindow;

/*!
 * \brief The About window, a notebook with four pages:
 *  - <b>About</b>: name, version, description, author, contact, web site, acknowledgements;
 *  - <b>License</b>: the state of the license (the button opens the License window);
 *  - <b>Changes</b>: the version history;
 *  - <b>System</b>: versions, paths and this PC's UID, with a button that copies them for a support request.
 */
class AboutDialog : public wxDialog
{
public:
    /*! \brief Builds the window. \param parent the main window. */
    explicit AboutDialog(wxWindow* parent);

    /*! \brief true if a license was installed or removed from the License window opened here. */
    bool licenseChanged() const { return m_licenseChanged; }

    /*! \brief The technical details of this installation, one "name: value" per line. */
    static wxString systemInfoText();

private:
    /*! \brief Builds the About page. */
    wxWindow* BuildAboutPage(wxNotebook* book);
    /*! \brief Builds the License page. */
    wxWindow* BuildLicensePage(wxNotebook* book);
    /*! \brief Builds the version history page. */
    wxWindow* BuildChangesPage(wxNotebook* book);
    /*! \brief Builds the System page. */
    wxWindow* BuildSystemPage(wxNotebook* book);

    bool m_licenseChanged = false; /*!< see licenseChanged(). */
};
