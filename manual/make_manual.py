"""
Process Launcher user manual - builds manual/Process_Launcher_User_Manual.pdf from the text below.

    python manual/make_manual.py

Needs reportlab and pillow (python -m pip install reportlab pillow). Fonts: Segoe UI and Consolas from
the Windows fonts folder, Liberation Sans / DejaVu Sans Mono elsewhere.

Read from the source, so the manual follows the program without editing this file:
  - owner, copyright, contact, project page: include/AppInfo.h (manual_legal.py);
  - version and release date: the top entry of kAppVersionHistory in include/Version.h;
  - product code, features, editions, trial days: include/LicensePolicy.h;
  - the list of example batch files: line 4 of every bat_examples/NN_*.bat.
Pictures are optional: a file listed in PICTURES that exists in manual/images is shown where it belongs;
"Process Launcher.exe" --screenshots manual\\images makes them (the License window is left out: it shows the UID).

When a release changes something this manual states, change the text, add a line to REVISIONS and
raise REVISION.
"""
import io
import os
import re
import sys

from PIL import Image
from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (BaseDocTemplate, Frame, Image as RLImage, KeepTogether, NextPageTemplate,
                                PageBreak, PageTemplate, Paragraph, Spacer, Table, TableStyle)
from reportlab.platypus.tableofcontents import TableOfContents

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
IMAGES = os.path.join(HERE, 'images')
OUTPUT = os.path.join(HERE, 'Process_Launcher_User_Manual.pdf')

sys.path.insert(0, HERE)
from manual_legal import (OWNER, CONTACT, WEBSITE, PRODUCT, COPYRIGHT_LINE,  # noqa: E402
                          SECTIONS as LEGAL, TITLE as LEGAL_TITLE)

DOC_ID = 'PCR-UM-001'
REVISION = '1.1'
DATE = '9 October 2026'
REVISIONS = [
    ('1.0', DATE, 'First issue: commands terminated with the window, rows unlocked after a run, batch files saved '
                  'in their own encoding.'),
    ('1.1', DATE, 'Version 0.2.0-rc.2: projects (rows kept), time limit and Stop per row, :Exit: and {id} in the '
                  'expected result, PCR_CMD_ID, Repeat with summary, CSV export, command line, screenshots.'),
]
AUTHOR = OWNER
COPYRIGHT = COPYRIGHT_LINE


# --- Facts read from the source ---------------------------------------------------------------------
def _read(*parts):
    with open(os.path.join(ROOT, *parts), encoding='utf-8') as f:
        return f.read()


_version_h = _read('include', 'Version.h')
_top = re.search(r'kAppVersionHistory\[\]\s*=\s*\{\s*\{\s*"([^"]+)",\s*"([^"]+)"', _version_h)
VERSION, RELEASE_DATE = (_top.group(1), _top.group(2)) if _top else ('?', '?')

_policy_h = _read('include', 'LicensePolicy.h')
PRODUCT_CODE = re.search(r'kProduct\s*=\s*"([^"]+)"', _policy_h).group(1)
TRIAL_DAYS = int(re.search(r'kTrialDays\s*=\s*(\d+)', _policy_h).group(1))
FEATURES = re.findall(r'kFeature\w+\s*=\s*"([^"]+)"', _policy_h)
EDITIONS = re.findall(r'\{\s*"(\w+)",\s*"([^"]+)"\s*\}', _policy_h)

EXAMPLES = []  # (file, what it checks)
_examples_dir = os.path.join(ROOT, 'bat_examples')
for _name in sorted(os.listdir(_examples_dir)):
    if re.match(r'\d\d_.*\.bat$', _name, re.I):
        with open(os.path.join(_examples_dir, _name), encoding='utf-8', errors='replace') as f:
            _lines = f.read().splitlines()
        _m = re.match(r'rem\s+\S+\s+-\s+(.*)', _lines[3].strip()) if len(_lines) > 3 else None
        EXAMPLES.append((_name, _m.group(1) if _m else ''))

# --- Fonts -------------------------------------------------------------------------------------------
FONTS = os.path.join(os.environ.get('WINDIR', r'C:\Windows'), 'Fonts')
_FONT_SETS = [
    [os.path.join(FONTS, f) for f in ('segoeui.ttf', 'segoeuib.ttf', 'segoeuii.ttf', 'segoeuiz.ttf', 'consola.ttf')],
    ['/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf', '/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf',
     '/usr/share/fonts/truetype/liberation/LiberationSans-Italic.ttf', '/usr/share/fonts/truetype/liberation/LiberationSans-BoldItalic.ttf',
     '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf'],
]
for _fonts in _FONT_SETS:  # Segoe UI / Consolas on Windows, Liberation Sans / DejaVu Sans Mono elsewhere
    if all(os.path.exists(f) for f in _fonts):
        for _name, _file in zip(('Body', 'Body-Bold', 'Body-Italic', 'Body-BoldItalic', 'Mono'), _fonts):
            pdfmetrics.registerFont(TTFont(_name, _file))
        break
else:
    sys.exit('No usable fonts (Segoe UI / Consolas or Liberation Sans / DejaVu Sans Mono).')
pdfmetrics.registerFontFamily('Body', normal='Body', bold='Body-Bold', italic='Body-Italic', boldItalic='Body-BoldItalic')

NAVY = colors.HexColor('#1F3A5F')
GREY = colors.HexColor('#5A6270')
LIGHT = colors.HexColor('#EEF2F7')
RULE = colors.HexColor('#B8C2D0')

