# Third-party notices

## Taskbar-host access

Portions of `system-resource-alert.wh.cpp` are adapted from **Taskbar tray icon spacing and grid** (`taskbar-notification-icon-spacing.wh.cpp`) by **m417z**.

- Author: [m417z](https://github.com/m417z)
- Source: [ramensoftware/windhawk-mods](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-notification-icon-spacing.wh.cpp)
- Upstream development: [m417z/my-windhawk-mods](https://github.com/m417z/my-windhawk-mods)
- Upstream notice: source code is published under the GNU General Public License v3.0.

This project's adaptations include read-only taskbar-host discovery, validation of the supported accessor layout, a dedicated resource-alert UI, and ownership-aware cleanup. The upstream attribution is also retained in the source file. The combined work is distributed under GNU GPL v3.0; see [LICENSE](LICENSE).

## External dependencies

The mod is built and loaded by [Windhawk](https://windhawk.net/) and uses Windows / C++/WinRT interfaces. It may call NVIDIA NVML when the system driver provides it, and may read an already-running LibreHardwareMonitor instance.

This repository does not vendor or distribute Windhawk, Windows SDK / runtime components, NVIDIA libraries or drivers, or LibreHardwareMonitor binaries. Each external project remains subject to its own license. References and interface declarations do not imply endorsement by their respective maintainers.

## License text

The unmodified GNU GPL v3.0 license text is included in `LICENSE`. Its canonical source is the [Free Software Foundation](https://www.gnu.org/licenses/gpl-3.0.html).
