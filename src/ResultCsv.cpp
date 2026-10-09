/*!
 * \file ResultCsv.cpp
 * \brief Implementation of ResultCsv.h.
 *
 * Application by Coga Fation (developed with the help of ChatGPT and Claude).
 * Released under the MIT licence, see LICENSE.
 */

#include "ResultCsv.h"

#include <wx/file.h>

namespace ResultCsv {

wxString quote(const wxString& field, wxChar separator)
{
    if (field.find_first_of(wxString(separator) + "\"\r\n") == wxString::npos) return field;
    wxString quoted = field;
    quoted.Replace("\"", "\"\"");
    return "\"" + quoted + "\"";
}

wxString format(const std::vector<Record>& records, wxChar separator)
{
    const wxString sep(separator);
    wxString text = "Timestamp" + sep + "Run" + sep + "Row" + sep + "Command" + sep + "Expected" + sep + "Result" + sep +
                    "Exit code" + sep + "Duration (s)" + sep + "Note" + sep + "Output\r\n";
    for (const Record& r : records) {
        wxString output = r.output;
        output.Trim(); // the trailing line break of a console program
        // The decimal point follows the separator: "1,5" would split a comma separated line.
        wxString seconds = wxString::Format("%ld.%01ld", r.durationMs / 1000, (r.durationMs % 1000) / 100);
        if (separator != ',') seconds.Replace(".", ",");
        text += quote(r.timestamp, separator) + sep + wxString::Format("%d", r.run) + sep + wxString::Format("%d", r.row) + sep +
                quote(r.command, separator) + sep + quote(r.expected, separator) + sep + (r.pass ? "PASS" : "FAIL") + sep +
                wxString::Format("%ld", r.exitCode) + sep + seconds + sep + quote(r.note, separator) + sep +
                quote(output, separator) + "\r\n";
    }
    return text;
}

bool write(const wxString& path, const std::vector<Record>& records, wxChar separator, wxString& error)
{
    const wxScopedCharBuffer utf8 = format(records, separator).utf8_str();
    wxFile file(path, wxFile::write);
    const char bom[] = "\xEF\xBB\xBF";
    if (!file.IsOpened() || file.Write(bom, 3) != 3 || file.Write(utf8.data(), utf8.length()) != utf8.length()) {
        error = wxString::Format("Cannot write %s.", path);
        return false;
    }
    return true;
}

} // namespace ResultCsv