# --- Styles ------------------------------------------------------------------------------------------
S = {
    'body': ParagraphStyle('body', fontName='Body', fontSize=10, leading=14.2, alignment=TA_JUSTIFY, spaceAfter=6),
    'bullet': ParagraphStyle('bullet', fontName='Body', fontSize=10, leading=14, leftIndent=14, bulletIndent=4, spaceAfter=3),
    'step': ParagraphStyle('step', fontName='Body', fontSize=10, leading=14, leftIndent=18, bulletIndent=2, spaceAfter=3),
    'h1': ParagraphStyle('h1', fontName='Body-Bold', fontSize=17, leading=22, textColor=NAVY, spaceBefore=6, spaceAfter=10),
    'h2': ParagraphStyle('h2', fontName='Body-Bold', fontSize=12.5, leading=17, textColor=NAVY, spaceBefore=10, spaceAfter=5),
    'h3': ParagraphStyle('h3', fontName='Body-Bold', fontSize=10.5, leading=14, textColor=colors.black, spaceBefore=6, spaceAfter=3),
    'caption': ParagraphStyle('caption', fontName='Body-Italic', fontSize=8.8, leading=11.5, textColor=GREY, alignment=TA_CENTER, spaceBefore=3, spaceAfter=10),
    'cell': ParagraphStyle('cell', fontName='Body', fontSize=8.8, leading=11.6, alignment=TA_LEFT),
    'cellb': ParagraphStyle('cellb', fontName='Body-Bold', fontSize=8.8, leading=11.6, textColor=colors.white),
    'code': ParagraphStyle('code', fontName='Mono', fontSize=8.4, leading=11, leftIndent=10, backColor=LIGHT,
                           borderPadding=(5, 6, 5, 6), spaceBefore=4, spaceAfter=10),
    'note': ParagraphStyle('note', fontName='Body', fontSize=9.4, leading=13.2, leftIndent=8, borderPadding=(6, 8, 6, 8),
                           backColor=colors.HexColor('#FFF7E0'), borderColor=colors.HexColor('#E0B84C'), borderWidth=0.6,
                           spaceBefore=4, spaceAfter=10),
    'toc0': ParagraphStyle('toc0', fontName='Body-Bold', fontSize=10.5, leading=16, leftIndent=0),
    'toc1': ParagraphStyle('toc1', fontName='Body', fontSize=9.6, leading=13.5, leftIndent=16),
    'cover_title': ParagraphStyle('ct', fontName='Body-Bold', fontSize=30, leading=36, textColor=NAVY, alignment=TA_LEFT),
    'cover_sub': ParagraphStyle('cs', fontName='Body', fontSize=15, leading=20, textColor=GREY, alignment=TA_LEFT),
}


class Manual(BaseDocTemplate):
    """Numbered headings, table of contents entries and PDF outline."""

    def __init__(self, path):
        super().__init__(path, pagesize=A4, leftMargin=22 * mm, rightMargin=22 * mm, topMargin=24 * mm, bottomMargin=22 * mm,
                         title=f'{PRODUCT} - User Manual', author=AUTHOR, subject=f'{PRODUCT} {VERSION} user manual - {COPYRIGHT}',
                         keywords=f'{PRODUCT}, PCR, {COPYRIGHT}', creator=f'{PRODUCT} documentation - {AUTHOR}',
                         producer=f'{AUTHOR} ({PRODUCT})')
        frame = Frame(self.leftMargin, self.bottomMargin, self.width, self.height, id='normal')
        self.addPageTemplates([PageTemplate(id='cover', frames=[frame], onPage=self.cover_page),
                               PageTemplate(id='body', frames=[frame], onPage=self.body_page)])

    def afterFlowable(self, flowable):
        if isinstance(flowable, Paragraph) and hasattr(flowable, 'toc_level'):
            text = flowable.getPlainText()
            key = f'h{id(flowable)}'
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(text, key, level=flowable.toc_level, closed=False)
            self.notify('TOCEntry', (flowable.toc_level, text, self.page, key))

    @staticmethod
    def cover_page(canvas, doc):
        canvas.saveState()
        w, h = A4
        canvas.setFillColor(NAVY)
        canvas.rect(0, h - 18 * mm, w, 18 * mm, stroke=0, fill=1)
        canvas.rect(0, 0, w, 10 * mm, stroke=0, fill=1)
        canvas.setFillColor(colors.white)
        canvas.setFont('Body', 8.5)
        canvas.drawString(22 * mm, 4 * mm, COPYRIGHT)
        canvas.drawRightString(w - 22 * mm, 4 * mm, f'{DOC_ID}  Rev. {REVISION}')
        canvas.restoreState()

    @staticmethod
    def body_page(canvas, doc):
        canvas.saveState()
        w, h = A4
        canvas.setStrokeColor(RULE)
        canvas.setLineWidth(0.6)
        canvas.line(22 * mm, h - 16 * mm, w - 22 * mm, h - 16 * mm)
        canvas.line(22 * mm, 15 * mm, w - 22 * mm, 15 * mm)
        canvas.setFont('Body', 8.2)
        canvas.setFillColor(GREY)
        canvas.drawString(22 * mm, h - 14 * mm, f'{PRODUCT} {VERSION} - User Manual')
        canvas.drawRightString(w - 22 * mm, h - 14 * mm, f'{DOC_ID}  Rev. {REVISION}')
        canvas.drawString(22 * mm, 10.5 * mm, COPYRIGHT)
        canvas.drawRightString(w - 22 * mm, 10.5 * mm, f'Page {doc.page}')
        canvas.restoreState()


# --- Helpers -----------------------------------------------------------------------------------------
story = []
chapter = [0, 0]
figure = [0]
table_no = [0]


def esc(text):
    return text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')


def h1(text):
    chapter[0] += 1
    chapter[1] = 0
    p = Paragraph(f'{chapter[0]}&nbsp;&nbsp;{text}', S['h1'])
    p.toc_level = 0
    story.append(p)


def h2(text):
    chapter[1] += 1
    p = Paragraph(f'{chapter[0]}.{chapter[1]}&nbsp;&nbsp;{text}', S['h2'])
    p.toc_level = 1
    story.append(p)


