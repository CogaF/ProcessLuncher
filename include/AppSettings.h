/*!
 * \file AppSettings.h
 * \brief Small persistent settings (key = value) kept in "<data folder>/settings.ini".
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 */
#pragma once

#include <wx/string.h>

/*! \brief Persistent settings; every call reads / writes the file at once, so a crash loses nothing. */
namespace AppSettings
{
    /*!
     * \brief Reads a setting.
     * \param key name of the setting.
     * \param defaultValue returned when the setting does not exist.
     */
    wxString getString(const wxString& key, const wxString& defaultValue = wxString());
    /*! \brief Reads an integer setting. \param key name \param defaultValue value when missing. */
    int getInt(const wxString& key, int defaultValue);
    /*! \brief Writes a setting. \param key name \param value new value. */
    void set(const wxString& key, const wxString& value);
    /*! \brief Writes an integer setting. \param key name \param value new value. */
    void setInt(const wxString& key, int value);
}
