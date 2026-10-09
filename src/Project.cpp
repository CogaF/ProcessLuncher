/*!
 * \file Project.cpp
 * \brief Implementation of Project.h.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */

#include "Project.h"

#include <wx/filefn.h>
#include <wx/fileconf.h>
#include <wx/log.h>
#include <wx/wfstream.h>

namespace {

/*! \brief Format version written in [Project]; a newer file is refused. */
constexpr long kFormatVersion = 1;

/*! \brief Name of the group of row \p index (zero based): Row01, Row02, ... */
wxString rowGroup(size_t index)
{
    return wxString::Format("/Row%02u", static_cast<unsigned>(index + 1));
}

} // namespace

namespace Project {

bool load(const wxString& path, Data& data, wxString& error)
{
    if (!wxFileExists(path)) {
        error = wxString::Format("The project file %s does not exist.", path);
        return false;
    }
    wxFileInputStream input(path);
    if (!input.IsOk()) {
        error = wxString::Format("Cannot read %s.", path);
        return false;
    }
    wxLogNull noLog; // a malformed file is reported through error, not with a log window
    wxFileConfig config(input, wxConvUTF8);
    config.SetExpandEnvVars(false); // %VARIABLES% belong to the commands, not to the file

    long version = 0;
    if (!config.Read("/Project/Version", &version) || version < 1) {
        error = wxString::Format("%s is not a project file of Process Launcher.", path);
        return false;
    }
    if (version > kFormatVersion) {
        error = wxString::Format("%s was written by a newer version of Process Launcher.", path);
        return false;
    }

    Data result;
    result.resultFile = config.Read("/Project/ResultFile", wxString());
    result.repeat = static_cast<int>(config.ReadLong("/Project/Repeat", 1));
    if (result.repeat < 0) result.repeat = 1;
    for (size_t i = 0; config.HasGroup(rowGroup(i)); i++) {
        const wxString group = rowGroup(i) + "/";
        Row row;
        row.active = config.ReadBool(group + "Active", true);
        row.single = config.ReadBool(group + "Single", false);
        row.command = config.Read(group + "Command", wxString());
        row.expected = config.Read(group + "Expected", wxString());
        row.timeout = static_cast<int>(config.ReadLong(group + "Timeout", 0));
        if (row.timeout < 0) row.timeout = 0;
        result.rows.push_back(row);
    }
    data = result;
    return true;
}

bool save(const wxString& path, const Data& data, wxString& error)
{
    wxFileConfig config(wxEmptyString, wxEmptyString, wxEmptyString, wxEmptyString, 0);
    config.SetExpandEnvVars(false);
    config.Write("/Project/Version", kFormatVersion);
    config.Write("/Project/ResultFile", data.resultFile);
    config.Write("/Project/Repeat", static_cast<long>(data.repeat));
    for (size_t i = 0; i < data.rows.size(); i++) {
        const wxString group = rowGroup(i) + "/";
        const Row& row = data.rows[i];
        config.Write(group + "Active", row.active);
        config.Write(group + "Single", row.single);
        config.Write(group + "Command", row.command);
        config.Write(group + "Expected", row.expected);
        config.Write(group + "Timeout", static_cast<long>(row.timeout));
    }

    // Written to a temporary file first, so a failed write never leaves half a project.
    const wxString temporary = path + ".tmp";
    {
        wxFileOutputStream output(temporary);
        if (!output.IsOk() || !config.Save(output, wxConvUTF8) || !output.Close()) {
            wxRemoveFile(temporary);
            error = wxString::Format("Cannot write %s.", path);
            return false;
        }
    }
    if (!wxRenameFile(temporary, path, true)) {
        wxRemoveFile(temporary);
        error = wxString::Format("Cannot replace %s.", path);
        return false;
    }
    return true;
}

} // namespace Project