def h3(text):
    story.append(Paragraph(text, S['h3']))


def para(text):
    story.append(Paragraph(text, S['body']))


def bullets(items):
    for item in items:
        story.append(Paragraph(item, S['bullet'], bulletText='•'))
    story.append(Spacer(1, 4))


def steps(items):
    for i, item in enumerate(items, 1):
        story.append(Paragraph(item, S['step'], bulletText=f'{i}.'))
    story.append(Spacer(1, 4))


def note(text, label='Note'):
    story.append(Paragraph(f'<b>{label}.</b> {text}', S['note']))


def code(text):
    story.append(Paragraph(esc(text).replace('\n', '<br/>').replace(' ', '&nbsp;'), S['code']))


def picture(name, caption, width_mm=166):
    """A screenshot from manual/images, scaled to width_mm; nothing when the file does not exist."""
    path = os.path.join(IMAGES, name)
    if not os.path.exists(path):
        return
    im = Image.open(path).convert('RGB')
    buf = io.BytesIO()
    im.save(buf, 'PNG')
    buf.seek(0)
    w, h = im.size
    width = min(width_mm, 166) * mm
    img = RLImage(buf, width=width, height=width * h / w)
    img.hAlign = 'CENTER'
    figure[0] += 1
    frame = Table([[img]], style=TableStyle([('BOX', (0, 0), (-1, -1), 0.5, RULE), ('LEFTPADDING', (0, 0), (-1, -1), 0),
                                             ('RIGHTPADDING', (0, 0), (-1, -1), 0), ('TOPPADDING', (0, 0), (-1, -1), 0),
                                             ('BOTTOMPADDING', (0, 0), (-1, -1), 0)]))
    story.append(KeepTogether([Spacer(1, 4), frame, Paragraph(f'Figure {figure[0]} - {caption}', S['caption'])]))


def table(header, rows, widths, caption=None):
    data = [[Paragraph(c, S['cellb']) for c in header]] + [[Paragraph(c, S['cell']) for c in r] for r in rows]
    t = Table(data, colWidths=[w * mm for w in widths], repeatRows=1, hAlign='LEFT')
    t.setStyle(TableStyle([
        ('BACKGROUND', (0, 0), (-1, 0), NAVY),
        ('ROWBACKGROUNDS', (0, 1), (-1, -1), [colors.white, LIGHT]),
        ('GRID', (0, 0), (-1, -1), 0.4, RULE),
        ('VALIGN', (0, 0), (-1, -1), 'TOP'),
        ('TOPPADDING', (0, 0), (-1, -1), 3.5), ('BOTTOMPADDING', (0, 0), (-1, -1), 3.5),
        ('LEFTPADDING', (0, 0), (-1, -1), 5), ('RIGHTPADDING', (0, 0), (-1, -1), 5),
    ]))
    story.append(t)
    if caption:
        table_no[0] += 1
        story.append(Paragraph(f'Table {table_no[0]} - {caption}', S['caption']))
    else:
        story.append(Spacer(1, 8))


def ui(text):
    """A user interface element."""
    return f'<b>{text}</b>'


def mono(text):
    return f'<font name="Mono" size="8.8">{esc(text)}</font>'


# Optional pictures (manual/images): name -> caption.
PICTURES = {
    'main-window.png': 'The main window',
    'batch-editor.png': 'The batch file editor',
    'about.png': 'The About window',
}

# =====================================================================================================
# Cover
# =====================================================================================================
story.append(Spacer(1, 42 * mm))
story.append(Paragraph(PRODUCT, S['cover_title']))
story.append(Spacer(1, 4 * mm))
story.append(Paragraph('PCR - Parallel Command Runner', S['cover_sub']))
story.append(Spacer(1, 14 * mm))
story.append(Paragraph('<b>User Manual</b>', ParagraphStyle('x', parent=S['cover_sub'], fontSize=22, leading=28, textColor=colors.black)))
story.append(Spacer(1, 3 * mm))
story.append(Paragraph(f'Software version {VERSION}', S['cover_sub']))
story.append(Spacer(1, 40 * mm))
cover = [
    ['Document', DOC_ID],
    ['Revision', REVISION],
    ['Date', DATE],
    ['Author', AUTHOR],
    ['Contact', CONTACT],
    ['Project page', WEBSITE],
    ['Classification', f'Proprietary - for users of {PRODUCT}'],
]
t = Table([[Paragraph(f'<b>{a}</b>', S['cell']), Paragraph(b, S['cell'])] for a, b in cover], colWidths=[38 * mm, 110 * mm], hAlign='LEFT')
t.setStyle(TableStyle([('LINEBELOW', (0, 0), (-1, -1), 0.4, RULE), ('TOPPADDING', (0, 0), (-1, -1), 4), ('BOTTOMPADDING', (0, 0), (-1, -1), 4)]))
story.append(t)
story.append(Spacer(1, 10 * mm))
story.append(Paragraph(f'<b>{COPYRIGHT}</b>', S['cell']))
story.append(NextPageTemplate('body'))
story.append(PageBreak())

story.append(Paragraph(LEGAL_TITLE, S['h1']))
for _head, _text in LEGAL:
    story.append(Paragraph(f'<b>{_head}</b>', S['h3']))
    story.append(Paragraph(esc(_text), S['body']))
story.append(PageBreak())

# =====================================================================================================
# Document control and contents
# =====================================================================================================
story.append(Paragraph('Document control', S['h1']))
table(['Revision', 'Date', 'Author', 'Description'], [[r, d, AUTHOR, text] for r, d, text in REVISIONS], [20, 30, 32, 84])
para(f'This manual describes {PRODUCT} version {VERSION} (released {RELEASE_DATE}) on Microsoft Windows 10 and 11, '
     'together with the corrections made after that release which change what the program does (see the document control '
     'table above and the release notes).')
