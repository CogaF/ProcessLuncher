r"""
The owner of Process Launcher and the legal notices printed after the cover of its manuals.

The owner, the copyright years, the contact and the project page are read from include\AppInfo.h
(kCopyright, kContact, kOrganisation, kWebsite), the one place the program keeps them, so the About
window and the manuals always name the same holder - the holder of the license system.
No reportlab here: only the texts.
"""
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def _app_info(name):
    """The string constant kName of include/AppInfo.h."""
    with open(os.path.join(ROOT, 'include', 'AppInfo.h'), encoding='utf-8') as f:
        match = re.search(r'\b' + name + r'\s*=\s*"([^"]*)"', f.read())
    if not match:
        raise SystemExit(f'{name} not found in include/AppInfo.h')
    return match.group(1)


_copyright = re.match(r'Copyright\s*\(C\)\s*([0-9-]+)\s+(.+)', _app_info('kCopyright'))
if not _copyright:
    raise SystemExit('kCopyright of include/AppInfo.h is not "Copyright (C) <years> <owner>"')
YEARS, OWNER = _copyright.group(1), _copyright.group(2).strip()
CONTACT = _app_info('kContact')
ORGANISATION = _app_info('kOrganisation')
WEBSITE = _app_info('kWebsite')
PRODUCT = _app_info('kName')
COPYRIGHT_LINE = f'Copyright © {YEARS} {OWNER}. All rights reserved.'

TRADEMARK_OWNERS = 'Microsoft (Windows, Visual Studio, Visual C++, PowerShell, SmartScreen), GitHub'

TITLE = 'Legal notices'
SECTIONS = [
    ('Copyright',
     f'{COPYRIGHT_LINE} This document - its text, structure, tables and examples - is an original work of {OWNER}, '
     f'author of {PRODUCT} and holder of its license system. It may be used by users of {PRODUCT} for their own use with '
     f'the software. Any other reproduction, distribution, publication, translation or modification, in whole or in part, '
     f'needs the written permission of {OWNER} ({CONTACT}). The licence of the source code of {PRODUCT} (see LICENSE and '
     f'README.md) does not extend to this document.'),
    ('Trademarks',
     f'Company names, product names and brand names mentioned in this document - among them {TRADEMARK_OWNERS} - are '
     f'trademarks or registered trademarks of their respective owners. They are used only to name the software '
     f'{PRODUCT} works with (descriptive use). {PRODUCT} and {OWNER} are not affiliated with, sponsored or endorsed by any '
     f'of these owners.'),
    ('Third-party software',
     f'{PRODUCT} is built with the wxWidgets library, used under the wxWindows Library Licence. The commands of cmd.exe '
     f'and of the Windows tools quoted in the examples are defined by Microsoft and described in its documentation; this '
     f'document only states how {PRODUCT} uses them.'),
    ('Disclaimer',
     f'This document and the example batch files are provided "as is", without warranty of any kind. A command started by '
     f'{PRODUCT} runs with the rights of the user who started {PRODUCT}: check every command and batch file before using '
     f'it on equipment or data that matter.'),
]
