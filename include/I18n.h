/*!
 * \file I18n.h
 * \brief tr(): hook for translated texts. Process Launcher is English only, so it returns the text as is.
 *
 * The shared license code (LicenseDialog) shows its texts through tr(); a translation layer can be
 * added later by changing this one function (see PdfEncryptor's I18n for a complete one).
 */
#pragma once

#include <wx/string.h>

/*! \brief Returns \p englishText unchanged (no translation installed). */
inline wxString tr(const wxString& englishText) { return englishText; }