para(f'Questions about this document, license requests and problem reports go to the author at <b>{CONTACT}</b>, or to '
     f'<b>{WEBSITE}/issues</b>.')
story.append(Spacer(1, 6))
story.append(Paragraph('Contents', S['h1']))
toc = TableOfContents()
toc.levelStyles = [S['toc0'], S['toc1']]
toc.dotsMinLevel = 0
story.append(toc)
story.append(PageBreak())

# =====================================================================================================
# 1 Introduction
# =====================================================================================================
h1('Introduction')
h2('Purpose')
para(f'{PRODUCT} (PCR, Parallel Command Runner) starts a list of commands - batch files, programs, anything '
     + mono('cmd /c') + ' understands - in parallel or one after the other, looks in their output (or in a result file) for '
     'an expected text and records every run as <b>PASS</b> or <b>FAIL</b>, with pass / fail counters for each command.')
para('Typical uses: functional and end-of-line tests driven by batch files, checks of a PC or a test bench before a '
     'session (network, services, disk space, tools installed), repeated runs of the same command to catch intermittent '
     'failures, and smoke tests after an installation.')
h2('Typographic conventions')
table(['Convention', 'Meaning'],
      [[ui('Bold'), 'An element of the user interface: menu, button, field.'],
       ['Settings &gt; Stop waiting', 'A menu command: the menu, then the command.'],
       [mono(':File:::PASS'), 'Text typed exactly as shown: a command, an expected result, a file name.'],
       ['Note / Caution', 'Information that prevents an error.']],
      [42, 124], 'Typographic conventions')
h2('Terms')
table(['Term', 'Definition'],
      [['Command row', 'One line of the command table: a command, its expected result and its options.'],
       ['Expected result', 'The text that must be found for the run to be a PASS.'],
       ['Result file', 'A text file where batch files append their time stamped PASS / FAIL lines.'],
       ['Parallel command', 'Started together with the other parallel commands.'],
       ['Single command', 'Started alone: "Run command(s)" waits for it before starting the next rows.'],
       ['Project', 'A file (.pcr) holding the rows, the result file and the repeat count.'],
       ['Run', 'One pass of "Run command(s)" over the rows; Repeat makes several.']],
      [38, 128], 'Terms')

# =====================================================================================================
# 2 Installation
# =====================================================================================================
h1('Installation')
h2('Requirements')
bullets(['Windows 10 or 11, 64 or 32 bit.',
         'Microsoft Visual C++ Redistributable 2015-2022 (or later) of the same bitness: without it Windows reports a '
         'missing ' + mono('VCRUNTIME140.dll') + ' or ' + mono('MSVCP140.dll') + '.',
         'A folder the user can write to: the program keeps its data next to the executable.'])
h2('Packages')
table(['Package', 'Contents', 'Use it when'],
      [['x64 static', mono('Process Launcher.exe') + ', ' + mono('bat_examples\\'), 'normal 64-bit Windows: one file'],
       ['Win32 static', 'the same, 32 bit', '32-bit Windows'],
       ['x64 / Win32 DLL', 'the executable and the two wxWidgets DLLs', 'several wxWidgets programs share the DLLs']],
      [32, 74, 60], 'Packages')
para('There is no installer: unzip the package anywhere. Keep the DLLs (DLL packages) in the folder of the executable. '
     'The binaries are not code-signed, so Windows SmartScreen may warn at the first start (More info &gt; Run anyway). '
     'Check a download with ' + mono('certutil -hashfile "Process Launcher.exe" SHA256') + ' against the published SHA-256.')
h2('Files and folders')
table(['Path (next to the executable)', 'Contents'],
      [[mono('Process Launcher data\\'), 'Created at the first start. ' + mono('log.txt') + ' (log of the sessions), '
        + mono('settings.ini') + ' (result file, last project, editor text size), ' + mono('commands.pcr') + ' (the default '
        'project), the license and the trial state. If the folder cannot be created the files are written next to the '
        'executable.'],
       [mono('result.txt'), 'The default result file (see chapter 5).'],
       [mono('bat_examples\\'), 'The example batch files (Appendix B).']],
      [52, 114], 'Files and folders')

# =====================================================================================================
# 3 Main window
# =====================================================================================================
h1('The main window')
picture('main-window.png', PICTURES['main-window.png'])
para('From the top: the ' + ui('Run command(s)') + ' button with the ' + ui('Repeat') + ' field, the ' + ui('Result file') + ' entry, the table of commands '
     'and the list of results. The program always starts in dark mode. The status bar shows the menu help on the left and '
     'the license state on the right.')
h2('Command rows')
table(['Field', 'Meaning'],
      [[ui('ON / OFF'), 'Whether ' + ui('Run command(s)') + ' starts the command. An OFF row is greyed out.'],
       [ui('Run / Stop'), 'Starts only this command, at once (it never blocks the others). While the command runs the '
        'button reads Stop: it terminates the command and everything it started (counted as FAIL).'],
       ['command', 'Anything ' + mono('cmd /c') + ' understands: ' + mono('bat_examples\\02_ping_host.bat 192.168.1.1') + ', '
        + mono('ping -n 1 127.0.0.1') + ', a program with its arguments.'],
       ['expected result', 'The text to find, the result file or the exit code to check (chapter 5).'],
       ['time limit', 'Seconds; a command still running then is terminated with everything it started and counted as '
        'FAIL. 0 = no limit.'],
       ['counters', mono('P= nnn || F= nnn') + ': how many runs passed and failed since the program started.'],
       [ui('Parallel / Single'), 'Parallel commands start together; a Single command starts alone (chapter 4.3).'],
       [ui('Hide'), 'Reserved for showing the console of a command; not available in this version.'],
       [ui('Busy / Ready'), 'The command is running / idle (indicator only).']],
      [34, 132], 'Fields of a command row')
