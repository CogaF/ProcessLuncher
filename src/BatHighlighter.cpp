/*!
 * \file BatHighlighter.cpp
 * \brief Implementation of BatHighlighter.h.
 */

#include "BatHighlighter.h"

#include "BatCommands.h"

#include <wx/wxcrt.h>

namespace {

/*! \brief The kinds of lexical pieces of a line. */
enum class Lex { Word, Quoted, Variable, InnerVar, Op };

/*! \brief A lexical piece of a line. */
struct Piece
{
    size_t start;  /*!< position in the whole text. */
    size_t length; /*!< characters. */
    Lex    lex;    /*!< kind. */
    wxString text; /*!< the characters. */
};

/*! \brief true for the characters that end a word. */
bool isBreak(wxUniChar c)
{
    return c == ' ' || c == '\t' || c == '"' || c == '&' || c == '|' || c == '>' || c == '<' || c == '(' || c == ')' ||
           c == '^' || c == ',' || c == ';' || c == '=';
}

/*! \brief Length of the variable reference that starts at \p i (a '%' or '!'), 0 if there is none. */
size_t variableLength(const wxString& line, size_t i)
{
    const size_t n = line.length();
    const wxUniChar c = line[i];
    if (c == '%') {
        if (i + 1 >= n) return 0;
        if (line[i + 1] == '%') { // %%i of a for loop
            return (i + 2 < n && wxIsalpha(line[i + 2])) ? 3 : 0;
        }
        if (line[i + 1] == '*') return 2;
        if (wxIsdigit(line[i + 1])) return 2; // %1
        if (line[i + 1] == '~') { // %~dp0
            size_t j = i + 2;
            while (j < n && wxIsalpha(line[j])) j++; // modifiers: d p n x f s a t z
            if (j < n && wxIsdigit(line[j])) j++;     // the argument number
            return j - i;
        }
        // %NAME% (optionally with :~0,8 or :a=b modifiers inside)
        size_t j = i + 1;
        while (j < n && line[j] != '%' && line[j] != ' ' && line[j] != '"' && line[j] != '\t') j++;
        if (j < n && line[j] == '%' && j > i + 1) return j - i + 1;
        return 0;
    }
    if (c == '!') {
        size_t j = i + 1;
        while (j < n && line[j] != '!' && line[j] != ' ' && line[j] != '"' && line[j] != '\t') j++;
        if (j < n && line[j] == '!' && j > i + 1) return j - i + 1;
    }
    return 0;
}

/*! \brief Cuts a line into pieces. */
std::vector<Piece> lex(const wxString& line, size_t base)
{
    std::vector<Piece> pieces;
    const size_t n = line.length();
    size_t i = 0;
    while (i < n) {
        const wxUniChar c = line[i];
        if (c == ' ' || c == '\t' || c == ',' || c == ';') { i++; continue; }
        if (c == '"') {
            size_t j = i + 1;
            while (j < n && line[j] != '"') j++;
            if (j < n) j++; // closing quote
            pieces.push_back({ base + i, j - i, Lex::Quoted, line.Mid(i, j - i) });
            // Variables inside the quotes are listed as pieces of their own right after the string.
            for (size_t v = i + 1; v < j; v++) {
                if (line[v] == '%' || line[v] == '!') {
                    const size_t len = variableLength(line, v);
                    if (len > 0 && v + len <= j) { pieces.push_back({ base + v, len, Lex::InnerVar, line.Mid(v, len) }); v += len - 1; }
                }
            }
            i = j;
            continue;
        }
        if (c == '%' || c == '!') {
            const size_t len = variableLength(line, i);
            if (len > 0) {
                pieces.push_back({ base + i, len, Lex::Variable, line.Mid(i, len) });
                i += len;
                continue;
            }
        }
        if (c == '&' || c == '|' || c == '>' || c == '<' || c == '(' || c == ')' || c == '^' || c == '=') {
            // && || >> are one operator
            size_t len = 1;
            if (i + 1 < n && line[i + 1] == c && (c == '&' || c == '|' || c == '>' || c == '=')) len = 2;
            pieces.push_back({ base + i, len, Lex::Op, line.Mid(i, len) });
            i += len;
            continue;
        }
        size_t j = i;
        while (j < n && !isBreak(line[j])) {
            if ((line[j] == '%' || line[j] == '!') && j > i && variableLength(line, j) > 0) break;
            j++;
        }
        if (j == i) j = i + 1;
        pieces.push_back({ base + i, j - i, Lex::Word, line.Mid(i, j - i) });
        i = j;
    }
    return pieces;
}

/*! \brief Where we are inside an "if" condition. */
enum class IfStage { None, Modifiers, AfterOperand1, Operator, AfterOperator };

/*! \brief Analyses one line. */
void highlightLine(const wxString& line, size_t base, std::vector<BatSpan>& out)
{
    const std::vector<Piece> pieces = lex(line, base);
    if (pieces.empty()) return;

    bool atCommand = true;       // the next word is a command
    bool afterJump = false;      // the next word is the target of goto / call
    int  skipOperands = 0;       // operands still to be skipped (errorlevel N, exist F, defined V)
    IfStage ifStage = IfStage::None;

    const size_t lineEnd = base + line.length();
    for (size_t k = 0; k < pieces.size(); k++) {
        const Piece& p = pieces[k];

        // --- comments and labels
        if (p.lex == Lex::Word && atCommand) {
            if (p.text.StartsWith("::")) { out.push_back({ p.start, lineEnd - p.start, BatToken::Comment }); return; }
            if (p.text.StartsWith(":") && k == 0) { out.push_back({ p.start, p.length, BatToken::Label }); return; }
            wxString w = p.text;
            if (w.StartsWith("@")) w = w.Mid(1);
            if (w.CmpNoCase("rem") == 0) { out.push_back({ p.start, lineEnd - p.start, BatToken::Comment }); return; }
        }

        if (p.lex == Lex::Quoted) {
            out.push_back({ p.start, p.length, BatToken::String });
            if (atCommand && ifStage == IfStage::None) atCommand = false; // a program named in quotes
            if (skipOperands > 0) { skipOperands--; if (skipOperands == 0) atCommand = true; }
            else if (ifStage == IfStage::Modifiers) ifStage = IfStage::AfterOperand1;
            else if (ifStage == IfStage::AfterOperand1) ifStage = IfStage::Operator;
            else if (ifStage == IfStage::AfterOperator) { ifStage = IfStage::None; atCommand = true; }
            afterJump = false;
            continue;
        }
        if (p.lex == Lex::InnerVar) { // a variable inside a string: colour only, no effect on the state
            out.push_back({ p.start, p.length, BatToken::Variable });
            continue;
        }
        if (p.lex == Lex::Variable) {
            out.push_back({ p.start, p.length, BatToken::Variable });
            if (atCommand && ifStage == IfStage::None) atCommand = false; // %PROGRAM% is the command itself
            if (skipOperands > 0) { skipOperands--; if (skipOperands == 0) atCommand = true; }
            else if (ifStage == IfStage::Modifiers) ifStage = IfStage::AfterOperand1;
            else if (ifStage == IfStage::AfterOperand1) ifStage = IfStage::Operator;
            else if (ifStage == IfStage::AfterOperator) { ifStage = IfStage::None; atCommand = true; }
            afterJump = false;
            continue;
        }
        if (p.lex == Lex::Op) {
            out.push_back({ p.start, p.length, BatToken::Operator });
            if (p.text == "==" && ifStage == IfStage::Operator) { ifStage = IfStage::AfterOperator; continue; }
            if (p.text == "&" || p.text == "&&" || p.text == "|" || p.text == "||" || p.text == "(") {
                atCommand = true;
                ifStage = IfStage::None;
                skipOperands = 0;
            }
            else if (p.text == ")") {
                atCommand = true;
            }
            continue;
        }

        // --- words
        wxString word = p.text;
        size_t wordStart = p.start;
        if (word.StartsWith("@")) { // @echo off
            out.push_back({ p.start, 1, BatToken::Operator });
            word = word.Mid(1);
            wordStart++;
            if (word.empty()) continue;
        }
        const size_t wordLen = word.length();

        if (afterJump) {
            afterJump = false;
            out.push_back({ wordStart, wordLen, BatToken::Label });
            continue;
        }
        if (word.StartsWith("/") && word.length() <= 4 && !atCommand) {
            out.push_back({ wordStart, wordLen, BatToken::Switch });
            continue;
        }
        if (word == "PASS" || word == "FAIL") {
            out.push_back({ wordStart, wordLen, word == "PASS" ? BatToken::Pass : BatToken::Fail });
            continue;
        }

        // operands skipped after errorlevel / exist / defined
        if (skipOperands > 0) {
            skipOperands--;
            if (skipOperands == 0) atCommand = true;
            continue;
        }

        if (ifStage != IfStage::None && !atCommand) {
            if (BatCommands::isFlow(word)) {
                out.push_back({ wordStart, wordLen, BatToken::Flow });
                const wxString lw = word.Lower();
                if (lw == "errorlevel" || lw == "exist" || lw == "defined") { skipOperands = 1; ifStage = IfStage::None; }
                else if (lw == "equ" || lw == "neq" || lw == "lss" || lw == "leq" || lw == "gtr" || lw == "geq") {
                    ifStage = IfStage::AfterOperator;
                }
                continue;
            }
            if (word.Lower() == "/i") { out.push_back({ wordStart, wordLen, BatToken::Switch }); continue; }
            // a plain operand of a comparison
            if (ifStage == IfStage::Modifiers) ifStage = IfStage::AfterOperand1;
            else if (ifStage == IfStage::AfterOperand1) ifStage = IfStage::Operator;
            else if (ifStage == IfStage::AfterOperator) { ifStage = IfStage::None; atCommand = true; }
            continue;
        }

        if (atCommand) {
            if (BatCommands::isFlow(word)) {
                out.push_back({ wordStart, wordLen, BatToken::Flow });
                const wxString lw = word.Lower();
                if (lw == "if") { ifStage = IfStage::Modifiers; atCommand = false; }
                else if (lw == "else" || lw == "do") atCommand = true;
                else if (lw == "goto" || lw == "call") { atCommand = false; afterJump = true; }
                else atCommand = false;
            }
            else {
                out.push_back({ wordStart, wordLen, BatToken::Command });
                atCommand = false;
            }
            continue;
        }

        // not in command position: flow words still stand out (for ... in ... do, not, else)
        if (BatCommands::isFlow(word)) {
            const wxString lw = word.Lower();
            if (lw == "do" || lw == "else" || lw == "in" || lw == "not") {
                out.push_back({ wordStart, wordLen, BatToken::Flow });
                if (lw == "do" || lw == "else") atCommand = true;
            }
        }
    }
}

} // namespace

std::vector<BatSpan> HighlightBatch(const wxString& rawText)
{
    wxString text = rawText;
    text.Replace("\r", "");

    std::vector<BatSpan> spans;
    size_t lineStart = 0;
    const size_t n = text.length();
    while (lineStart <= n) {
        size_t lineEnd = text.find('\n', lineStart);
        if (lineEnd == wxString::npos) lineEnd = n;
        highlightLine(text.Mid(lineStart, lineEnd - lineStart), lineStart, spans);
        lineStart = lineEnd + 1;
    }
    return spans;
}
