# Licensing - how Process Launcher licenses are issued

Process Launcher uses the same license system as PdfEncryptor, SerialPortManager and Reactor Control,
and its licenses are issued with the same generic generator, **LicGen** (see its `docs\LicGen-Guide-EN.pdf`).
The shared code (`License`, `Ed25519`, `HashUtils`, `TextUtils`, `TimeUtils`, `MachineId`,
`LicenseManager`) is copied unchanged: copy changes across the products.

| | Process Launcher |
|---|---|
| Product code (`kProduct`) | `ProcessLuncher` (spelling as the repository) |
| Features | `run` (Run buttons and "Run command(s)"), `sequential` ("Single" commands), `editor` (batch file editor) |
| Editions | `professional` = everything (`*`, future features too); `standard` = `run` |
| Trial | 14 days, every feature (`LicensePolicy.h`) |
| Without a license | the window, About and License windows; no running, no single mode, no editor |

`include\LicensePolicy.h` is read as text by LicGen: keep its `inline constexpr` form.

## First setup (once) - the build ships with NO key

1. Start LicGen, **Product folder > Browse...** -> this folder. It shows *Product ProcessLuncher -
   features: run, sequential, editor*.
2. **New Signing Key...**: a key ID such as `pcr-2026a` and a passphrase of at least 12 characters. The
   encrypted private key goes to `privateData\license-signing.key` (git-ignored: never commit it), the
   public key into `include\LicenseKeys.h`.
3. **Back up** the key file and the passphrase in two places that are not this PC.
4. Commit `include\LicenseKeys.h`, rebuild: from this build on licenses signed with that key are accepted.
   Until then only the trial works and the License window says *"This build contains no license
   verification key"*.

Use a **new** key for this product: each product has its own key, and a license is also bound to the
product code, so a PdfEncryptor license cannot unlock Process Launcher.

## Issuing a license

1. The user opens **Info > License** (Ctrl-K) and sends `license-request.txt` (**Save Request...**) or the UID.
2. In LicGen (this folder chosen, key unlocked): **Load Request File...** (or **Paste UID**), licensee,
   e-mail, edition, validity, **Generate License...**.
3. Send the `.lic` file; the user installs it with **Load License...**, **Paste License** or by dropping it on
   the License window.

The version date of the top entry of `kAppVersionHistory` (`Version.h`) is compared with the license's
"updates until": add an entry with the release date for every release.

This protects against casual misuse and honest mistakes, not against a determined attacker with a
debugger - no purely local check does. Nobody without the private key can make a license.
