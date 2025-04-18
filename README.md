The purpose of this SW is to be able to launch different commands in parallel (or sequential).
Analyze the result, seeking a specific text inside the result and if found record it as a "PASS" otherwise as a "FAIL"
It can analyze the result as direct response from the command or search inside a file in case the command prints the result in a file instead of return it.

Each command runs in its own thread so it doesn't freeze the GUI.

depends on wxWidgets GUI library: https://github.com/wxWidgets/wxWidgets