note('The rows are saved in the current project when the program closes and at every Run command(s), and come back at '
     'the next start (chapter 6).')
h2('The list of results')
para('Each run adds a line at the top: the time and ' + mono('Cmd n PASS|FAIL, result is: <output>') + ', green for PASS, '
     'red for FAIL. A problem of the check itself (a missing file, a malformed expected result) adds a red line before it. '
     'In the list ' + ui('Ctrl+A') + ' selects every line and ' + ui('Ctrl+C') + ' copies the selected lines, tab separated '
     '(they paste into a spreadsheet as two columns); the tooltip shows the whole text of the line under the mouse.')
h2('Menus')
table(['Menu', 'Command', 'Keys', 'Action'],
      [['File', 'Open project...', '', 'Opens a project file (chapter 6).'],
       ['File', 'Save project / Save project as...', 'Ctrl+S', 'Saves the rows in the current / another project file.'],
       ['File', 'Export results (CSV)...', '', 'Saves the results of the session as a CSV file (chapter 7).'],
       ['File', 'Clear results', '', 'Empties the result list.'],
       ['File', 'New batch file', 'Ctrl+N', 'Opens the batch editor with the PASS / FAIL skeleton.'],
       ['File', 'Open batch file...', 'Ctrl+O', 'Opens a batch file in the editor.'],
       ['File', 'Open examples folder', '', 'Opens ' + mono('bat_examples') + ' in Explorer.'],
       ['File', 'Exit', '', 'Closes the program (see 4.7).'],
       ['Settings', 'Enable Edit / Disable Edit', 'Ctrl+E / Ctrl+D', 'Unlocks / locks the editable fields of the rows.'],
       ['Settings', 'Select result file...', '', 'Chooses the result file.'],
       ['Settings', 'Open result file', '', 'Opens the result file with its default program.'],
       ['Settings', 'Open data folder', '', 'Opens the data folder (log, settings, license).'],
       ['Settings', 'Stop waiting', 'Ctrl+B', 'Releases the Single command (or the end of a run) that Run command(s) '
        'waits for.'],
       ['Settings', 'Stop repeating', 'Ctrl+R', 'The current run ends, the next repetition is not started.'],
       ['Info', 'License...', 'Ctrl+K', 'The License window (chapter 11).'],
       ['Info', 'About...', '', 'Version, author, license, changes and system information.']],
      [18, 40, 26, 82], 'Menus')

# =====================================================================================================
# 4 Running commands
# =====================================================================================================
h1('Running commands')
h2('How a command runs')
bullets(['The command line is run as ' + mono('cmd /c "<command>"') + ' without any visible window.',
         'Its current folder is the folder of the executable, so relative paths such as '
         + mono('bat_examples\\01_minimal_pass_fail.bat') + ' work however the program was started.',
         'Standard output and standard error are captured together; standard input is empty, so a command that waits for '
         'a key (' + mono('pause') + ', ' + mono('set /p') + ') ends instead of hanging.',
         'Each command gets its own environment with ' + mono('PCR_RESULT_FILE') + ' (full path of the result file), '
         + mono('PCR_APP_DIR') + ' (folder of the executable) and ' + mono('PCR_CMD_ID') + ' (its row number).',
         'The output is decoded with the OEM code page of the console, so accented letters appear correctly.',
         'Every command runs in its own thread: the window stays responsive. Its exit code and duration are shown in the '
         'result list; the exit code decides PASS or FAIL only with an expected result ' + mono(':Exit:') + ' (5.1).',
         'Every command runs in its own Windows job: Stop, the time limit and the closing of the program terminate the '
         'command together with every program it started.'])
h2('Run command(s)')
steps(['If a command of an earlier run is still running, the program asks whether to start the others; that command is '
       'skipped.',
       'The editable fields of every row are locked.',
       'The ON rows are started from the top: Parallel ones at once, Single ones as described in 4.3. A row whose expected '
       'result needs the result file is started only once the file exists (5.3).',
       'Run command(s) waits until every command of the run has ended (' + ui('Settings &gt; Stop waiting') + ', Ctrl+B, '
       'releases the wait), then adds a summary line: ' + mono('Run 1/3 ended: 4 PASS, 1 FAIL (CMD 2)') + ', green when '
       'nothing failed.',
       'When the last command has ended the rows are unlocked again - unless ' + ui('Settings &gt; Disable Edit') + ' locked '
       'them, in which case ' + ui('Enable Edit') + ' unlocks them.'])
para('The rows are saved in the current project before the run starts.')
h2('Single commands')
para('When ' + ui('Run command(s)') + ' reaches a Single row it shows a message: the next rows wait for this command. Make '
     'sure the commands still running in parallel are not needed by it, then click OK. If any earlier row failed in this '
     'run, the program names those rows and asks whether to go on; answering No skips the Single command.')
para('While a Single command runs the window stays usable. ' + ui('Settings &gt; Stop waiting') + ' (Ctrl+B) stops waiting '
     'for it: the next rows start, the command goes on and its result is still recorded.')
note('Single mode needs the ' + mono('sequential') + ' feature of the license; without it the check box goes back to '
     'Parallel.')
h2('Repeat')
para('The ' + ui('Repeat') + ' field next to the Run button says how many runs Run command(s) makes; each run starts '
     'when every command of the previous one has ended. 0 repeats until ' + ui('Settings &gt; Stop repeating') + ' '
     '(Ctrl+R), which lets the current run end. After the last run a total line follows, for example '
     + mono('Repeat ended after 100 run(s): 498 PASS, 2 FAIL') + '. The questions of Single commands are asked in the first run '
     'only; in the later runs a previous failure is only recorded in the list. Repeat is saved in the project.')
note('Use Repeat to catch intermittent failures: the per-row counters and the CSV export (chapter 7) show which command '
     'failed and in which run.')
