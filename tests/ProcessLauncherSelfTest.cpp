/*!
 * \file ProcessLauncherSelfTest.cpp
 * \brief Console self-test of the parts without GUI: ResultCheck and BatHighlighter.
 *
 * Build (Visual Studio developer prompt, wxWidgets base only):
 *   cl /EHsc /std:c++20 /utf-8 /DwxUSE_GUI=0 /I include /I %WXWIN%\include /I %WXWIN%\include\msvc
 *      tests\ProcessLauncherSelfTest.cpp src\ResultCheck.cpp src\BatHighlighter.cpp src\BatCommands.cpp
 * On Linux with libwxbase3.2-dev: g++ -std=c++20 -I include $(wx-config --cxxflags base) ... $(wx-config --libs base)
 * Returns 0 if everything passes.
 */

#include <cstdio>

#include <wx/app.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/init.h>

#include "BatCommands.h"
#include "BatHighlighter.h"
#include "ResultCheck.h"

namespace {

int g_failures = 0; /*!< number of failed checks. */

/*! \brief Reports one check. */
void check(bool ok, const char* what)
{
    if (!ok) { g_failures++; std::printf("FAIL: %s\n", what); }
}

/*! \brief true if a span of \p kind covers exactly \p word in \p text. */
bool has(const wxString& text, const std::vector<BatSpan>& spans, BatToken kind, const wxString& word)
{
    for (const BatSpan& s : spans)
        if (s.kind == kind && text.Mid(s.start, s.length) == word) return true;
    return false;
}

/*! \brief Deletes a file if it exists. */
void removeIfExists(const wxString& path)
{
    if (wxFileExists(path)) wxRemoveFile(path);
}

/*! \brief Writes a text file. */
void writeFile(const wxString& path, const wxString& content)
{
    wxFFile f(path, "wb");
    f.Write(content.utf8_str().data(), content.utf8_str().length());
}

} // namespace

int main(int argc, char** argv)
{
    wxInitializer init(argc, argv);

    // --- ResultCheck::parse
    auto plain = ResultCheck::parse("TTL=");
    check(!plain.isFile && plain.text == "TTL=", "plain text");
    auto file = ResultCheck::parse(":File:C:\\temp\\result.txt::String: PASS");
    check(file.isFile && file.valid && file.path == "C:\\temp\\result.txt" && file.text == "String: PASS", "explicit file with colon in text");
    auto dflt = ResultCheck::parse(":File:::PASS");
    check(dflt.isFile && dflt.usesResultFile && dflt.text == "PASS", "default result file");
    check(!ResultCheck::parse(":File:abc").valid, "file tag without separator");

    // --- ResultCheck::evaluate with a real file
    const wxString path = wxFileName::GetTempDir() + wxFileName::GetPathSeparator() + "pcr_selftest_result.txt";
    removeIfExists(path);
    wxString note, missing;
    check(!ResultCheck::evaluate(":File:::PASS", "", path, 0, note, missing) && missing == path, "missing result file reported");
    writeFile(path, "2026-01-01 00:00:00 old PASS\n");
    const wxFileOffset offset = ResultCheck::fileSize(path);
    note.clear(); missing.clear();
    check(ResultCheck::evaluate(":File:::PASS", "", path, 0, note, missing), "old PASS found when searching the whole file");
    check(!ResultCheck::evaluate(":File:::PASS", "", path, offset, note, missing), "old PASS ignored after the start offset");
    { wxFFile f(path, "ab"); f.Write("2026-01-01 00:00:01 new PASS\n"); }
    check(ResultCheck::evaluate(":File:::PASS", "", path, offset, note, missing), "new PASS found after the start offset");
    check(ResultCheck::evaluate(":File:" + path + "::old", "", "unused", offset, note, missing), "explicit path searches the whole file");
    check(ResultCheck::evaluate("hello", "say hello there", path, 0, note, missing), "plain text found in output");
    check(!ResultCheck::evaluate("bye", "say hello there", path, 0, note, missing), "plain text not found in output");
    removeIfExists(path);

    // --- BatHighlighter
    const wxString bat =
        "@echo off\r\n"
        "rem a comment\r\n"
        ":start\r\n"
        "if errorlevel 1 (\r\n"
        "    echo FAIL %errorlevel%\r\n"
        ") else (\r\n"
        "    ping -n 1 \"%~dp0host\" | find \"TTL=\" >nul\r\n"
        ")\r\n"
        "if exist \"a.txt\" goto :start\r\n"
        "for %%f in (*.txt) do type %%f\r\n"
        "echo PASS>>\"%RESULT_FILE%\"\r\n";
    wxString clean = bat; clean.Replace("\r", "");
    const auto spans = HighlightBatch(bat);
    check(has(clean, spans, BatToken::Command, "echo"), "echo is a command");
    check(has(clean, spans, BatToken::Comment, "rem a comment"), "rem comment");
    check(has(clean, spans, BatToken::Label, ":start"), "label");
    check(has(clean, spans, BatToken::Flow, "if"), "if is flow");
    check(has(clean, spans, BatToken::Flow, "errorlevel"), "errorlevel is flow");
    check(has(clean, spans, BatToken::Fail, "FAIL"), "FAIL word");
    check(has(clean, spans, BatToken::Pass, "PASS"), "PASS word");
    check(has(clean, spans, BatToken::Variable, "%errorlevel%"), "variable");
    check(has(clean, spans, BatToken::Variable, "%~dp0"), "variable inside a string");
    check(has(clean, spans, BatToken::Command, "ping"), "ping is a command");
    check(has(clean, spans, BatToken::Command, "find"), "find after a pipe is a command");
    check(has(clean, spans, BatToken::Flow, "else"), "else is flow");
    check(has(clean, spans, BatToken::Flow, "exist"), "exist is flow");
    check(has(clean, spans, BatToken::Flow, "goto"), "goto is flow");
    check(has(clean, spans, BatToken::Label, ":start") , "goto target is a label");
    check(has(clean, spans, BatToken::Command, "type"), "command after do");
    check(has(clean, spans, BatToken::Variable, "%%f"), "for variable");
    check(has(clean, spans, BatToken::Operator, ">>"), "append operator");
    check(has(clean, spans, BatToken::Switch, "-n") == false, "-n is not a switch (only / switches)");

    // a variable at the start of a line is the command: what follows is not a command
    const wxString line = "%1 %2 >nul 2>&1\n";
    const auto lineSpans = HighlightBatch(line);
    check(!has(line, lineSpans, BatToken::Command, "nul"), "nul after a redirection is not a command");

    // --- BatCommands
    check(BatCommands::all().size() > 80, "command table is filled");
    check(BatCommands::isCommand("FINDSTR") && BatCommands::isFlow("GoTo") && !BatCommands::isCommand("banana"), "word classes");

    std::printf(g_failures == 0 ? "All checks passed.\n" : "%d check(s) failed.\n", g_failures);
    return g_failures == 0 ? 0 : 1;
}
