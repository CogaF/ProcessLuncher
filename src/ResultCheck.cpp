/*!
 * \file ResultCheck.cpp
 * \brief Implementation of ResultCheck.h.
 */

#include "ResultCheck.h"

#include <wx/arrstr.h>
#include <wx/filename.h>

#include <algorithm>
#include <wx/utils.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

namespace ResultCheck {

wxString expandId(const wxString& expected, int commandId)
{
    wxString text = expected;
    text.Replace(kIdPlaceholder, wxString::Format("%d", commandId));
    return text;
}

/*! \brief Reads "0", "0-7", "0,3,10-12" into ranges. \return false if a part is not a number or a range. */
static bool parseExitCodes(const wxString& list, std::vector<std::pair<long, long>>& ranges)
{
    for (wxString part : wxSplit(list, ',')) {
        part.Trim().Trim(false);
        // a '-' after the first character separates a range (the first may be the sign of a number)
        const size_t dash = part.find('-', 1);
        long from = 0, to = 0;
        if (dash == wxString::npos) {
            if (!part.ToLong(&from)) return false;
            to = from;
        }
        else {
            wxString a = part.Left(dash), b = part.Mid(dash + 1);
            a.Trim().Trim(false);
            b.Trim().Trim(false);
            if (!a.ToLong(&from) || !b.ToLong(&to) || to < from) return false;
        }
        ranges.emplace_back(from, to);
    }
    return !ranges.empty();
}

Spec parse(const wxString& expected)
{
    Spec spec;
    spec.text = expected;
    wxString trimmed = expected;
    trimmed.Trim(false);
    if (trimmed.StartsWith(kExitTag)) {
        spec.isExit = true;
        spec.valid = parseExitCodes(trimmed.Mid(kExitTag.length()), spec.exitCodes);
        return spec;
    }
    const int tagPos = expected.Find(kFileTag);
    if (tagPos == wxNOT_FOUND) return spec; // plain text

    spec.isFile = true;
    const wxString rest = expected.Mid(static_cast<size_t>(tagPos) + kFileTag.length());
    // The first "::" ends the path: a path cannot contain it (the ':' of a drive letter stands alone).
    const int sepPos = rest.Find(kSeparator);
    if (sepPos == wxNOT_FOUND) {
        spec.valid = false;
        return spec;
    }
    wxString path = rest.Left(static_cast<size_t>(sepPos));
    path.Trim().Trim(false);
    spec.text = rest.Mid(static_cast<size_t>(sepPos) + kSeparator.length());
    spec.usesResultFile = path.empty();
    if (!path.empty()) {
        wxFileName name(path);
        name.Normalize(wxPATH_NORM_ENV_VARS); // expands %VARIABLES%
        path = name.GetFullPath();
    }
    spec.path = path;
    return spec;
}

wxFileOffset fileSize(const wxString& path)
{
    if (!wxFileExists(path)) return 0;
    const wxULongLong size = wxFileName::GetSize(path);
    return size == wxInvalidSize ? 0 : static_cast<wxFileOffset>(size.GetValue());
}

bool findInFile(const wxString& path, const wxString& text, wxFileOffset startOffset, bool& opened)
{
    wxFileInputStream input(path);
    opened = input.IsOk();
    if (!opened) return false;

    // A result file that was replaced by a shorter one is searched from its start.
    if (startOffset > 0 && startOffset <= static_cast<wxFileOffset>(input.GetLength())) input.SeekI(startOffset);

    wxTextInputStream lines(input);
    while (input.IsOk() && !input.Eof()) {
        if (lines.ReadLine().Contains(text)) return true;
    }
    return false;
}

bool evaluate(const wxString& expectedField, const wxString& output, long exitCode, int commandId,
              const wxString& resultFile, wxFileOffset resultFileOffset, wxString& note, wxString& missingResultFile)
{
    const wxString expected = expandId(expectedField, commandId);
    const Spec spec = parse(expected);
    if (spec.isExit) {
        if (!spec.valid) {
            note = wxString::Format("Expected result \"%s\" is not of the form %s<codes>, e.g. %s0 or %s0-7,16",
                                    expected, kExitTag, kExitTag, kExitTag);
            return false;
        }
        return std::any_of(spec.exitCodes.begin(), spec.exitCodes.end(),
                           [exitCode](const std::pair<long, long>& r) { return exitCode >= r.first && exitCode <= r.second; });
    }
    if (!spec.isFile) return output.Contains(expected);

    if (!spec.valid) {
        note = wxString::Format("Expected result \"%s\" is not of the form %s<file>%s<text>", expected, kFileTag, kSeparator);
        return false;
    }

    const wxString path = spec.usesResultFile ? resultFile : spec.path;
    if (!wxFileExists(path)) {
        note = wxString::Format("File %s doesn't exist", path);
        if (spec.usesResultFile) missingResultFile = path;
        return false;
    }

    bool opened = false;
    const bool found = findInFile(path, spec.text, spec.usesResultFile ? resultFileOffset : 0, opened);
    if (!opened) note = wxString::Format("Failed to open file \"%s\"", path);
    return found;
}

} // namespace ResultCheck