h2('Time limit and Stop')
para('A row with a time limit (seconds) is terminated when it is still running after that time: the list shows '
     + mono('CMD n terminated: time limit of 30 s expired') + ' and the run counts as FAIL. The ' + ui('Stop') + ' button '
     'of a running row does the same at once. In both cases every program the command started is terminated too.')
h2('Commands that share the result file')
para('Commands running at the same time append to the same result file. The expected result ' + mono(':File:::PASS') + ' '
     'would also accept the PASS line of another command, so put the name of the batch file in it, as the batch files of '
     + mono('bat_examples') + ' write it: ' + mono(':File:::[02_ping_host] PASS') + '. For two rows that run the same batch '
     'file in parallel, let the batch file add its row number ' + mono('%PCR_CMD_ID%') + ' to the tag - '
     + mono('set "TEST_NAME=%~n0#%PCR_CMD_ID%"') + ' - and write ' + mono('{id}') + ' in the expected result: '
     + mono(':File:::[02_ping_host#{id}] PASS') + '.')
h2('Closing the program')
para('If commands are still running, the program asks for confirmation. Closing then <b>terminates the unfinished '
     'commands together with every program they started</b>; their results are dropped. Programs left running on purpose '
     'by commands that had already ended (for example with ' + mono('start') + ') are not touched. Open batch editors are '
     'closed first and ask to save their changes; answering Cancel keeps the program open.')

# =====================================================================================================
# 5 Expected result and result file
# =====================================================================================================
h1('Expected result and result file')
h2('Forms of the expected result')
table(['Expected result', 'PASS when'],
      [['plain text, e.g. ' + mono('TTL='), 'the console output of the command contains the text (case sensitive).'],
       [mono(':File:<path>::<text>'), 'the file ' + mono('<path>') + ' contains ' + mono('<text>') + ' on one line. The '
        'whole file is searched. ' + mono('%VARIABLES%') + ' in the path are expanded.'],
       [mono(':File:::<text>'), 'the <b>result file</b> contains ' + mono('<text>') + ' in a line written <b>after the '
        'command started</b>; a PASS left by an earlier run never counts.'],
       [mono(':Exit:<codes>'), 'the exit code of the command is one of the codes: a comma separated list of numbers and '
        'ranges, e.g. ' + mono(':Exit:0') + ', ' + mono(':Exit:0-7') + ' (robocopy), ' + mono(':Exit:0,3,10-12') + '. The '
        'output is not looked at.']],
      [48, 118], 'Forms of the expected result')
para('The first ' + mono('::') + ' after ' + mono(':File:') + ' ends the path (the colon of a drive letter stands alone). '
     'The text cannot span two lines. An expected result that starts with ' + mono(':File:') + ' but has no ' + mono('::')
     + ' is reported as malformed and counted as FAIL, as is a ' + mono(':Exit:') + ' without valid codes. ' + mono('{id}')
     + ' anywhere in the expected result is replaced by the row number.')
note('Without ' + mono('@echo off') + ' at the top of a batch file, cmd.exe prints every command before running it, so '
     'the expected text is found inside the printed command: a false PASS. Keep the word PASS out of the FAIL lines and '
     'the word FAIL out of the PASS lines.', 'Caution')
h2('The result file')
para('The ' + ui('Result file') + ' entry under the Run button shows its full path - by default ' + mono('result.txt') +
     ' in the folder of the executable. Type another path and press Enter (a relative path is relative to the executable), '
     'or click ' + ui('...') + ' to choose one; the choice is kept in ' + mono('settings.ini') + '. The file is only ever '
     'appended to, never cleared: ' + ui('Settings &gt; Open result file') + ' opens it to read or empty it by hand.')
h2('When the result file does not exist')
para('A command whose expected result uses the result file is started only when the file exists. Otherwise the program '
     'asks: ' + ui('Create here') + ' creates it at the path shown (with its folder), ' + ui('Choose location...') + ' '
     'chooses another place, ' + ui('Cancel') + ' leaves the command not started (a line in the list says so).')

# =====================================================================================================
# 6 Projects
# =====================================================================================================
h1('Projects')
para('A project file (' + mono('.pcr') + ', INI text) holds the rows - ON / OFF, Single, command, expected result, time '
     'limit - the result file and the repeat count. The current project is named in the window title.')
bullets(['The rows are saved in the current project when the program closes and when Run command(s) starts, and the '
         'project is opened again at the next start.',
         'Until another is chosen the project is ' + mono('Process Launcher data\\commands.pcr') + '.',
         ui('File &gt; Save project as...') + ' saves the rows in a new file, which becomes the current project; '
         + ui('File &gt; Open project...') + ' saves the current rows, then loads another project.',
         'A project file can be given on the command line (chapter 8), also by associating ' + mono('.pcr') + ' files with '
         'the program in Windows.',
         'A project that cannot be read (damaged, or written by a newer version) is never overwritten: the default '
         'project is used, and a damaged default project is kept as ' + mono('commands.pcr.bad') + '.'])
code('[Project]\nVersion=1\nResultFile=C:\\Tests\\result.txt\nRepeat=1\n[Row01]\nActive=1\nSingle=0\n'
     'Command=bat_examples\\02_ping_host.bat 192.168.1.1\nExpected=:File:::[02_ping_host] PASS\nTimeout=30')
note('A project holds at most as many rows as the window shows (11); further rows are ignored with a message.')

# =====================================================================================================
# 7 Results export
# =====================================================================================================
h1('Exporting the results')
para(ui('File &gt; Export results (CSV)...') + ' saves every result since the start (or the last ' + ui('Clear results')
     + ') as a CSV file that a spreadsheet opens in columns: the separator is the list separator of the Windows regional '
     'settings (; where the decimal sign is a comma, as in Italy), the file is UTF-8 so accents are kept.')
