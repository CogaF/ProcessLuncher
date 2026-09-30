# Example batch files for Process Launcher

Each file is a **test that reports its own result**: it prints `PASS` or `FAIL`, ends with exit code
0 or 1, and appends a line with the time to the **result file** chosen in Process Launcher
(variable `PCR_RESULT_FILE`; `result.txt` next to the batch file when it is started by hand):

    2026-09-30 21:45:10 [02_ping_host] PASS 127.0.0.1 answers

Copy `00_TEMPLATE.bat`, put your test in the marked place, done. (Or File > New batch file: the editor
opens the same skeleton, with a reference of all the batch commands.)

## How to use one in Process Launcher

1. Command: `bat_examples\02_ping_host.bat 192.168.1.1` (relative paths start from the folder of the exe).
2. Expected result, one of:
   - `:File:::[02_ping_host] PASS` - looks in the **result file**, only at the lines this run added, so a
     PASS from an earlier run is never counted (the name in brackets also keeps parallel commands apart);
   - `PASS` - looks in the console output of the batch file.

## The files

| File | Checks |
|---|---|
| `00_TEMPLATE` | skeleton to copy |
| `01_minimal_pass_fail` | nothing - the smallest file Process Launcher can judge |
| `02_ping_host` | a host answers to ping |
| `03_file_exists` / `04_folder_exists` | a file / a folder exists |
| `05_text_in_file` | a text is in a file (`findstr`) |
| `06_compare_files` | two files are identical (`fc`) |
| `07_exit_code_of_program` | any program ends with exit code 0 |
| `08_service_running` / `09_process_running` | a Windows service / a program is running |
| `10_port_listening` | a TCP port is open |
| `11_http_status` | a web address answers 200 (`curl`) |
| `12_disk_free_space` | a drive has at least N GB free |
| `13_wait_for_file` | a file appears within a time limit |
| `14_retry_command` | a command succeeds within N attempts |
| `15_timeout_guard` | a command ends within a time limit (killed otherwise) |
| `16_registry_value` | a registry value contains a text |
| `17_env_variable` | an environment variable is defined / has a value |
| `18_copy_and_verify` | a copied file is identical to its source |
| `19_steps_stop_at_first_failure` | several checks in one file, stops at the first failure |
| `20_log_with_time_demo` | three ways to append a time stamped line |
| `21_arguments_and_defaults` | reading arguments, defaults, usage error |
| `22_check_admin_rights` / `23_check_windows_version` | elevated prompt / Windows 10 or 11 |
| `24_dns_lookup` | a name resolves |
| `25_no_errors_in_logs` | no `*.log` of a folder contains ERROR |
| `26_file_hash` | SHA-256 of a file |
| `27_file_is_recent` | a file was modified in the last N minutes |
| `28_create_and_delete_file` | create, check, delete, check |
| `29_robocopy_exit_codes` | robocopy (exit codes 0-7 are success) |
| `30_run_another_batch` | runs another batch file and judges its exit code |
| `31_tool_installed` | a tool is in the PATH |
| `32_setting_in_config_file` | `key=value` in a config file |

## Rules that make a batch file work well here

- First line `@echo off`: otherwise each command is printed before it runs and the expected text is found
  inside the echoed command (false PASS).
- Print **one** of PASS / FAIL, keep each word out of the other's line.
- Never `pause`, `choice` or `set /p`: a command gets no keyboard; use `ping -n N 127.0.0.1 >nul` to wait.
- Append with `>>` (at the END of the line), never `>`: the result file is shared by all commands.
- Files must have Windows (CRLF) line ends: labels and `goto` can fail with bare LF. Git does this for
  `*.bat` (see `.gitattributes`); the editor of Process Launcher saves with CRLF.
- Inside a `( ... )` block `%VAR%` is expanded *before* the block runs: set a variable and read it in
  different blocks, or use `goto` labels as these examples do.

The files were written for Windows 10 / 11 (`curl`, `tar` and PowerShell are part of it). They were
written without a Windows machine at hand and are not tested on every edition: try each one by hand
(double click, then look at the result file) before relying on it.
