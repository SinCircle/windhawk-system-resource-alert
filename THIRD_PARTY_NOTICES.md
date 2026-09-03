# Third-party notices

## Taskbar-host access

Portions of `system-resource-alert.wh.cpp` are adapted from **Taskbar tray icon spacing and grid** (`taskbar-notification-icon-spacing.wh.cpp`) by **m417z**.

- Author: [m417z](https://github.com/m417z)
- Source: [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-notification-icon-spacing.wh.cpp)
- Upstream development: [m417z/my-windhawk-mods](https://github.com/m417z/my-windhawk-mods)
- Upstream notice: source code is published under the GNU General Public License v3.0.

This project's adaptations include read-only taskbar-host discovery, validation of the supported accessor layout, a dedicated resource-alert UI, and ownership-aware cleanup. The upstream attribution is also retained in the source file. The combined work is distributed under GNU GPL v3.0; see [LICENSE](LICENSE).

## Fluent UI System Icons

The 16 px vector path data used for resource icons is derived from
[Microsoft Fluent UI System Icons](https://github.com/microsoft/fluentui-system-icons).

MIT License

Copyright (c) 2020 Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## External dependencies

The mod is built and loaded by [Windhawk](https://windhawk.net/) and uses Windows / C++/WinRT interfaces. It may call NVIDIA NVML when the system driver provides it, and may read an already-running LibreHardwareMonitor instance.

This repository does not vendor or distribute Windhawk, Windows SDK / runtime components, NVIDIA libraries or drivers, or LibreHardwareMonitor binaries. Each external project remains subject to its own license. References and interface declarations do not imply endorsement by their respective maintainers.

## License text

The unmodified GNU GPL v3.0 license text is included in `LICENSE`. Its canonical source is the [Free Software Foundation](https://www.gnu.org/licenses/gpl-3.0.html).
