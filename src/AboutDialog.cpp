/*!
 * \file AboutDialog.cpp
 * \brief Implementation of AboutDialog.h.
 */

#include "AboutDialog.h"

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/clipbrd.h>
#include <wx/hyperlink.h>
#include <wx/icon.h>
#include <wx/log.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>
#include <wx/version.h>

#include "AppInfo.h"
#include "AppSettings.h"
#include "BuildInfo.h"
#include "DataDir.h"
#include "LicenseDialog.h"
#include "LicenseManager.h"
#include "LicensePolicy.h"
#include "Version.h"

namespace {

/*! \brief UTF-8 text of the code as a wxString. */
wxString utf8(const char* text) { return wxString::FromUTF8(text); }

/*! \brief A read-only multi-line text box. */
wxTextCtrl* readOnlyText(wxWindow* parent, const wxString& text, const wxFont& font = wxNullFont)
{
    auto* box = new wxTextCtrl(parent, wxID_ANY, text, wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxBORDER_NONE);
    if (font.IsOk()) box->SetFont(font);
    return box;
}

} // namespace

AboutDialog::AboutDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, wxString::Format("About %s", utf8(AppInfo::kName)), wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto* top = new wxBoxSizer(wxVERTICAL);
    auto* book = new wxNotebook(this, wxID_ANY);
    book->AddPage(BuildAboutPage(book), "About");
    book->AddPage(BuildLicensePage(book), "License");
    book->AddPage(BuildChangesPage(book), "Changes");
    book->AddPage(BuildSystemPage(book), "System");
    top->Add(book, 1, wxEXPAND | wxALL, FromDIP(8));

    auto* buttons = new wxBoxSizer(wxHORIZONTAL);
    buttons->AddStretchSpacer();
    auto* close = new wxButton(this, wxID_OK, "Close");
    close->SetDefault();
    buttons->Add(close, 0);
    top->Add(buttons, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));

    SetSizerAndFit(top);
    SetMinSize(FromDIP(wxSize(640, 480)));
    SetSize(FromDIP(wxSize(700, 520)));
    SetEscapeId(wxID_OK);
    CentreOnParent();
}

wxWindow* AboutDialog::BuildAboutPage(wxNotebook* book)
{
    auto* page = new wxPanel(book);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    const int gap = FromDIP(8);

    // Header: icon, name, version.
    auto* header = new wxBoxSizer(wxHORIZONTAL);
    wxIcon icon;
    {
        wxLogNull quiet; // the icon is a Windows resource: no message where it does not exist
        icon = wxIcon("appicon", wxBITMAP_TYPE_ICO_RESOURCE, FromDIP(64), FromDIP(64));
    }
    if (icon.IsOk()) {
        header->Add(new wxStaticBitmap(page, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icon))), 0,
                    wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(14));
    }
    auto* titles = new wxBoxSizer(wxVERTICAL);
    auto* name = new wxStaticText(page, wxID_ANY, utf8(AppInfo::kName));
    name->SetFont(name->GetFont().Bold().Scaled(1.8f));
    titles->Add(name, 0);
    titles->Add(new wxStaticText(page, wxID_ANY, "Parallel Command Runner (PCR)"), 0, wxTOP, FromDIP(2));
    titles->Add(new wxStaticText(page, wxID_ANY, wxString::Format("Version %s   -   built %s", utf8(GetAppVersion().c_str()),
                                                                   utf8(GetBuildDate().c_str()))), 0, wxTOP, FromDIP(2));
    header->Add(titles, 1, wxALIGN_CENTER_VERTICAL);
    sizer->Add(header, 0, wxEXPAND | wxALL, gap);

    auto* description = new wxStaticText(page, wxID_ANY, utf8(AppInfo::kDescription));
    description->Wrap(FromDIP(600));
    sizer->Add(description, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, gap);

    // Who / where.
    auto* grid = new wxFlexGridSizer(2, FromDIP(4), FromDIP(12));
    grid->AddGrowableCol(1);
    auto row = [&](const wxString& label, wxWindow* value) {
        auto* l = new wxStaticText(page, wxID_ANY, label);
        l->SetFont(l->GetFont().Bold());
        grid->Add(l, 0, wxALIGN_TOP);
        grid->Add(value, 1, wxALIGN_LEFT);
    };
    row("Author:", new wxStaticText(page, wxID_ANY, "Coga Fation"));
    row("Organisation:", new wxStaticText(page, wxID_ANY, utf8(AppInfo::kOrganisation)));
    row("Contact:", new wxHyperlinkCtrl(page, wxID_ANY, utf8(AppInfo::kContact), wxString("mailto:") + utf8(AppInfo::kContact)));
    row("Project page:", new wxHyperlinkCtrl(page, wxID_ANY, utf8(AppInfo::kWebsite), utf8(AppInfo::kWebsite)));
    row("Copyright:", new wxStaticText(page, wxID_ANY, utf8(AppInfo::kCopyright)));
    row("License:", new wxStaticText(page, wxID_ANY, utf8(AppInfo::kLicense)));
    sizer->Add(grid, 0, wxEXPAND | wxALL, gap);

    // Acknowledgements.
    auto* credits = readOnlyText(page,
        "Built with wxWidgets (wxWindows Library Licence 3.1) - https://www.wxwidgets.org\n"
        "Licenses use Ed25519 signatures and SHA-2 (the code of the license system is part of this program).\n"
        "Developed with the help of ChatGPT and Claude.\n\n"
        "Every command runs in its own thread, so the window never freezes. The result of a command is "
        "PASS when its output, or the result file, contains the expected text.");
    sizer->Add(credits, 1, wxEXPAND | wxALL, gap);

    page->SetSizer(sizer);
    return page;
}