table(['Column', 'Contents'],
      [['Timestamp', 'When the command ended.'], ['Run', 'Number of the run of Run command(s); 0 = Run button of the row.'],
       ['Row', 'Row number.'], ['Command / Expected', 'What ran and what was looked for.'], ['Result', 'PASS or FAIL.'],
       ['Exit code', 'Exit code of the command, -1 if unknown.'], ['Duration (s)', 'How long it ran.'],
       ['Note', 'Why the check failed or the command was terminated.'], ['Output', 'The console output.']],
      [40, 126], 'Columns of the CSV file')

# =====================================================================================================
# 8 Command line
# =====================================================================================================
h1('Command line')
code('"Process Launcher.exe" [project.pcr] [--project <file>] [--run]\n'
     '                       [--repeat <n>] [--csv <file>] [--screenshots <folder>]')
table(['Option', 'Effect'],
      [[mono('project.pcr') + ', ' + mono('--project <file>'), 'Opens this project instead of the last one.'],
       [mono('--run'), 'Runs the rows at once without any question, exports the CSV if asked and closes.'],
       [mono('--repeat <n>'), 'Repeat count for this start (0 = until stopped).'],
       [mono('--csv <file>'), 'With --run: exports the results to this file.'],
       [mono('--screenshots <folder>'), 'Saves pictures of the main window, the batch editor and the About window '
        '(demonstration rows, run when licensed) and closes; used to illustrate this manual.']],
      [50, 116], 'Command line options')
para('With ' + mono('--run') + ' the exit code is <b>0</b> when every result is PASS, <b>1</b> when at least one is FAIL '
     'and <b>2</b> when nothing could run (no license, unreadable project, no row ON). A missing result file is created '
     'without asking. Neither the project nor the settings are written, and a project given with --project does not become '
     'the one opened at the next start. A summary line is printed to the console the program was started from.')
code('start /wait "" "Process Launcher.exe" --run --project tests.pcr ^\n'
     '                                       --repeat 10 --csv results.csv\n'
     'if errorlevel 2 echo could not run & exit /b 2\n'
     'if errorlevel 1 echo at least one FAIL & exit /b 1\n'
     'echo all PASS')
note('Process Launcher is a Windows program, not a console one: without ' + mono('start /wait') + ' a batch file goes on '
     'at once and the exit code is lost. In PowerShell use ' + mono('(Start-Process -Wait -PassThru ...).ExitCode') + '.')

# =====================================================================================================
# 9 Batch files
# =====================================================================================================
h1('Writing batch files')
h2('The PASS / FAIL pattern')
para('The batch files of ' + mono('bat_examples') + ' report their own result: they print PASS or FAIL, end with exit code '
     '0 or 1 and append one time stamped line to the result file. Copy ' + mono('00_TEMPLATE.bat') + ' (or use File &gt; '
     'New batch file) and put the test in the marked place:')
code('if not defined PCR_RESULT_FILE set "PCR_RESULT_FILE=%~dp0result.txt"\n'
     'set "RESULT_FILE=%PCR_RESULT_FILE%"\n'
     'set "TEST_NAME=%~n0"\n'
     '\n'
     'ping -n 1 127.0.0.1 >nul\n'
     'if errorlevel 1 (set "MSG=ping returned an error" & goto :fail)\n'
     'set "MSG=ping answered" & goto :pass\n'
     '\n'
     ':pass\n'
     'call :log PASS %MSG%\n'
     'exit /b 0\n'
     ':fail\n'
     'call :log FAIL %MSG%\n'
     'exit /b 1\n'
     ':log\n'
     'set "T=%time: =0%"\n'
     '>>"%RESULT_FILE%" echo %date% %T:~0,8% [%TEST_NAME%] %*\n'
     'echo %*\n'
     'goto :eof')
para('In the row: command ' + mono('bat_examples\\my_test.bat') + ', expected result ' + mono(':File:::[my_test] PASS') +
     ' (or simply ' + mono('PASS') + ' to look in the console output). Started by hand, the batch file writes '
     + mono('result.txt') + ' next to itself.')
h2('Rules')
bullets(['First line ' + mono('@echo off') + '.',
         'Never ' + mono('pause') + ', ' + mono('choice') + ' or ' + mono('set /p') + ': there is no keyboard. To wait, use '
         + mono('ping -n N 127.0.0.1 >nul') + '.',
         'Append with ' + mono('>>') + ', never ' + mono('>') + ': the result file is shared by all commands.',
         'Windows (CRLF) line ends: labels and ' + mono('goto') + ' can fail with bare LF. The editor saves with CRLF.',
         'Inside a ' + mono('( ... )') + ' block ' + mono('%VAR%') + ' is expanded before the block runs: set a variable and '
         'read it in different blocks, or use labels as the examples do.',
         'A command that may hang needs a time limit: the time limit of its row, or one inside the batch file (see '
         + mono('15_timeout_guard.bat') + ').',
         'For ' + mono(':Exit:') + ' end the batch file with ' + mono('exit /b <code>') + '.'])

# =====================================================================================================
# 7 Batch editor
# =====================================================================================================
h1('The batch file editor')
picture('batch-editor.png', PICTURES['batch-editor.png'])
para('File &gt; New batch file (Ctrl+N) and File &gt; Open batch file (Ctrl+O) open the editor. On the left the text, '
     'highlighted a moment after the last key: commands and flow words in bold, comments, labels, variables and strings in '
     'their colours, PASS and FAIL standing out. On the right a button for every batch command: a click inserts it at the '
     'cursor, hovering shows its explanation (tooltip and status bar), a right click shows the details and an example; the '
     'box above the buttons filters them.')
table(['Keys', 'Action'],
      [['Ctrl+N / Ctrl+O', 'New / open'], ['Ctrl+S', 'Save (asks for a name the first time)'],
       ['Ctrl+ + / Ctrl+ -', 'Larger / smaller text (A+ / A-), kept for the next time']],
      [40, 126], 'Keys of the editor')
