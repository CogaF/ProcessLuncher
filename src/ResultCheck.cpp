/*!
 * \file ResultCheck.cpp
 * \brief Implementation of ResultCheck.h.
 */

#include "ResultCheck.h"

#include <wx/filename.h>
#include <wx/utils.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

namespace ResultCheck {

Spec parse(const wxString& expected)
{
    Spec spec;
    spec.text = expected;
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

bool evaluate(const wxString& expected, const wxString& output, const wxString& resultFile,
              wxFileOffset resultFileOffset, wxString& note, wxString& missingResultFile)
{
    const Spec spec = parse(expected);
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