wxWindow* AboutDialog::BuildLicensePage(wxNotebook* book)
{
    auto* page = new wxPanel(book);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    const int gap = FromDIP(8);

    Licensing::refresh();
    wxString state;
    switch (Licensing::mode()) {
    case Licensing::Mode::Licensed: state = "Licensed"; break;
    case Licensing::Mode::Trial: state = wxString::Format("Trial version - %d day(s) left", Licensing::trialDaysLeft()); break;
    default: state = "Not licensed"; break;
    }
    auto* title = new wxStaticText(page, wxID_ANY, state);
    title->SetFont(title->GetFont().Bold().Scaled(1.4f));
    sizer->Add(title, 0, wxALL, gap);

    wxString text = LicenseDialog::statusBarText() + "\n";
    const License::Evaluation& e = Licensing::evaluation();
    if (Licensing::mode() != Licensing::Mode::Licensed) {
        const wxString problem = LicenseDialog::problemText(e);
        if (!problem.empty()) text += "\n" + problem + "\n";
    }
    else {
        text += "\nLicensed to: " + utf8(e.info.licensee.c_str());
        if (!e.info.edition.empty()) text += "\nEdition: " + utf8(e.info.edition.c_str());
        text += "\nLicense ID: " + utf8(e.info.id.c_str()) + "\n";
    }
    text += "\nFeatures:\n";
    for (const char* feature : LicensePolicy::kAllFeatures) {
        text += wxString::Format("  %s  %s\n", Licensing::allows(feature) ? "[x]" : "[ ]", LicenseDialog::featureName(feature));
    }
    sizer->Add(readOnlyText(page, text), 1, wxEXPAND | wxLEFT | wxRIGHT, gap);

    auto* open = new wxButton(page, wxID_ANY, "License window...");
    open->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        LicenseDialog dialog(this);
        dialog.ShowModal();
        if (dialog.changed()) m_licenseChanged = true;
    });
    sizer->Add(open, 0, wxALL, gap);

    page->SetSizer(sizer);
    return page;
}

wxWindow* AboutDialog::BuildChangesPage(wxNotebook* book)
{
    auto* page = new wxPanel(book);
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    wxString text;
    for (const AppVersionHistoryEntry& entry : kAppVersionHistory) {
        text += wxString::Format("%s  (%s)\n", utf8(entry.version), utf8(entry.date));
        wxString notes = utf8(entry.notes);
        notes.Replace("\n", "\n  - ");
        text += "  - " + notes + "\n\n";
    }
    sizer->Add(readOnlyText(page, text), 1, wxEXPAND | wxALL, FromDIP(8));
    page->SetSizer(sizer);
    return page;
}

wxString AboutDialog::systemInfoText()
{
    wxString text;
    text << "Application: " << utf8(AppInfo::kName) << "\n";
    text << "Version: " << utf8(GetAppVersion().c_str()) << "\n";
    text << "Built: " << utf8(GetBuildDate().c_str()) << "\n";
    text << "wxWidgets: " << wxVERSION_STRING << "\n";
    text << "Operating system: " << wxGetOsDescription() << "\n";
#if defined(_WIN64) || defined(__x86_64__)
    text << "Platform: 64 bit\n";
#else
    text << "Platform: 32 bit\n";
#endif
    text << "Executable folder: " << wxString(DataDir::exeDirectory().wstring()) << "\n";
    text << "Data folder: " << wxString(DataDir::path().wstring()) << "\n";
    text << "Result file: " << AppSettings::getString("resultFile", wxString(DataDir::exeDirectory().wstring()) + wxFILE_SEP_PATH + AppInfo::kDefaultResultFileName) << "\n";
    text << "Product code: " << LicensePolicy::kProduct << "\n";
    const std::string& uid = Licensing::machineUid();
    text << "This PC's UID: " << (uid.empty() ? wxString("(unavailable)") : utf8(License::displayUid(uid).c_str())) << "\n";
    return text;
}

wxWindow* AboutDialog::BuildSystemPage(wxNotebook* book)
{
    auto* page = new wxPanel(book);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    const wxString info = systemInfoText();
    sizer->Add(readOnlyText(page, info, wxFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE))), 1, wxEXPAND | wxALL, FromDIP(8));

    auto* copy = new wxButton(page, wxID_ANY, "Copy to clipboard");
    copy->Bind(wxEVT_BUTTON, [info](wxCommandEvent&) {
        if (wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxTextDataObject(info));
            wxTheClipboard->Close();
        }
    });
    sizer->Add(copy, 0, wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));
    page->SetSizer(sizer);
    return page;
}
