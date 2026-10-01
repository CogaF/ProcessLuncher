/*!
 * \file AppSettings.cpp
 * \brief Implementation of AppSettings.h.
 */

#include "AppSettings.h"

#include <memory>

#include <wx/fileconf.h>

#include "DataDir.h"

namespace {

/*! \brief The settings file, opened at every call (the file is tiny and this keeps it always current). */
std::unique_ptr<wxFileConfig> openConfig()
{
    const wxString file = wxString(DataDir::file("settings.ini").wstring());
    return std::make_unique<wxFileConfig>(wxEmptyString, wxEmptyString, file, wxEmptyString, wxCONFIG_USE_LOCAL_FILE);
}

} // namespace

namespace AppSettings {

wxString getString(const wxString& key, const wxString& defaultValue)
{
    return openConfig()->Read(key, defaultValue);
}

int getInt(const wxString& key, int defaultValue)
{
    long value = defaultValue;
    openConfig()->Read(key, &value, defaultValue);
    return static_cast<int>(value);
}

void set(const wxString& key, const wxString& value)
{
    auto config = openConfig();
    config->Write(key, value);
    config->Flush();
}

void setInt(const wxString& key, int value)
{
    auto config = openConfig();
    config->Write(key, static_cast<long>(value));
    config->Flush();
}

} // namespace AppSettings