h2('Encoding of the saved file')
para('cmd.exe reads batch files in the OEM code page of the console (for example 850 in Western Europe). The editor saves '
     'a file in the encoding it was read in: OEM for ordinary batch files, UTF-8 for a UTF-8 file with accented letters '
     '(a byte order mark is kept). New files are OEM. If the text holds a character the OEM code page cannot represent '
     '(such as €), the file is saved in UTF-8 and a message says so: put ' + mono('chcp 65001 >nul') + ' at the top of '
     'such a file.')
note('A batch file with accented letters saved in UTF-8 by an earlier version of the editor stays UTF-8: add '
     + mono('chcp 65001 >nul') + ' to it, or retype the letters and save it as a new OEM file.')
h2('Closing')
para('Closing the editor with unsaved changes asks whether to save them; the same question comes when the main window '
     'closes.')

# =====================================================================================================
# 8 License
# =====================================================================================================
h1('License')
para(f'Running commands, Single mode and the batch editor need a license. The first start on a PC begins a '
     f'<b>{TRIAL_DAYS}-day trial</b> with every feature. Without a license the window, the About window and the License '
     'window stay available.')
table(['Feature key', 'Unlocks'],
      [[mono('run'), 'the Run buttons and Run command(s)'], [mono('sequential'), 'Single commands'],
       [mono('editor'), 'the batch file editor']],
      [40, 126], 'Features')
table(['Edition', 'Features'], [[mono(k), 'all, also future ones' if v == '*' else mono(v)] for k, v in EDITIONS], [40, 126],
      'Editions')
h2('Getting a license')
steps(['Open ' + ui('Info &gt; License') + ' (Ctrl+K).',
       'Click ' + ui('Save Request...') + ' and save ' + mono('license-request.txt') + ', or copy the UID of the PC.',
       f'Send it to <b>{CONTACT}</b> with the licensee name and the edition wanted.',
       'Install the ' + mono('.lic') + ' file received with ' + ui('Load License...') + ', ' + ui('Paste License') + ', or '
       'by dropping it on the License window.'])
bullets(['A license is bound to one PC and to the product ' + mono(PRODUCT_CODE) + '; a license of another product does '
         'not unlock this one.',
         'A license may end on a date and may cover the versions released up to a date ("updates until").',
         'Setting the PC clock back by more than a few days suspends the trial and time-limited licenses until the date is '
         'right again.'])

# =====================================================================================================
# 9 Troubleshooting
# =====================================================================================================
h1('Troubleshooting')
table(['Symptom', 'Cause and remedy'],
      [['"VCRUNTIME140.dll was not found"', 'Install the Visual C++ Redistributable of the same bitness.'],
       ['A command always passes', 'The batch file lacks ' + mono('@echo off') + ', or the PASS text also appears in FAIL '
        'lines, or ' + mono(':File:::PASS') + ' accepts the line of another command: add ' + mono('[name]') + '.'],
       ['A command always fails with ' + mono(':File:...'), 'The line is written to another file than the result file shown: '
        'the batch file must append to ' + mono('%PCR_RESULT_FILE%') + '. The path is wrong, or the text spans two lines.'],
       ['"File ... doesn\'t exist"', 'The path after ' + mono(':File:') + ' is wrong, or the command did not create it.'],
       ['A command stays Busy', 'It waits for something (a dialog, a network). Click its Stop button, or give the row a '
        'time limit; Stop waiting (Ctrl+B) lets Run command(s) go on without it.'],
       ['--run returns at once in a batch file', 'Use ' + mono('start /wait "" "Process Launcher.exe" --run ...') + ' (chapter 8).'],
       ['Exit code 2 from --run', 'No license, the project could not be read or no row is ON: see the log.'],
       ['"...not started: no result file"', 'The question about the result file was cancelled: choose the file (Settings '
        '&gt; Select result file).'],
       ['Accented letters wrong in a batch file', 'It was saved in UTF-8 without ' + mono('chcp 65001') + '; see 10.1.'],
       ['Run buttons do nothing / ask for a license', 'The trial is over: see chapter 11.']],
      [52, 114], 'Troubleshooting')
para('The log of every session is ' + mono('Process Launcher data\\log.txt') + ' (Settings &gt; Open data folder); add it '
     f'to a problem report sent to <b>{CONTACT}</b>.')

# =====================================================================================================
# Appendices
# =====================================================================================================
h1('Appendix A - Keyboard shortcuts')
table(['Keys', 'Command'],
      [['Ctrl+N / Ctrl+O', 'New / open batch file'], ['Ctrl+S', 'Save project'],
       ['Ctrl+E / Ctrl+D', 'Enable / disable edit of the rows'],
       ['Ctrl+B', 'Stop waiting for the Single command / the end of a run'], ['Ctrl+R', 'Stop repeating'],
       ['Ctrl+K', 'License'],
       ['Ctrl+A / Ctrl+C', 'In the result list: select all / copy']],
      [40, 126], 'Keyboard shortcuts')
story.append(PageBreak())
h1('Appendix B - Example batch files')
para('In ' + mono('bat_examples') + ' next to the executable. Each one names its arguments and its expected result in its '
     'first lines. They were written for Windows 10 / 11: try each by hand (double click, then look at the result file) '
     'before relying on it.')
table(['File', 'Checks'], [[mono(n), esc(d)] for n, d in EXAMPLES], [62, 104], 'Example batch files')
story.append(Spacer(1, 10 * mm))
story.append(Paragraph(f'<i>End of document {DOC_ID}, revision {REVISION}. Author: {AUTHOR} - {CONTACT}.</i>',
                       ParagraphStyle('end', parent=S['body'], alignment=TA_CENTER, textColor=GREY)))


def main():
    doc = Manual(OUTPUT)
    doc.multiBuild(story)
    print(OUTPUT)


if __name__ == '__main__':
    main()
