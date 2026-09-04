// ==WindhawkMod==
// @id              system-resource-alert
// @name            System Resource Alert for Taskbar
// @description     Native taskbar resource alerts: two rows per column, expanding left before the system tray.
// @version         0.8.4
// @author          SinCircle
// @github          https://github.com/SinCircle
// @homepage        https://github.com/SinCircle/windhawk-system-resource-alert
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lpsapi -ladvapi32 -lole32 -loleaut32 -lruntimeobject -lcomctl32 -ldxgi -lpdh -lwbemuuid -lwinhttp
// @license         GPL-3.0
// ==/WindhawkMod==
// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 SinCircle. See THIRD_PARTY_NOTICES.md for attribution.
// Taskbar-host access is adapted from m417z's Taskbar Notification Icon Spacing:
// https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-notification-icon-spacing.wh.cpp
// License: https://www.gnu.org/licenses/gpl-3.0.html

// ==WindhawkModReadme==
/*
# System Resource Alert for Taskbar

Shows resources that have remained beyond their alert threshold. When no alert
remains, the component is completely hidden and reserves no taskbar space.
The old showOverviewWhenHealthy setting is no longer read.
Alerts appear in transparent two-row columns
just before the system tray (to the left of the overflow arrow):

- CPU icon + utilization percentage
- Memory icon + utilization percentage
- Commit icon + commit-limit percentage
- System-drive icon + free GB
- Disk-activity icon + active-time percentage
- Disk-write icon + compact throughput
- GPU card icon + utilization percentage (per adapter)
- VRAM icon + dedicated-memory percentage (per adapter)
- CPU/GPU thermometer icon + degrees Celsius

Amber means warning and red means critical. There is no panel background,
border, or status label. Short spikes are ignored, and alerts remain visible
briefly during recovery to avoid flickering.

Each row is one icon plus its value. Exactly three alerts use one compact
three-row column. One or two alerts use one normal column; four or more return
to two rows per column and expand left. The tray-side edge remains anchored.
The compact default uses 18-DIP rows with no extra row gap, including upgrades
with older saved spacing. Enable Use custom row spacing to apply the saved
Row height and Row gap instead. Font and icon sizes are unchanged.
For compact horizontal spacing, use Item width 54, Column gap 4, and
Icon-to-value gap 3. Saved settings from older versions are retained.
This version inserts native XAML controls into SystemTrayFrameGrid, in their
own auto-sized column before the tray. It creates no overlay window and uses
no topmost positioning. Taskbar layout owns the position and allocated space.
The entire column can collapse when no resource is alerting. Native text and
Microsoft Fluent UI System Icons vector paths provide antialiasing without a
bitmap or colored background.

Click any alert to open a rounded native, light-dismiss
flyout with three columns: parameter, current value, and rolling fifteen-minute extremum.
Its tray-side edge follows the alert column horizontally, while its bottom edge
keeps the native flyout gap above the taskbar instead of following alert-row height.
There is no title/timestamp block or threshold/status column. Critical,
warning, and pending rows sort before normal rows; confirmed alerts are colored.
Unavailable/stale readings are omitted from the table, not displayed as zero.
Hover a current value for its status or a parameter for device/source details.
All readable rows are shown at once, without scrolling or pagination. Cells
stay on one line; the wider card grows to its content and splits long tables
into adjacent groups. On unusually small screens it scales down only as needed
to remain inside the work area.
Rows with a resource icon show it before the parameter name. Each physical GPU
has its own collapsible summary row (load, dedicated VRAM, and temperature);
expanding one GPU never mixes its readings or state with another adapter.
CPU and disk active-time warning durations accumulate across brief dips below
their thresholds. System-drive write time above the configured rate also
accumulates; by default, dips below 5 MiB/s of at most five seconds keep the
episode alive and 180 accumulated seconds trigger a warning. Read throughput
remains diagnostic because storage devices differ greatly.
The table refreshes while open. Extrema cover available samples from the most
recent fifteen minutes (or the shorter time since startup) and remain in memory only; free
disk space records its minimum while other metrics record their maximum. No
arrow is appended to minimum values. Hiding an unavailable table row does
not confirm recovery from a previous alert: that alert remains until a valid
reading confirms recovery. The flyout closes when the last alert clears.

GPU collection: NVIDIA NVML when available, with Windows PDH counters and
documented D3DKMT temperature readings as fallbacks for AMD/Intel/NVIDIA.
Adapters are separate; process engine loads are summed per engine, not across
engines. Small UMA dedicated carve-outs below 1 GiB do not trigger VRAM alerts.
Only physical NVIDIA/AMD/Intel adapters are enumerated (up to eight).
GPU defaults: load 95% for 30 seconds; VRAM warning 90%, critical 97%; core
temperature warning 80 C, critical 87 C. Temperature defaults are configurable
alert preferences, not claims of manufacturer safety limits.

CPU temperature is optional: run LibreHardwareMonitor with its local Web Server
enabled (default port 8085); legacy WMI providers are also supported. New LHM
releases no longer publish WMI. The mod only requests 127.0.0.1/data.json,
without a proxy, authentication, or redirects. Do not expose the sensor server
to the internet or allow unnecessary inbound firewall access.
This mod reads existing sensors only; it does not install a kernel
driver or launch LibreHardwareMonitor. CPU defaults are warning 90 C, critical
95 C. No provider means CPU temperature is unavailable, not normal. The CPU
sensor query runs separately so a stalled provider cannot stop other sampling.
GPU hotspot/VRAM junction, motherboard, and disk temperatures are not monitored.

All XAML changes run on the taskbar thread. Resource sampling stays on a
worker thread. Disabling the mod removes its controls and restores the column
assignments it changed. Legacy trayGap/rightShift settings are no longer used.

Windows 11's internal taskbar structures are not a stable extension API.
Unrecognized host layouts or missing symbols cause a logged failure instead
of falling back to an overlay or using a guessed memory offset. Changes to
the same grid by other mods can require additional compatibility work.

Data comes from Windows APIs, NVIDIA NVML, and an optional localhost sensor
endpoint/legacy WMI. The mod doesn't read user files, prompts, terminal contents,
or inspect network traffic. Only the primary horizontal
taskbar is supported in this version.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- itemWidth: 54
  $name: Item width
  $description: Minimum width of each icon-and-number row in logical pixels. Longer values can grow naturally without clipping.
- useCustomRowSpacing: 0
  $name: Use custom row spacing
  $description: Off uses compact 18-pixel rows with no gap, even with old saved settings. Enable to use Row height and Row gap below.
- rowHeight: 18
  $name: Row height
  $description: Used only with custom row spacing enabled. Preferred height in logical pixels, reduced if needed to fit the taskbar.
- rowGap: 0
  $name: Row gap
  $description: Used only with custom row spacing enabled. Vertical gap between rows in logical pixels.
- iconValueGap: 3
  $name: Icon-to-value gap
  $description: Horizontal space between each icon and its number, in logical pixels.
- alertColumnGap: 4
  $name: Column gap
  $description: Gap before the extra column that expands to the left when more than two resources are alerting.
- componentGap: 2
  $name: Component gap
  $description: Gap between the native alert column and the tray, in logical pixels. No absolute screen offsets are used.
- gpuWarningPct: 95
  $name: GPU load warning percentage
- gpuWarningSeconds: 30
  $name: GPU load warning duration
- vramWarningPct: 90
  $name: Dedicated VRAM warning percentage
- vramCriticalPct: 97
  $name: Dedicated VRAM critical percentage
- gpuTemperatureWarning: 80
  $name: GPU core temperature warning (C)
- gpuTemperatureCritical: 87
  $name: GPU core temperature critical (C)
- cpuTemperatureWarning: 90
  $name: CPU temperature warning (C)
- cpuTemperatureCritical: 95
  $name: CPU temperature critical (C)
- enableCpuTemperature: 1
  $name: Read CPU temperature from LibreHardwareMonitor
  $description: Requires LibreHardwareMonitor running with its Web Server enabled. Reads 127.0.0.1 only; legacy WMI is a fallback. This mod installs no driver. Unavailable readings are never treated as zero.
- cpuSensorPort: 8085
  $name: Local CPU sensor Web Server port
  $description: LibreHardwareMonitor's local HTTP port. Requests are read-only, with no proxy or redirects. Do not expose this sensor service to the internet.
- updateIntervalMs: 1000
  $name: Update interval
  $description: Sampling interval in milliseconds. Values below 500 are clamped.
- commitWarningPct: 85
  $name: Commit warning percentage
- commitCriticalPct: 95
  $name: Commit critical percentage
- diskActivityWarningPct: 90
  $name: System-drive activity warning percentage
- diskActivityWarningSeconds: 60
  $name: System-drive activity warning duration
  $description: Accumulated above-threshold time needed for a warning. Brief dips use the High-usage gap setting.
- highUsageGapSeconds: 5
  $name: CPU / disk-activity gap tolerance
  $description: A below-threshold gap up to this many seconds doesn't discard accumulated CPU or disk-activity time.
- diskWriteWarningMiBps: 5
  $name: System-drive write warning rate (MiB/s)
- diskWriteWarningSeconds: 180
  $name: System-drive write accumulated duration
- diskWriteGapSeconds: 5
  $name: System-drive write gap tolerance
  $description: A write-rate dip below the threshold up to this many seconds doesn't discard accumulated write time.
- diskWarningPct: 10
  $name: Disk warning percentage
- diskCriticalPct: 5
  $name: Disk critical percentage
- diskWarningGB: 10
  $name: Disk warning free GB
- diskCriticalGB: 3
  $name: Disk critical free GB
- cpuWarningPct: 95
  $name: CPU warning percentage
- cpuWarningSeconds: 30
  $name: CPU warning duration
  $description: CPU must stay above the threshold for this many seconds.
- warningSustainSeconds: 5
  $name: Warning confirmation duration
- criticalSustainSeconds: 2
  $name: Critical confirmation duration
- recoverySeconds: 10
  $name: Recovery confirmation duration
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <psapi.h>
#include <commctrl.h>
#include <dxgi1_2.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <wbemidl.h>
#include <tlhelp32.h>
#include <winhttp.h>
#include <roapi.h>
#ifndef WH_MOD_ID
#define WH_MOD_ID L"system-resource-alert"
#endif
#include <windhawk_utils.h>

#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.Data.Json.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <memory>
#include <limits>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <deque>
#include <string_view>
#include <set>
#include <cstdio>
#include <cstddef>
#include <cwctype>
#include <charconv>

namespace {

using namespace winrt::Windows::UI::Xaml;
constexpr wchar_t kComponentName[] = L"WindhawkSystemResourceAlertComponent";
constexpr size_t kResourceCount = 12;
constexpr size_t kMaxGpus = 8;
constexpr size_t kMaxAlerts = 8 + 3 * kMaxGpus;
constexpr size_t kMaxColumns = (kMaxAlerts + 1) / 2;

enum class Severity : int {
    Normal = 0,
    Warning = 1,
    Critical = 2,
};

enum class ResourceKind : size_t {
    Cpu = 0,
    Memory = 1,
    Commit = 2,
    Disk = 3,
    Monitor = 4,
    Gpu = 5,
    Vram = 6,
    GpuTemperature = 7,
    CpuTemperature = 8,
    Overview = 9,
    DiskActivity = 10,
    DiskWrite = 11,
};

struct Settings {
    std::atomic<int> itemWidth{54};
    std::atomic<bool> useCustomRowSpacing{false};
    std::atomic<int> rowHeight{18};
    std::atomic<int> rowGap{0};
    std::atomic<int> iconValueGap{3};
    std::atomic<int> alertColumnGap{4};
    std::atomic<int> componentGap{2};
    std::atomic<bool> enableCpuTemperature{true};
    std::atomic<int> cpuSensorPort{8085};
    std::atomic<int> gpuWarningPct{95}, gpuWarningSeconds{30};
    std::atomic<int> vramWarningPct{90}, vramCriticalPct{97};
    std::atomic<int> gpuTemperatureWarning{80}, gpuTemperatureCritical{87};
    std::atomic<int> cpuTemperatureWarning{90}, cpuTemperatureCritical{95};
    std::atomic<int> updateIntervalMs{1000};
    std::atomic<int> commitWarningPct{85};
    std::atomic<int> commitCriticalPct{95};
    std::atomic<int> diskWarningPct{10};
    std::atomic<int> diskCriticalPct{5};
    std::atomic<int> diskWarningGB{10};
    std::atomic<int> diskCriticalGB{3};
    std::atomic<int> diskActivityWarningPct{90};
    std::atomic<int> diskActivityWarningSeconds{60};
    std::atomic<int> highUsageGapSeconds{5};
    std::atomic<int> diskWriteWarningMiBps{5};
    std::atomic<int> diskWriteWarningSeconds{180};
    std::atomic<int> diskWriteGapSeconds{5};
    std::atomic<int> cpuWarningPct{95};
    std::atomic<int> cpuWarningSeconds{30};
    std::atomic<int> warningSustainSeconds{5};
    std::atomic<int> criticalSustainSeconds{2};
    std::atomic<int> recoverySeconds{10};
};

struct GpuSample {
    std::wstring id, name;
    LUID luid{};
    UINT vendor = 0;
    double load = -1, dedicatedUsedGB = -1, dedicatedTotalGB = -1;
    double sharedUsedGB = -1, temperature = -1, powerW = -1, clockMHz = -1;
    bool nvml = false;
    double VramPct() const {
        return dedicatedUsedGB >= 0 && dedicatedTotalGB > 0
            ? std::clamp(100.0 * dedicatedUsedGB / dedicatedTotalGB, 0.0, 100.0) : -1;
    }
};

struct ResourceSample {
    bool valid = false;
    double cpuPct = -1.0;
    double ramUsedPct = 0.0;
    double physicalAvailablePct = 0.0;
    double physicalAvailableMB = 0.0;
    double commitPct = 0.0;
    double diskFreePct = 0.0;
    double diskFreeGB = 0.0;
    double diskActivityPct = -1.0;
    double diskReadBps = -1.0;
    double diskWriteBps = -1.0;
    double physicalTotalGB = 0, commitUsedGB = 0, commitLimitGB = 0, diskTotalGB = 0;
    DWORD processCount = 0, threadCount = 0, handleCount = 0;
    wchar_t diskLetter = L'C';
    double cpuTemperature = -1;
    int cpuTemperatureSource = 0;
    std::vector<GpuSample> gpus;
};

struct AlertTracker {
    Severity displayed = Severity::Normal;
    int warningStreak = 0;
    int criticalStreak = 0;
    int recoveryStreak = 0;
    int warningAccumulatedMs = 0;
    int warningGapMs = 0;
};

struct AlertItem {
    ResourceKind kind;
    Severity severity;
    std::wstring value;
    std::wstring label;
};

struct OverviewRow {
    std::wstring key, label, value, peak, threshold, state;
    Severity severity = Severity::Normal;
    bool available = true;
    bool minimum = false;
    std::optional<ResourceKind> icon;
    std::wstring gpuGroup;
    bool gpuSummary = false;
};
struct RuntimeSnapshot {
    std::vector<AlertItem> alerts;
    std::vector<OverviewRow> overview;
    std::wstring devices;
    std::wstring updated;
    ULONGLONG tick = 0;
};

Settings g_settings;
std::array<AlertTracker, kResourceCount> g_trackers{};
ResourceSample g_lastSample{};
std::atomic<HWND> g_taskbarWindow{nullptr};
std::atomic<bool> g_unloading{false};
std::atomic<bool> g_updatePending{false};
std::atomic<bool> g_resetRequested{false};
std::atomic<unsigned> g_uiCallbacks{0};
HANDLE g_thread = nullptr;
HANDLE g_stopEvent = nullptr;
HANDLE g_wakeEvent = nullptr;
UINT g_updateMessage = 0;
UINT g_dispatchMessage = 0;
std::mutex g_snapshotMutex;
RuntimeSnapshot g_snapshot;
std::map<std::wstring, std::array<AlertTracker, 3>> g_gpuTrackers;
constexpr ULONGLONG kExtremumWindowMs = 15ULL * 60 * 1000;
struct ExtremumSample { ULONGLONG tick; double value; };
struct RollingExtremum {
    bool minimum = false;
    std::deque<ExtremumSample> candidates;

    void Expire(ULONGLONG now) {
        while (!candidates.empty() && now >= candidates.front().tick &&
               now - candidates.front().tick >= kExtremumWindowMs) {
            candidates.pop_front();
        }
    }
    void Add(ULONGLONG now, double value) {
        Expire(now);
        while (!candidates.empty() &&
               (minimum ? candidates.back().value >= value
                        : candidates.back().value <= value)) {
            candidates.pop_back();
        }
        candidates.push_back({now, value});
    }
    std::optional<double> Value(ULONGLONG now) {
        Expire(now);
        if (candidates.empty()) return std::nullopt;
        return candidates.front().value;
    }
};
std::map<std::wstring, RollingExtremum> g_extrema;
HANDLE g_cpuThread = nullptr;
DWORD g_cpuThreadId = 0;
std::atomic<double> g_cpuTemperature{-1};
std::atomic<int> g_cpuTemperatureSource{0};
std::atomic<ULONGLONG> g_cpuTemperatureTick{0}, g_cpuQueryStarted{0};

FILETIME g_previousIdle{};
FILETIME g_previousKernel{};
FILETIME g_previousUser{};
bool g_hasPreviousCpuTimes = false;

AlertTracker& Tracker(ResourceKind kind) {
    return g_trackers[static_cast<size_t>(kind)];
}

uint64_t FileTimeToUInt64(const FILETIME& value) {
    ULARGE_INTEGER integer{};
    integer.LowPart = value.dwLowDateTime;
    integer.HighPart = value.dwHighDateTime;
    return integer.QuadPart;
}

int ClampInt(int value, int minimum, int maximum) {
    return std::max(minimum, std::min(value, maximum));
}

int RequiredSamples(int seconds) {
    const int interval = std::max(500, g_settings.updateIntervalMs.load());
    return std::max(1, static_cast<int>(std::ceil(seconds * 1000.0 / interval)));
}

void ResetTrackers() {
    for (auto& tracker : g_trackers) {
        tracker = {};
    }
    g_gpuTrackers.clear();
}

void LoadSettings() {
    g_settings.itemWidth = ClampInt(Wh_GetIntSetting(L"itemWidth"), 44, 100);
    g_settings.useCustomRowSpacing = Wh_GetIntSetting(L"useCustomRowSpacing") != 0;
    g_settings.rowHeight = ClampInt(Wh_GetIntSetting(L"rowHeight"), 18, 32);
    g_settings.rowGap = ClampInt(Wh_GetIntSetting(L"rowGap"), 0, 16);
    g_settings.iconValueGap = ClampInt(Wh_GetIntSetting(L"iconValueGap"), 0, 16);
    g_settings.alertColumnGap = ClampInt(Wh_GetIntSetting(L"alertColumnGap"), 0, 32);
    g_settings.componentGap = ClampInt(Wh_GetIntSetting(L"componentGap"), 0, 32);
    g_settings.enableCpuTemperature = Wh_GetIntSetting(L"enableCpuTemperature") != 0;
    g_settings.cpuSensorPort = ClampInt(Wh_GetIntSetting(L"cpuSensorPort"), 1, 65535);
    g_settings.gpuWarningPct = ClampInt(Wh_GetIntSetting(L"gpuWarningPct"), 50, 100);
    g_settings.gpuWarningSeconds = ClampInt(Wh_GetIntSetting(L"gpuWarningSeconds"), 1, 600);
    g_settings.vramWarningPct = ClampInt(Wh_GetIntSetting(L"vramWarningPct"), 50, 99);
    g_settings.vramCriticalPct = ClampInt(Wh_GetIntSetting(L"vramCriticalPct"), g_settings.vramWarningPct + 1, 100);
    g_settings.gpuTemperatureWarning = ClampInt(Wh_GetIntSetting(L"gpuTemperatureWarning"), 40, 110);
    g_settings.gpuTemperatureCritical = ClampInt(Wh_GetIntSetting(L"gpuTemperatureCritical"), g_settings.gpuTemperatureWarning + 1, 120);
    g_settings.cpuTemperatureWarning = ClampInt(Wh_GetIntSetting(L"cpuTemperatureWarning"), 40, 110);
    g_settings.cpuTemperatureCritical = ClampInt(Wh_GetIntSetting(L"cpuTemperatureCritical"), g_settings.cpuTemperatureWarning + 1, 120);
    g_settings.updateIntervalMs =
        ClampInt(Wh_GetIntSetting(L"updateIntervalMs"), 500, 10000);
    g_settings.commitWarningPct =
        ClampInt(Wh_GetIntSetting(L"commitWarningPct"), 50, 99);
    g_settings.commitCriticalPct =
        ClampInt(Wh_GetIntSetting(L"commitCriticalPct"),
                 g_settings.commitWarningPct.load() + 1, 100);
    g_settings.diskWarningPct =
        ClampInt(Wh_GetIntSetting(L"diskWarningPct"), 1, 50);
    g_settings.diskCriticalPct =
        ClampInt(Wh_GetIntSetting(L"diskCriticalPct"), 1,
                 g_settings.diskWarningPct.load());
    g_settings.diskWarningGB =
        ClampInt(Wh_GetIntSetting(L"diskWarningGB"), 1, 1000);
    g_settings.diskCriticalGB =
        ClampInt(Wh_GetIntSetting(L"diskCriticalGB"), 1,
                 g_settings.diskWarningGB.load());
    g_settings.diskActivityWarningPct =
        ClampInt(Wh_GetIntSetting(L"diskActivityWarningPct"), 50, 100);
    g_settings.diskActivityWarningSeconds =
        ClampInt(Wh_GetIntSetting(L"diskActivityWarningSeconds"), 1, 600);
    g_settings.highUsageGapSeconds =
        ClampInt(Wh_GetIntSetting(L"highUsageGapSeconds"), 0, 60);
    g_settings.diskWriteWarningMiBps =
        ClampInt(Wh_GetIntSetting(L"diskWriteWarningMiBps"), 1, 102400);
    g_settings.diskWriteWarningSeconds =
        ClampInt(Wh_GetIntSetting(L"diskWriteWarningSeconds"), 1, 3600);
    g_settings.diskWriteGapSeconds =
        ClampInt(Wh_GetIntSetting(L"diskWriteGapSeconds"), 0, 300);
    g_settings.cpuWarningPct =
        ClampInt(Wh_GetIntSetting(L"cpuWarningPct"), 50, 100);
    g_settings.cpuWarningSeconds =
        ClampInt(Wh_GetIntSetting(L"cpuWarningSeconds"), 1, 600);
    g_settings.warningSustainSeconds =
        ClampInt(Wh_GetIntSetting(L"warningSustainSeconds"), 1, 120);
    g_settings.criticalSustainSeconds =
        ClampInt(Wh_GetIntSetting(L"criticalSustainSeconds"), 1, 60);
    g_settings.recoverySeconds =
        ClampInt(Wh_GetIntSetting(L"recoverySeconds"), 1, 300);
}

bool QueryLightTheme() {
    DWORD value = 1;
    DWORD valueSize = sizeof(value);
    const LSTATUS result = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &valueSize);
    return result != ERROR_SUCCESS || value != 0;
}

double QueryCpuUsage() {
    FILETIME idle{};
    FILETIME kernel{};
    FILETIME user{};
    if (!GetSystemTimes(&idle, &kernel, &user)) {
        return -1.0;
    }

    if (!g_hasPreviousCpuTimes) {
        g_previousIdle = idle;
        g_previousKernel = kernel;
        g_previousUser = user;
        g_hasPreviousCpuTimes = true;
        return -1.0;
    }

    const uint64_t idleDelta =
        FileTimeToUInt64(idle) - FileTimeToUInt64(g_previousIdle);
    const uint64_t kernelDelta =
        FileTimeToUInt64(kernel) - FileTimeToUInt64(g_previousKernel);
    const uint64_t userDelta =
        FileTimeToUInt64(user) - FileTimeToUInt64(g_previousUser);

    g_previousIdle = idle;
    g_previousKernel = kernel;
    g_previousUser = user;

    const uint64_t total = kernelDelta + userDelta;
    if (total == 0 || idleDelta > total) {
        return -1.0;
    }

    return std::clamp(100.0 * (total - idleDelta) / total, 0.0, 100.0);
}

bool QueryResourceSample(ResourceSample* sample) {
    if (!sample) {
        return false;
    }

    ResourceSample current{};
    current.cpuPct = QueryCpuUsage();

    PERFORMANCE_INFORMATION performance{};
    performance.cb = sizeof(performance);
    if (!GetPerformanceInfo(&performance, sizeof(performance)) ||
        performance.PhysicalTotal == 0 || performance.CommitLimit == 0) {
        return false;
    }

    current.physicalAvailablePct =
        100.0 * performance.PhysicalAvailable / performance.PhysicalTotal;
    current.ramUsedPct = 100.0 - current.physicalAvailablePct;
    current.physicalAvailableMB =
        performance.PhysicalAvailable * performance.PageSize / 1024.0 / 1024.0;
    current.commitPct =
        100.0 * performance.CommitTotal / performance.CommitLimit;
    const double pageGB = performance.PageSize / (1024.0 * 1024 * 1024);
    current.physicalTotalGB = performance.PhysicalTotal * pageGB;
    current.commitUsedGB = performance.CommitTotal * pageGB;
    current.commitLimitGB = performance.CommitLimit * pageGB;
    current.processCount = performance.ProcessCount;
    current.threadCount = performance.ThreadCount;
    current.handleCount = performance.HandleCount;

    wchar_t windowsDirectory[MAX_PATH]{};
    if (!GetWindowsDirectoryW(windowsDirectory, ARRAYSIZE(windowsDirectory)) ||
        windowsDirectory[0] == L'\0') {
        return false;
    }

    wchar_t rootPath[] = L"C:\\";
    rootPath[0] = windowsDirectory[0];
    current.diskLetter = windowsDirectory[0];
    ULARGE_INTEGER freeBytesAvailable{};
    ULARGE_INTEGER totalBytes{};
    ULARGE_INTEGER totalFreeBytes{};
    if (!GetDiskFreeSpaceExW(rootPath, &freeBytesAvailable, &totalBytes,
                             &totalFreeBytes) ||
        totalBytes.QuadPart == 0) {
        return false;
    }

    current.diskFreePct =
        100.0 * totalFreeBytes.QuadPart / totalBytes.QuadPart;
    current.diskFreeGB =
        totalFreeBytes.QuadPart / 1024.0 / 1024.0 / 1024.0;
    current.diskTotalGB = totalBytes.QuadPart / (1024.0 * 1024 * 1024);
    current.valid = true;
    *sample = current;
    return true;
}

struct DiskIoCollector {
    PDH_HQUERY query = nullptr;
    PDH_HCOUNTER activity = nullptr, read = nullptr, write = nullptr;
    wchar_t drive = L'\0';

    ~DiskIoCollector() { Reset(); }

    void Reset() {
        if (query) PdhCloseQuery(query);
        query = nullptr;
        activity = read = write = nullptr;
        drive = L'\0';
    }

    bool Ensure(wchar_t requestedDrive) {
        requestedDrive = static_cast<wchar_t>(std::towupper(requestedDrive));
        if (query && drive == requestedDrive) return true;
        Reset();
        if (requestedDrive < L'A' || requestedDrive > L'Z' ||
            PdhOpenQueryW(nullptr, 0, &query) != ERROR_SUCCESS) {
            Reset();
            return false;
        }
        wchar_t path[96]{};
        const auto add = [&](const wchar_t* metric, PDH_HCOUNTER* counter) {
            swprintf_s(path, L"\\LogicalDisk(%c:)\\%s", requestedDrive, metric);
            return PdhAddEnglishCounterW(query, path, 0, counter) == ERROR_SUCCESS;
        };
        if (!add(L"% Disk Time", &activity) ||
            !add(L"Disk Read Bytes/sec", &read) ||
            !add(L"Disk Write Bytes/sec", &write) ||
            PdhCollectQueryData(query) != ERROR_SUCCESS) {
            Reset();
            return false;
        }
        drive = requestedDrive;
        return false; // The first collection only primes rate counters.
    }

    static double Value(PDH_HCOUNTER counter) {
        if (!counter) return -1;
        PDH_FMT_COUNTERVALUE value{};
        if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                        nullptr, &value) != ERROR_SUCCESS ||
            (value.CStatus != PDH_CSTATUS_VALID_DATA &&
             value.CStatus != PDH_CSTATUS_NEW_DATA) ||
            !std::isfinite(value.doubleValue) || value.doubleValue < 0) {
            return -1;
        }
        return value.doubleValue;
    }

    void Sample(ResourceSample* sample) {
        if (!sample || !sample->valid || !Ensure(sample->diskLetter)) return;
        if (PdhCollectQueryData(query) != ERROR_SUCCESS) {
            Reset();
            return;
        }
        const double active = Value(activity);
        sample->diskActivityPct = active < 0 ? -1 : std::clamp(active, 0.0, 100.0);
        sample->diskReadBps = Value(read);
        sample->diskWriteBps = Value(write);
    }
};

// Read-only GPU APIs. These small x64 ABI declarations mirror the Windows SDK
// d3dkmthk.h structures; Windhawk's bundled MinGW headers omit that header.
struct KmtOpen { LUID luid; UINT handle; };
struct KmtQuery { UINT handle, type; void* data; UINT size; };
struct KmtAddress { UINT bus, device, function; };
struct KmtPerformance {
    UINT physicalIndex;
    alignas(8) uint64_t memoryFrequency, maxMemoryFrequency, maxMemoryFrequencyOC;
    uint64_t memoryBandwidth, pcieBandwidth;
    ULONG fanRpm, powerTenthsPercent, temperatureTenthsC;
    BYTE powered;
};
static_assert(sizeof(KmtOpen) == 12 && sizeof(KmtQuery) == 24);
static_assert(sizeof(KmtPerformance) == 64 && offsetof(KmtPerformance, temperatureTenthsC) == 56);
struct NvMemory { uint64_t total, free, used; };
struct NvUtilization { unsigned gpu, memory; };

std::wstring LuidKey(LUID luid) {
    wchar_t text[64];
    swprintf_s(text, L"luid_0x%08x_0x%08x", static_cast<UINT>(luid.HighPart), luid.LowPart);
    return text;
}

struct GpuCollector {
    using OpenFn = LONG(WINAPI*)(KmtOpen*);
    using CloseFn = LONG(WINAPI*)(UINT*);
    using QueryFn = LONG(WINAPI*)(KmtQuery*);
    HMODULE gdi = nullptr, nvml = nullptr;
    OpenFn open = nullptr;
    CloseFn close = nullptr;
    QueryFn query = nullptr;
    int (*nvInit)() = nullptr;
    int (*nvShutdown)() = nullptr;
    int (*nvHandle)(const char*, void**) = nullptr;
    int (*nvMemory)(void*, NvMemory*) = nullptr;
    int (*nvUtil)(void*, NvUtilization*) = nullptr;
    int (*nvTemp)(void*, unsigned, unsigned*) = nullptr;
    int (*nvPower)(void*, unsigned*) = nullptr;
    int (*nvClock)(void*, unsigned, unsigned*) = nullptr;
    bool nvReady = false;
    PDH_HQUERY pdh = nullptr;
    PDH_HCOUNTER engines = nullptr, dedicated = nullptr, shared = nullptr;
    winrt::com_ptr<IDXGIFactory1> factory;
    std::vector<GpuSample> adapters;
    ULONGLONG enumeratedAt = 0;

    GpuCollector() {
        gdi = LoadLibraryExW(L"gdi32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (gdi) {
            open = reinterpret_cast<OpenFn>(GetProcAddress(gdi, "D3DKMTOpenAdapterFromLuid"));
            close = reinterpret_cast<CloseFn>(GetProcAddress(gdi, "D3DKMTCloseAdapter"));
            query = reinterpret_cast<QueryFn>(GetProcAddress(gdi, "D3DKMTQueryAdapterInfo"));
        }
        nvml = LoadLibraryExW(L"nvml.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (nvml) {
#define NV_READ(field, symbol) field = reinterpret_cast<decltype(field)>(GetProcAddress(nvml, symbol))
            NV_READ(nvInit, "nvmlInit_v2");
            NV_READ(nvShutdown, "nvmlShutdown");
            NV_READ(nvHandle, "nvmlDeviceGetHandleByPciBusId_v2");
            NV_READ(nvMemory, "nvmlDeviceGetMemoryInfo");
            NV_READ(nvUtil, "nvmlDeviceGetUtilizationRates");
            NV_READ(nvTemp, "nvmlDeviceGetTemperature");
            NV_READ(nvPower, "nvmlDeviceGetPowerUsage");
            NV_READ(nvClock, "nvmlDeviceGetClockInfo");
#undef NV_READ
            nvReady = nvInit && nvShutdown && nvInit() == 0;
        }
        if (PdhOpenQueryW(nullptr, 0, &pdh) == ERROR_SUCCESS) {
            PdhAddEnglishCounterW(pdh, L"\\GPU Engine(*)\\Utilization Percentage", 0, &engines);
            PdhAddEnglishCounterW(pdh, L"\\GPU Adapter Memory(*)\\Dedicated Usage", 0, &dedicated);
            PdhAddEnglishCounterW(pdh, L"\\GPU Adapter Memory(*)\\Shared Usage", 0, &shared);
            PdhCollectQueryData(pdh); // Rate counters need a previous sample.
        }
    }
    ~GpuCollector() {
        if (pdh) PdhCloseQuery(pdh);
        if (nvReady) nvShutdown();
        if (nvml) FreeLibrary(nvml);
        if (gdi) FreeLibrary(gdi);
    }

    static std::vector<std::pair<std::wstring, double>> Values(PDH_HCOUNTER counter) {
        std::vector<std::pair<std::wstring, double>> result;
        if (!counter) return result;
        DWORD bytes = 0, count = 0;
        constexpr DWORD format = PDH_FMT_DOUBLE | PDH_FMT_NOCAP100;
        if (PdhGetFormattedCounterArrayW(counter, format, &bytes, &count, nullptr) != static_cast<PDH_STATUS>(PDH_MORE_DATA))
            return result;
        // The engine list can grow between the size query and the data query.
        for (int retry = 0; retry < 3 && bytes <= 8 * 1024 * 1024; ++retry) {
            std::vector<uint64_t> buffer((bytes + 7) / 8);
            auto data = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buffer.data());
            const auto status = PdhGetFormattedCounterArrayW(counter, format, &bytes, &count, data);
            if (status == static_cast<PDH_STATUS>(PDH_MORE_DATA)) continue;
            if (status != ERROR_SUCCESS) return result;
            for (DWORD i = 0; i < count; ++i) {
                const auto& value = data[i].FmtValue;
                if ((value.CStatus == PDH_CSTATUS_VALID_DATA || value.CStatus == PDH_CSTATUS_NEW_DATA) &&
                    std::isfinite(value.doubleValue) && value.doubleValue >= 0 && data[i].szName)
                {
                    // PDH preserves uppercase hex in LUIDs; DXGI keys use lowercase.
                    std::wstring instance = data[i].szName;
                    std::transform(instance.begin(), instance.end(), instance.begin(),
                        [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
                    result.emplace_back(std::move(instance), value.doubleValue);
                }
            }
            break;
        }
        return result;
    }

    void Enumerate() {
        if (factory && factory->IsCurrent() && GetTickCount64() - enumeratedAt < 30000) return;
        factory = nullptr;
        adapters.clear();
        enumeratedAt = GetTickCount64();
        if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), factory.put_void()))) return;
        for (UINT i = 0; i < 32 && adapters.size() < kMaxGpus; ++i) {
            winrt::com_ptr<IDXGIAdapter1> adapter;
            if (factory->EnumAdapters1(i, adapter.put()) == DXGI_ERROR_NOT_FOUND) break;
            if (!adapter) continue;
            DXGI_ADAPTER_DESC1 desc{};
            if (FAILED(adapter->GetDesc1(&desc)) || (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) continue;
            // Exclude virtual/remote display drivers, which are not physical GPUs.
            if (desc.VendorId != 0x10DE && desc.VendorId != 0x1002 && desc.VendorId != 0x8086) continue;
            const auto id = LuidKey(desc.AdapterLuid);
            if (std::any_of(adapters.begin(), adapters.end(), [&](const auto& known) { return known.id == id; })) continue;
            if (open && close && query) {
                KmtOpen opened{desc.AdapterLuid, 0};
                if (open(&opened) >= 0) {
                    UINT type = 0;
                    KmtQuery typeQuery{opened.handle, 15, &type, sizeof(type)};
                    const bool excluded = query(&typeQuery) >= 0 && (type & ((1u << 2) | (1u << 6) | (1u << 7)));
                    close(&opened.handle);
                    if (excluded) continue; // Software, indirect display, or paravirtualized device.
                }
            }
            GpuSample gpu;
            gpu.id = id;
            gpu.name = desc.Description;
            gpu.luid = desc.AdapterLuid;
            gpu.vendor = desc.VendorId;
            gpu.dedicatedTotalGB = desc.DedicatedVideoMemory / (1024.0 * 1024 * 1024);
            adapters.push_back(std::move(gpu));
        }
    }

    std::vector<GpuSample> Sample() {
        Enumerate();
        auto result = adapters; // Start every metric unavailable, never reuse stale samples.
        const bool pdhValid = pdh && PdhCollectQueryData(pdh) == ERROR_SUCCESS;
        const auto engineValues = pdhValid ? Values(engines) : decltype(Values(engines)){};
        const auto dedicatedValues = pdhValid ? Values(dedicated) : decltype(Values(dedicated)){};
        const auto sharedValues = pdhValid ? Values(shared) : decltype(Values(shared)){};
        for (auto& gpu : result) {
            std::map<std::wstring, double> engineTotals;
            for (const auto& [instance, value] : engineValues) {
                const auto at = instance.find(gpu.id + L"_");
                if (at == std::wstring::npos) continue;
                const auto end = instance.find(L"_engtype_", at);
                if (end == std::wstring::npos) continue;
                engineTotals[instance.substr(at, end - at)] += value;
            }
            // Sum processes on each engine, then take the busiest engine.
            // Summing independent 3D/copy/video engines would falsely exceed 100%.
            for (const auto& [engine, value] : engineTotals)
                gpu.load = std::max(gpu.load, std::clamp(value, 0.0, 100.0));
            const auto sumMemory = [&](const auto& values) {
                double total = -1;
                for (const auto& [instance, value] : values)
                    if (instance.starts_with(gpu.id + L"_")) total = std::max(0.0, total) + value;
                return total < 0 ? -1 : total / (1024.0 * 1024 * 1024);
            };
            gpu.dedicatedUsedGB = sumMemory(dedicatedValues);
            gpu.sharedUsedGB = sumMemory(sharedValues);
            void* nvDevice = nullptr;
            if (open && close && query) {
                KmtOpen opened{gpu.luid, 0};
                if (open(&opened) >= 0) {
                    KmtPerformance perf{};
                    KmtQuery perfQuery{opened.handle, 62, &perf, sizeof(perf)};
                    if (query(&perfQuery) >= 0 && perf.temperatureTenthsC > 0 && perf.temperatureTenthsC < 1500)
                        gpu.temperature = perf.temperatureTenthsC / 10.0;
                    KmtAddress address{};
                    KmtQuery addressQuery{opened.handle, 6, &address, sizeof(address)};
                    if (nvReady && nvHandle && gpu.vendor == 0x10DE && query(&addressQuery) >= 0) {
                        char pci[32];
                        sprintf_s(pci, "0000:%02x:%02x.%x", address.bus, address.device, address.function);
                        if (nvHandle(pci, &nvDevice) != 0) nvDevice = nullptr;
                    }
                    close(&opened.handle);
                }
            }
            if (nvDevice) {
                gpu.nvml = true;
                NvUtilization use{};
                if (nvUtil && nvUtil(nvDevice, &use) == 0 && use.gpu <= 100) gpu.load = use.gpu;
                NvMemory memory{};
                // Reject NVML's unavailable sentinel (UINT64_MAX), common in WDDM APIs.
                if (nvMemory && nvMemory(nvDevice, &memory) == 0 && memory.total > 0 &&
                    memory.total < (1ULL << 50) && memory.used <= memory.total) {
                    gpu.dedicatedUsedGB = memory.used / (1024.0 * 1024 * 1024);
                    gpu.dedicatedTotalGB = memory.total / (1024.0 * 1024 * 1024);
                }
                unsigned value = 0;
                if (nvTemp && nvTemp(nvDevice, 0, &value) == 0 && value > 0 && value < 150)
                    gpu.temperature = value;
                if (nvPower && nvPower(nvDevice, &value) == 0 && value < 2000000) gpu.powerW = value / 1000.0;
                if (nvClock && nvClock(nvDevice, 0, &value) == 0 && value < 20000) gpu.clockMHz = value;
            }
        }
        return result;
    }
};

bool IsLibreHardwareMonitorRunning() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool found = false;
    if (Process32FirstW(snapshot, &entry)) do {
        if (_wcsicmp(entry.szExeFile, L"LibreHardwareMonitor.exe") == 0) { found = true; break; }
    } while (Process32NextW(snapshot, &entry));
    CloseHandle(snapshot);
    return found;
}

bool IsCpuTemperatureSensor(std::wstring_view id, std::wstring_view name) {
    return (id.starts_with(L"/amdcpu/") || id.starts_with(L"/intelcpu/")) &&
        name.find(L"Distance") == std::wstring_view::npos &&
        name.find(L"distance") == std::wstring_view::npos &&
        name.find(L"TjMax") == std::wstring_view::npos;
}

double ParseTemperatureText(std::wstring text) {
    const auto unit = text.find(L'°');
    if (unit == std::wstring::npos || unit + 1 >= text.size()) return -1;
    const wchar_t scale = text[unit + 1];
    if (scale != L'C' && scale != L'F') return -1;
    text.resize(unit);
    while (!text.empty() && std::iswspace(text.back())) text.pop_back();
    size_t start = 0;
    while (start < text.size() && std::iswspace(text[start])) ++start;
    std::string numeric;
    for (size_t i = start; i < text.size(); ++i) {
        const wchar_t c = text[i] == L',' ? L'.' : text[i];
        if (c > 127) return -1;
        numeric.push_back(static_cast<char>(c));
    }
    double value = -1;
    const auto parsed = std::from_chars(numeric.data(), numeric.data() + numeric.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != numeric.data() + numeric.size()) return -1;
    if (scale == L'F') value = (value - 32.0) * 5.0 / 9.0;
    return std::isfinite(value) && value > 0 && value < 150 ? value : -1;
}

double FindCpuTemperatureJson(const winrt::Windows::Data::Json::JsonObject& node,
                              unsigned* visited, unsigned depth = 0) {
    using namespace winrt::Windows::Data::Json;
    if (++*visited > 4096 || depth > 20) throw winrt::hresult_invalid_argument();
    double hottest = -1;
    const auto id = node.GetNamedString(L"SensorId", L"");
    const auto name = node.GetNamedString(L"Text", L"");
    if (node.GetNamedString(L"Type", L"") == L"Temperature" &&
        IsCpuTemperatureSensor(id, name)) {
        const auto value = node.GetNamedValue(node.HasKey(L"RawValue") ? L"RawValue" : L"Value", nullptr);
        if (value && value.ValueType() == JsonValueType::String)
            hottest = ParseTemperatureText(std::wstring(value.GetString()));
        else if (value && value.ValueType() == JsonValueType::Number) {
            const double number = value.GetNumber();
            if (std::isfinite(number) && number > 0 && number < 150) hottest = number;
        }
    }
    if (node.HasKey(L"Children")) {
        const auto children = node.GetNamedArray(L"Children");
        for (const auto& child : children)
            if (child.ValueType() == JsonValueType::Object)
                hottest = std::max(hottest, FindCpuTemperatureJson(child.GetObject(), visited, depth + 1));
    }
    return hottest;
}

struct HttpHandle {
    HINTERNET handle = nullptr;
    explicit HttpHandle(HINTERNET value) : handle(value) {}
    ~HttpHandle() { if (handle) WinHttpCloseHandle(handle); }
    HttpHandle(const HttpHandle&) = delete;
    HttpHandle& operator=(const HttpHandle&) = delete;
};

double ReadCpuTemperatureHttp() {
    HttpHandle session(WinHttpOpen(L"WindhawkResourceAlert/0.5", WINHTTP_ACCESS_TYPE_NO_PROXY,
                                   WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (!session.handle) return -1;
    WinHttpSetTimeouts(session.handle, 750, 750, 750, 750);
    HttpHandle connection(WinHttpConnect(session.handle, L"127.0.0.1",
        static_cast<INTERNET_PORT>(g_settings.cpuSensorPort.load()), 0));
    if (!connection.handle) return -1;
    HttpHandle request(WinHttpOpenRequest(connection.handle, L"GET", L"/data.json", nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0));
    if (!request.handle) return -1;
    DWORD disabled = WINHTTP_DISABLE_REDIRECTS | WINHTTP_DISABLE_COOKIES | WINHTTP_DISABLE_AUTHENTICATION;
    if (!WinHttpSetOption(request.handle, WINHTTP_OPTION_DISABLE_FEATURE, &disabled, sizeof(disabled))) return -1;
    if (!WinHttpSendRequest(request.handle, L"Accept-Encoding: identity\r\n", static_cast<DWORD>(-1),
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0) || !WinHttpReceiveResponse(request.handle, nullptr)) return -1;
    DWORD status = 0, size = sizeof(status);
    if (!WinHttpQueryHeaders(request.handle, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX) || status != 200) return -1;
    std::string body;
    const ULONGLONG deadline = GetTickCount64() + 1500;
    for (;;) {
        if (g_unloading || GetTickCount64() >= deadline) return -1;
        char buffer[8192];
        DWORD received = 0;
        if (!WinHttpReadData(request.handle, buffer, sizeof(buffer), &received)) return -1;
        if (!received) break;
        if (body.size() + received > 2 * 1024 * 1024) return -1;
        body.append(buffer, received);
    }
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, body.data(), static_cast<int>(body.size()), nullptr, 0);
    if (!length) return -1;
    std::wstring json(length, L'\0');
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, body.data(), static_cast<int>(body.size()), json.data(), length)) return -1;
    winrt::Windows::Data::Json::JsonObject root{nullptr};
    if (!winrt::Windows::Data::Json::JsonObject::TryParse(json, root)) return -1;
    unsigned visited = 0;
    return FindCpuTemperatureJson(root, &visited);
}

double ReadCpuTemperatureWmi() {
    winrt::com_ptr<IWbemLocator> locator;
    if (FAILED(CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                                IID_IWbemLocator, locator.put_void()))) return -1;
    winrt::com_ptr<IWbemServices> service;
    BSTR space = SysAllocString(L"ROOT\\LibreHardwareMonitor");
    if (!space) return -1;
    const HRESULT connected = locator->ConnectServer(space, nullptr, nullptr, nullptr,
        WBEM_FLAG_CONNECT_USE_MAX_WAIT, nullptr, nullptr, service.put());
    SysFreeString(space);
    if (FAILED(connected)) return -1;
    if (FAILED(CoSetProxyBlanket(service.get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
            nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE))) return -1;
    BSTR language = SysAllocString(L"WQL");
    BSTR queryText = SysAllocString(L"SELECT Identifier, Name, Value FROM Sensor WHERE SensorType = 'Temperature'");
    winrt::com_ptr<IEnumWbemClassObject> results;
    HRESULT queried = E_OUTOFMEMORY;
    if (language && queryText) queried = service->ExecQuery(language, queryText,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, results.put());
    SysFreeString(language);
    SysFreeString(queryText);
    if (FAILED(queried)) return -1;
    double hottest = -1;
    bool complete = false;
    const ULONGLONG deadline = GetTickCount64() + 1000;
    for (int i = 0; i < 256 && !g_unloading && GetTickCount64() < deadline; ++i) {
        winrt::com_ptr<IWbemClassObject> object;
        ULONG returned = 0;
        const HRESULT status = results->Next(100, 1, object.put(), &returned);
        if (status == WBEM_S_FALSE) { complete = true; break; }
        if (FAILED(status) || !returned) return -1;
        VARIANT id{}, name{}, value{}, number{};
        object->Get(L"Identifier", 0, &id, nullptr, nullptr);
        object->Get(L"Name", 0, &name, nullptr, nullptr);
        object->Get(L"Value", 0, &value, nullptr, nullptr);
        if (id.vt == VT_BSTR && name.vt == VT_BSTR && id.bstrVal && name.bstrVal &&
            IsCpuTemperatureSensor(id.bstrVal, name.bstrVal) &&
            value.vt != VT_EMPTY && value.vt != VT_NULL &&
            SUCCEEDED(VariantChangeType(&number, &value, 0, VT_R8)) &&
            std::isfinite(number.dblVal) && number.dblVal > 0 && number.dblVal < 150)
            hottest = std::max(hottest, number.dblVal);
        VariantClear(&id); VariantClear(&name); VariantClear(&value); VariantClear(&number);
    }
    return complete ? hottest : -1;
}

DWORD WINAPI CpuTemperatureThread(void*) {
    const HRESULT initialized = RoInitialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialized)) return 0;
    CoEnableCallCancellation(nullptr);
    while (!g_unloading) {
        double value = -1;
        int source = 0;
        try {
            if (g_settings.enableCpuTemperature && IsLibreHardwareMonitorRunning()) {
                value = ReadCpuTemperatureHttp();
                if (value >= 0) source = 1;
                else {
                    g_cpuQueryStarted = GetTickCount64();
                    value = ReadCpuTemperatureWmi();
                    if (value >= 0) source = 2;
                }
            }
        } catch (...) { value = -1; }
        g_cpuQueryStarted = 0;
        g_cpuTemperature = value;
        g_cpuTemperatureSource = source;
        g_cpuTemperatureTick = GetTickCount64();
        if (WaitForSingleObject(g_stopEvent, 2000) == WAIT_OBJECT_0) break;
    }
    CoDisableCallCancellation(nullptr);
    RoUninitialize();
    return 0;
}

Severity CpuSeverity(const ResourceSample& sample) {
    return sample.cpuPct >= g_settings.cpuWarningPct.load()
               ? Severity::Warning
               : Severity::Normal;
}

Severity MemorySeverity(const ResourceSample& sample) {
    if (sample.physicalAvailablePct < 1.0 &&
        sample.physicalAvailableMB < 500.0) {
        return Severity::Critical;
    }
    if (sample.physicalAvailablePct < 10.0 &&
        sample.physicalAvailableMB < 4096.0) {
        return Severity::Warning;
    }
    return Severity::Normal;
}

Severity CommitSeverity(const ResourceSample& sample) {
    if (sample.commitPct >= g_settings.commitCriticalPct.load()) {
        return Severity::Critical;
    }
    if (sample.commitPct >= g_settings.commitWarningPct.load()) {
        return Severity::Warning;
    }
    return Severity::Normal;
}

Severity DiskSeverity(const ResourceSample& sample) {
    if (sample.diskFreePct < g_settings.diskCriticalPct.load() &&
        sample.diskFreeGB < g_settings.diskCriticalGB.load()) {
        return Severity::Critical;
    }
    if (sample.diskFreePct < g_settings.diskWarningPct.load() &&
        sample.diskFreeGB < g_settings.diskWarningGB.load()) {
        return Severity::Warning;
    }
    return Severity::Normal;
}

Severity DiskActivitySeverity(const ResourceSample& sample) {
    return std::isfinite(sample.diskActivityPct) && sample.diskActivityPct >= 0 &&
           sample.diskActivityPct >= g_settings.diskActivityWarningPct.load()
        ? Severity::Warning : Severity::Normal;
}

Severity DiskWriteSeverity(const ResourceSample& sample) {
    const double threshold = g_settings.diskWriteWarningMiBps.load() *
        1024.0 * 1024.0;
    return std::isfinite(sample.diskWriteBps) && sample.diskWriteBps >= threshold
        ? Severity::Warning : Severity::Normal;
}

void UpdateTracker(AlertTracker* tracker,
                   Severity raw,
                   int warningSustainSeconds) {
    if (!tracker) {
        return;
    }

    if (raw == Severity::Critical) {
        ++tracker->criticalStreak;
        tracker->warningStreak = 0;
        tracker->recoveryStreak = 0;
        if (tracker->criticalStreak >=
            RequiredSamples(g_settings.criticalSustainSeconds.load())) {
            tracker->displayed = Severity::Critical;
        }
        return;
    }

    tracker->criticalStreak = 0;
    if (raw == Severity::Warning) {
        ++tracker->warningStreak;
        if (tracker->displayed == Severity::Critical) {
            ++tracker->recoveryStreak;
            if (tracker->recoveryStreak >=
                RequiredSamples(g_settings.recoverySeconds.load())) {
                tracker->displayed = Severity::Warning;
                tracker->recoveryStreak = 0;
            }
        } else {
            tracker->recoveryStreak = 0;
            if (tracker->warningStreak >=
                RequiredSamples(warningSustainSeconds)) {
                tracker->displayed = Severity::Warning;
            }
        }
        return;
    }

    tracker->warningStreak = 0;
    if (tracker->displayed == Severity::Normal) {
        tracker->recoveryStreak = 0;
        return;
    }

    ++tracker->recoveryStreak;
    if (tracker->recoveryStreak >=
        RequiredSamples(g_settings.recoverySeconds.load())) {
        tracker->displayed = Severity::Normal;
        tracker->recoveryStreak = 0;
    }
}

std::wstring FormatPercent(double value) {
    if (!std::isfinite(value) || value < 0) return L"--";
    wchar_t buffer[16]{};
    swprintf_s(buffer, L"%.0f%%", std::clamp(value, 0.0, 100.0));
    return buffer;
}

std::wstring FormatDisk(double freeGB) {
    wchar_t buffer[16]{};
    if (freeGB < 10.0) {
        swprintf_s(buffer, L"%.1fG", std::max(0.0, freeGB));
    } else {
        swprintf_s(buffer, L"%.0fG", freeGB);
    }
    return buffer;
}

std::wstring FormatNumber(double value, const wchar_t* suffix, int precision = 1) {
    if (!std::isfinite(value) || value < 0) return L"--";
    wchar_t buffer[80];
    swprintf_s(buffer, L"%.*f%s", precision, value, suffix);
    return buffer;
}

std::wstring FormatRate(double bytesPerSecond) {
    if (!std::isfinite(bytesPerSecond) || bytesPerSecond < 0) return L"--";
    constexpr double kib = 1024.0;
    constexpr double mib = kib * 1024.0;
    constexpr double gib = mib * 1024.0;
    if (bytesPerSecond < kib) return FormatNumber(bytesPerSecond, L" B/s", 0);
    if (bytesPerSecond < mib) return FormatNumber(bytesPerSecond / kib, L" KiB/s");
    if (bytesPerSecond < gib) return FormatNumber(bytesPerSecond / mib, L" MiB/s");
    return FormatNumber(bytesPerSecond / gib, L" GiB/s");
}

std::wstring FormatCompactRate(double bytesPerSecond) {
    if (!std::isfinite(bytesPerSecond) || bytesPerSecond < 0) return L"--";
    constexpr double kib = 1024.0;
    constexpr double mib = kib * 1024.0;
    constexpr double gib = mib * 1024.0;
    if (bytesPerSecond < mib)
        return FormatNumber(bytesPerSecond / kib, L"K/s", 0);
    if (bytesPerSecond < gib) {
        const double value = bytesPerSecond / mib;
        return FormatNumber(value, L"M/s", value < 10 ? 1 : 0);
    }
    const double value = bytesPerSecond / gib;
    return FormatNumber(value, L"G/s", value < 10 ? 1 : 0);
}

Severity ThresholdSeverity(double value, double warning, double critical) {
    if (!std::isfinite(value) || value < 0) return Severity::Normal;
    if (value >= critical) return Severity::Critical;
    return value >= warning ? Severity::Warning : Severity::Normal;
}

Severity VramSeverity(const GpuSample& gpu) {
    // A small UMA carve-out is not the GPU's full usable memory budget.
    // Shared memory pressure is covered by the system RAM/commit alerts.
    if (gpu.dedicatedTotalGB < 1.0) return Severity::Normal;
    return ThresholdSeverity(gpu.VramPct(), g_settings.vramWarningPct, g_settings.vramCriticalPct);
}

void UpdateKnownTracker(AlertTracker* tracker, double value, Severity raw, int seconds) {
    if (value < 0 || !std::isfinite(value)) {
        tracker->warningStreak = tracker->criticalStreak = tracker->recoveryStreak = 0;
        return; // Missing data cannot confirm recovery from an existing alert.
    }
    UpdateTracker(tracker, raw, seconds);
}

void UpdateAccumulatingWarning(AlertTracker* tracker, double value, Severity raw,
                               int requiredSeconds, int allowedGapSeconds) {
    if (!tracker) return;
    if (!std::isfinite(value) || value < 0) {
        tracker->warningStreak = tracker->criticalStreak = tracker->recoveryStreak = 0;
        tracker->warningAccumulatedMs = tracker->warningGapMs = 0;
        return; // Missing data cannot confirm recovery or bridge an episode.
    }

    const int sampleMs = std::max(500, g_settings.updateIntervalMs.load());
    const int requiredMs = requiredSeconds * 1000;
    const int allowedGapMs = allowedGapSeconds * 1000;
    tracker->criticalStreak = 0;
    if (raw == Severity::Warning) {
        tracker->warningAccumulatedMs = std::min(
            requiredMs, tracker->warningAccumulatedMs + sampleMs);
        tracker->warningGapMs = 0;
        tracker->recoveryStreak = 0;
        if (tracker->warningAccumulatedMs >= requiredMs)
            tracker->displayed = Severity::Warning;
        return;
    }

    if (tracker->displayed != Severity::Normal) {
        ++tracker->recoveryStreak;
        if (tracker->recoveryStreak >=
            RequiredSamples(g_settings.recoverySeconds.load())) {
            *tracker = {};
        }
        return;
    }

    tracker->recoveryStreak = 0;
    tracker->warningGapMs += sampleMs;
    if (tracker->warningGapMs > allowedGapMs) {
        tracker->warningAccumulatedMs = 0;
        tracker->warningGapMs = 0;
    }
}

std::vector<AlertItem> BuildAlertItems() {
    std::vector<AlertItem> items;
    const auto monitorSeverity = Tracker(ResourceKind::Monitor).displayed;
    if (monitorSeverity != Severity::Normal) {
        items.push_back({ResourceKind::Monitor, monitorSeverity, L"ERR"});
    }

    const auto append = [&](ResourceKind kind, std::wstring value,
                            std::wstring label = L"") {
        const Severity severity = Tracker(kind).displayed;
        if (severity != Severity::Normal) {
            items.push_back({kind, severity, std::move(value), std::move(label)});
        }
    };

    append(ResourceKind::Cpu, g_lastSample.valid ? FormatPercent(g_lastSample.cpuPct) : L"--");
    append(ResourceKind::Memory, g_lastSample.valid ? FormatPercent(g_lastSample.ramUsedPct) : L"--");
    append(ResourceKind::Commit, g_lastSample.valid ? FormatPercent(g_lastSample.commitPct) : L"--");
    append(ResourceKind::Disk, g_lastSample.valid ? FormatDisk(g_lastSample.diskFreeGB) : L"--");
    append(ResourceKind::DiskActivity, FormatPercent(g_lastSample.diskActivityPct),
        std::wstring(1, g_lastSample.diskLetter) + L": 活动率 / 读 " +
        FormatRate(g_lastSample.diskReadBps) + L" / 写 " + FormatRate(g_lastSample.diskWriteBps));
    append(ResourceKind::DiskWrite, FormatCompactRate(g_lastSample.diskWriteBps),
        std::wstring(1, g_lastSample.diskLetter) + L": 写入速度 · 累计写入告警");
    append(ResourceKind::CpuTemperature, FormatNumber(g_lastSample.cpuTemperature, L"\u00B0", 0));
    for (size_t i = 0; i < g_lastSample.gpus.size(); ++i) {
        const auto& gpu = g_lastSample.gpus[i];
        auto& trackers = g_gpuTrackers[gpu.id];
        const std::wstring prefix = L"GPU " + std::to_wstring(i) + L" · " + gpu.name;
        const auto add = [&](size_t metric, ResourceKind kind, std::wstring value, const wchar_t* label) {
            if (trackers[metric].displayed != Severity::Normal)
                items.push_back({kind, trackers[metric].displayed, std::move(value), prefix + L" / " + label});
        };
        add(0, ResourceKind::Gpu, FormatPercent(gpu.load), L"使用率");
        add(1, ResourceKind::Vram, FormatPercent(gpu.VramPct()), L"专用显存");
        add(2, ResourceKind::GpuTemperature, FormatNumber(gpu.temperature, L"\u00B0", 0), L"核心温度");
    }
    return items;
}

RuntimeSnapshot BuildSnapshot(ULONGLONG tick = GetTickCount64()) {
    RuntimeSnapshot snapshot;
    snapshot.alerts = BuildAlertItems();
    snapshot.tick = tick;
    SYSTEMTIME now{};
    GetLocalTime(&now);
    wchar_t time[32];
    swprintf_s(time, L"%02u:%02u:%02u", now.wHour, now.wMinute, now.wSecond);
    snapshot.updated = time;
    const auto& sample = g_lastSample;
    const auto add = [&](std::wstring key, std::wstring label, double numeric,
                         std::wstring formatted, const wchar_t* peakSuffix,
                         std::wstring threshold, Severity displayed, Severity raw,
                         bool minimum = false, int decimals = 1,
                         bool adaptiveRate = false,
                         std::optional<ResourceKind> icon = std::nullopt,
                         std::wstring gpuGroup = L"", bool gpuSummary = false) {
        const bool available = std::isfinite(numeric) && numeric >= 0;
        if (available) {
            auto [it, inserted] = g_extrema.try_emplace(key, RollingExtremum{minimum});
            if (!inserted && it->second.minimum != minimum) {
                it->second = RollingExtremum{minimum};
            }
            it->second.Add(tick, numeric);
        }
        auto extremum = g_extrema.find(key);
        const auto extremeValue = extremum == g_extrema.end()
            ? std::optional<double>{} : extremum->second.Value(tick);
        std::wstring state = !available ? L"不可用" :
            displayed == Severity::Critical ? L"严重" :
            displayed == Severity::Warning ? L"告警" :
            raw != Severity::Normal ? L"确认中" : L"正常";
        snapshot.overview.push_back({std::move(key), std::move(label),
            available ? std::move(formatted) : L"--",
            !extremeValue ? L"--" : adaptiveRate ? FormatRate(*extremeValue)
                : FormatNumber(*extremeValue, peakSuffix, decimals),
            std::move(threshold), std::move(state), displayed, available, minimum,
            icon, std::move(gpuGroup), gpuSummary});
    };
    const auto thresholds = [](int warning, int critical, const wchar_t* unit) {
        return std::to_wstring(warning) + unit + L" / " + std::to_wstring(critical) + unit;
    };
    const double cpu = sample.valid ? sample.cpuPct : -1;
    add(L"cpu", L"CPU 使用率", cpu, FormatPercent(cpu), L"%",
        L"≥" + std::to_wstring(g_settings.cpuWarningPct.load()) + L"% · 累计" +
            std::to_wstring(g_settings.cpuWarningSeconds.load()) + L"秒 · 间断≤" +
            std::to_wstring(g_settings.highUsageGapSeconds.load()) + L"秒",
        Tracker(ResourceKind::Cpu).displayed, CpuSeverity(sample), false, 0, false, ResourceKind::Cpu);
    add(L"cpu-temp", L"CPU 最高温度", sample.cpuTemperature,
        FormatNumber(sample.cpuTemperature, L" °C"), L" °C",
        thresholds(g_settings.cpuTemperatureWarning, g_settings.cpuTemperatureCritical, L"°C"),
        Tracker(ResourceKind::CpuTemperature).displayed,
        ThresholdSeverity(sample.cpuTemperature, g_settings.cpuTemperatureWarning, g_settings.cpuTemperatureCritical),
        false, 1, false, ResourceKind::CpuTemperature);
    const double ram = sample.valid ? sample.ramUsedPct : -1;
    add(L"ram", L"物理内存", ram, FormatPercent(ram) + L" · " +
        FormatNumber(sample.physicalTotalGB - sample.physicalAvailableMB / 1024, L"") + L" / " +
        FormatNumber(sample.physicalTotalGB, L" GiB"), L"%", L"可用<10%且<4GiB\n严重<1%且<500MiB",
        Tracker(ResourceKind::Memory).displayed, MemorySeverity(sample), false, 0, false, ResourceKind::Memory);
    add(L"commit", L"提交内存", sample.valid ? sample.commitPct : -1,
        FormatPercent(sample.commitPct) + L" · " + FormatNumber(sample.commitUsedGB, L"") + L" / " +
        FormatNumber(sample.commitLimitGB, L" GiB"), L"%",
        thresholds(g_settings.commitWarningPct, g_settings.commitCriticalPct, L"%"),
        Tracker(ResourceKind::Commit).displayed, CommitSeverity(sample), false, 0, false, ResourceKind::Commit);
    add(L"disk", std::wstring(1, sample.diskLetter) + L": 可用空间", sample.valid ? sample.diskFreeGB : -1,
        FormatNumber(sample.diskFreeGB, L"") + L" / " + FormatNumber(sample.diskTotalGB, L" GiB"), L" GiB",
        L"<" + std::to_wstring(g_settings.diskWarningPct.load()) + L"%且<" + std::to_wstring(g_settings.diskWarningGB.load()) +
        L"GiB\n严重<" + std::to_wstring(g_settings.diskCriticalPct.load()) + L"%且<" + std::to_wstring(g_settings.diskCriticalGB.load()) + L"GiB",
        Tracker(ResourceKind::Disk).displayed, DiskSeverity(sample), true, 1, false, ResourceKind::Disk);
    const std::wstring diskPrefix = std::wstring(1, sample.diskLetter) + L": ";
    add(L"disk-activity", diskPrefix + L"活动率", sample.diskActivityPct,
        FormatPercent(sample.diskActivityPct), L"%",
        L"≥" + std::to_wstring(g_settings.diskActivityWarningPct.load()) + L"% · 累计" +
            std::to_wstring(g_settings.diskActivityWarningSeconds.load()) + L"秒 · 间断≤" +
            std::to_wstring(g_settings.highUsageGapSeconds.load()) + L"秒",
        Tracker(ResourceKind::DiskActivity).displayed, DiskActivitySeverity(sample), false, 0, false, ResourceKind::DiskActivity);
    add(L"disk-read", diskPrefix + L"读取速度", sample.diskReadBps,
        FormatRate(sample.diskReadBps), L"", L"诊断指标，无通用阈值",
        Severity::Normal, Severity::Normal, false, 1, true);
    add(L"disk-write", diskPrefix + L"写入速度", sample.diskWriteBps,
        FormatRate(sample.diskWriteBps), L"",
        L"≥" + std::to_wstring(g_settings.diskWriteWarningMiBps.load()) +
            L" MiB/s · 累计" + std::to_wstring(g_settings.diskWriteWarningSeconds.load()) +
            L"秒 · 间断≤" + std::to_wstring(g_settings.diskWriteGapSeconds.load()) + L"秒",
        Tracker(ResourceKind::DiskWrite).displayed, DiskWriteSeverity(sample), false, 1, true, ResourceKind::DiskWrite);
    add(L"processes", L"进程数", sample.valid ? sample.processCount : -1.0, std::to_wstring(sample.processCount), L"", L"—", Severity::Normal, Severity::Normal, false, 0);
    add(L"threads", L"线程数", sample.valid ? sample.threadCount : -1.0, std::to_wstring(sample.threadCount), L"", L"—", Severity::Normal, Severity::Normal, false, 0);
    add(L"handles", L"句柄数", sample.valid ? sample.handleCount : -1.0, std::to_wstring(sample.handleCount), L"", L"—", Severity::Normal, Severity::Normal, false, 0);
    for (size_t i = 0; i < sample.gpus.size(); ++i) {
        const auto& gpu = sample.gpus[i];
        const auto& trackers = g_gpuTrackers[gpu.id];
        const auto prefix = L"GPU " + std::to_wstring(i) + L" ";
        snapshot.devices += prefix + gpu.name + (gpu.nvml ? L" [NVML + Windows]\n" : L" [Windows]\n");
        add(gpu.id + L"load", prefix + L"使用率", gpu.load, FormatPercent(gpu.load), L"%",
            std::to_wstring(g_settings.gpuWarningPct.load()) + L"% · " + std::to_wstring(g_settings.gpuWarningSeconds.load()) + L"秒",
            trackers[0].displayed, ThresholdSeverity(gpu.load, g_settings.gpuWarningPct, 101),
            false, 0, false, ResourceKind::Gpu, gpu.id);
        add(gpu.id + L"vram", prefix + L"专用显存", gpu.VramPct(),
            FormatPercent(gpu.VramPct()) + L" · " + FormatNumber(gpu.dedicatedUsedGB, L"") + L" / " + FormatNumber(gpu.dedicatedTotalGB, L" GiB"), L"%",
            gpu.dedicatedTotalGB < 1 ? L"小容量 UMA 不告警" : thresholds(g_settings.vramWarningPct, g_settings.vramCriticalPct, L"%"),
            trackers[1].displayed, VramSeverity(gpu),
            false, 0, false, ResourceKind::Vram, gpu.id);
        add(gpu.id + L"shared", prefix + L"共享显存", gpu.sharedUsedGB, FormatNumber(gpu.sharedUsedGB, L" GiB"), L" GiB",
            L"参照系统内存告警", Severity::Normal, Severity::Normal,
            false, 1, false, ResourceKind::Vram, gpu.id);
        add(gpu.id + L"temp", prefix + L"核心温度", gpu.temperature, FormatNumber(gpu.temperature, L" °C"), L" °C",
            thresholds(g_settings.gpuTemperatureWarning, g_settings.gpuTemperatureCritical, L"°C"),
            trackers[2].displayed, ThresholdSeverity(gpu.temperature, g_settings.gpuTemperatureWarning, g_settings.gpuTemperatureCritical),
            false, 1, false, ResourceKind::GpuTemperature, gpu.id);
        add(gpu.id + L"power", prefix + L"功耗", gpu.powerW, FormatNumber(gpu.powerW, L" W"), L" W", L"—", Severity::Normal, Severity::Normal);
        snapshot.overview.back().icon = ResourceKind::Gpu;
        snapshot.overview.back().gpuGroup = gpu.id;
        add(gpu.id + L"clock", prefix + L"核心频率", gpu.clockMHz, FormatNumber(gpu.clockMHz, L" MHz", 0), L" MHz", L"—",
            Severity::Normal, Severity::Normal, false, 0, false, ResourceKind::Gpu, gpu.id);

        const auto join = [](const std::vector<std::wstring>& parts) {
            std::wstring result;
            for (const auto& part : parts) {
                if (!result.empty()) result += L" · ";
                result += part;
            }
            return result.empty() ? std::wstring(L"--") : result;
        };
        const auto rowPeak = [&](const std::wstring& key) {
            const auto found = std::find_if(snapshot.overview.begin(), snapshot.overview.end(),
                [&](const auto& row) { return row.key == key; });
            return found == snapshot.overview.end() ? std::wstring(L"--") : found->peak;
        };
        std::vector<std::wstring> currentParts, peakParts;
        if (gpu.load >= 0) {
            currentParts.push_back(L"负载 " + FormatPercent(gpu.load));
            peakParts.push_back(L"负载 " + rowPeak(gpu.id + L"load"));
        }
        if (gpu.VramPct() >= 0) {
            currentParts.push_back(L"显存 " + FormatPercent(gpu.VramPct()));
            peakParts.push_back(L"显存 " + rowPeak(gpu.id + L"vram"));
        }
        if (gpu.temperature >= 0) {
            currentParts.push_back(L"温度 " + FormatNumber(gpu.temperature, L" °C", 0));
            peakParts.push_back(L"温度 " + rowPeak(gpu.id + L"temp"));
        }
        const Severity summaryDisplayed = std::max({trackers[0].displayed,
            trackers[1].displayed, trackers[2].displayed});
        const Severity summaryRaw = std::max({
            ThresholdSeverity(gpu.load, g_settings.gpuWarningPct, 101),
            VramSeverity(gpu),
            ThresholdSeverity(gpu.temperature, g_settings.gpuTemperatureWarning,
                              g_settings.gpuTemperatureCritical)});
        const bool summaryAvailable = !currentParts.empty();
        const std::wstring summaryState = !summaryAvailable ? L"不可用" :
            summaryDisplayed == Severity::Critical ? L"严重" :
            summaryDisplayed == Severity::Warning ? L"告警" :
            summaryRaw != Severity::Normal ? L"确认中" : L"正常";
        snapshot.overview.push_back({gpu.id + L"summary", L"GPU " + std::to_wstring(i),
            join(currentParts), join(peakParts), L"点击展开或折叠 · " + gpu.name,
            summaryState, summaryDisplayed, summaryAvailable, false,
            ResourceKind::Gpu, gpu.id, true});
    }
    if (sample.cpuTemperature >= 0)
        snapshot.devices += L"CPU 温度来源：LibreHardwareMonitor（" +
            std::wstring(sample.cpuTemperatureSource == 1 ? L"本地 Web Server" : L"旧版 WMI") + L"，CPU 温度传感器最高值）。";
    for (auto it = g_extrema.begin(); it != g_extrema.end();) {
        it->second.Expire(tick);
        if (it->second.candidates.empty()) it = g_extrema.erase(it);
        else ++it;
    }
    const auto priority = [](const OverviewRow& row) {
        if (row.severity == Severity::Critical) return 0;
        if (row.severity == Severity::Warning) return 1;
        if (row.state == L"确认中") return 2;
        return row.available ? 3 : 4;
    };
    struct OverviewBlock {
        std::wstring gpuGroup;
        std::vector<OverviewRow> rows;
        int priority = 4;
    };
    std::vector<OverviewBlock> blocks;
    std::map<std::wstring, size_t> gpuBlocks;
    for (auto& row : snapshot.overview) {
        if (row.gpuGroup.empty()) {
            blocks.push_back({L"", {}, priority(row)});
            blocks.back().rows.push_back(std::move(row));
            continue;
        }
        auto [found, inserted] = gpuBlocks.try_emplace(row.gpuGroup, blocks.size());
        if (inserted) blocks.push_back({row.gpuGroup, {}, priority(row)});
        auto& block = blocks[found->second];
        block.priority = std::min(block.priority, priority(row));
        block.rows.push_back(std::move(row));
    }
    for (auto& block : blocks) {
        if (block.gpuGroup.empty()) continue;
        std::stable_sort(block.rows.begin(), block.rows.end(), [&](const auto& left, const auto& right) {
            if (left.gpuSummary != right.gpuSummary) return left.gpuSummary;
            return priority(left) < priority(right);
        });
    }
    std::stable_sort(blocks.begin(), blocks.end(),
        [](const auto& left, const auto& right) { return left.priority < right.priority; });
    snapshot.overview.clear();
    for (auto& block : blocks)
        for (auto& row : block.rows) snapshot.overview.push_back(std::move(row));
    return snapshot;
}

// Native XAML display layer. No HWND is created by this mod.
struct RowMetrics {
    double height = 0;
    double gap = 0;
    double fontSize = 0;
    double iconSize = 0;
};

std::optional<RowMetrics> CalculateNativeRows(size_t count, double taskbarHeight) {
    const size_t rows = std::min(count, count == 3 ? size_t{3} : size_t{2});
    if (count == 0 || count > kMaxAlerts || !std::isfinite(taskbarHeight) ||
        taskbarHeight < rows * 12.0) {
        return std::nullopt;
    }
    const double padding = std::min(2.0, (taskbarHeight - rows * 12.0) / 2.0);
    const double available = taskbarHeight - 2.0 * padding;
    const bool compactThree = count == 3;
    const bool custom = g_settings.useCustomRowSpacing.load() && !compactThree;
    const int requestedGap = custom ? g_settings.rowGap.load() : 0;
    const int requestedHeight = compactThree ? 14 : custom ? g_settings.rowHeight.load() : 18;
    const double gap = rows == 1 ? 0.0 :
        std::min<double>(requestedGap,
                         (available - rows * 12.0) / (rows - 1));
    const double height = std::min<double>(
        requestedHeight, (available - (rows - 1) * gap) / rows);
    return RowMetrics{height, gap,
                      std::min(compactThree ? 11.0 : 40.0 / 3.0, height - 1.0),
                      std::min(compactThree ? 12.0 : 15.0, height - 2.0)};
}

constexpr size_t AlertRowsPerColumn(size_t count) {
    return count == 3 ? 3 : 2;
}

// Microsoft Fluent UI System Icons, 16 px regular variants (MIT).
// Keeping the original path data gives the taskbar and overview the same
// carefully hinted geometry at native small sizes.
Media::PathGeometry BuildIconGeometry(ResourceKind kind) {
    const wchar_t* data = nullptr;
    switch (kind) {
        case ResourceKind::Cpu:
            data = LR"PATH(M14.5 8.5C14.633 8.5 14.76 8.447 14.854 8.354C14.948 8.26 15 8.133 15 8C15 7.867 14.947 7.74 14.854 7.646C14.76 7.552 14.633 7.5 14.5 7.5H13V6H14.5C14.633 6 14.76 5.947 14.854 5.854C14.948 5.76 15 5.633 15 5.5C15 5.367 14.947 5.24 14.854 5.146C14.76 5.052 14.633 5 14.5 5H13C13 4.47 12.789 3.961 12.414 3.586C12.039 3.211 11.53 3 11 3V1.5C11 1.367 10.947 1.24 10.854 1.146C10.76 1.052 10.633 1 10.5 1C10.367 1 10.24 1.053 10.146 1.146C10.052 1.24 10 1.367 10 1.5V3H8.5V1.5C8.5 1.367 8.447 1.24 8.354 1.146C8.26 1.052 8.133 1 8 1C7.867 1 7.74 1.053 7.646 1.146C7.552 1.24 7.5 1.367 7.5 1.5V3H6V1.5C6 1.367 5.947 1.24 5.854 1.146C5.76 1.052 5.633 1 5.5 1C5.367 1 5.24 1.053 5.146 1.146C5.052 1.24 5 1.367 5 1.5V3C4.47 3 3.961 3.211 3.586 3.586C3.211 3.961 3 4.47 3 5H1.5C1.367 5 1.24 5.053 1.146 5.146C1.052 5.24 1 5.367 1 5.5C1 5.633 1.053 5.76 1.146 5.854C1.24 5.948 1.367 6 1.5 6H3V7.5H1.5C1.367 7.5 1.24 7.553 1.146 7.646C1.052 7.74 1 7.867 1 8C1 8.133 1.053 8.26 1.146 8.354C1.24 8.448 1.367 8.5 1.5 8.5H3V10H1.5C1.367 10 1.24 10.053 1.146 10.146C1.052 10.24 1 10.367 1 10.5C1 10.633 1.053 10.76 1.146 10.854C1.24 10.948 1.367 11 1.5 11H3C3 11.53 3.211 12.039 3.586 12.414C3.961 12.789 4.47 13 5 13V14.5C5 14.633 5.053 14.76 5.146 14.854C5.24 14.948 5.367 15 5.5 15C5.633 15 5.76 14.947 5.854 14.854C5.948 14.76 6 14.633 6 14.5V13H7.5V14.5C7.5 14.633 7.553 14.76 7.646 14.854C7.74 14.948 7.867 15 8 15C8.133 15 8.26 14.947 8.354 14.854C8.448 14.76 8.5 14.633 8.5 14.5V13H10V14.5C10 14.633 10.053 14.76 10.146 14.854C10.24 14.948 10.367 15 10.5 15C10.633 15 10.76 14.947 10.854 14.854C10.948 14.76 11 14.633 11 14.5V13C11.53 13 12.039 12.789 12.414 12.414C12.789 12.039 13 11.53 13 11H14.5C14.633 11 14.76 10.947 14.854 10.854C14.948 10.76 15 10.633 15 10.5C15 10.367 14.947 10.24 14.854 10.146C14.76 10.052 14.633 10 14.5 10H13V8.5H14.5ZM12 11C12 11.265 11.895 11.52 11.707 11.707C11.519 11.894 11.265 12 11 12H5C4.735 12 4.48 11.895 4.293 11.707C4.105 11.519 4 11.265 4 11V5C4 4.735 4.105 4.48 4.293 4.293C4.481 4.105 4.735 4 5 4H11C11.265 4 11.52 4.105 11.707 4.293C11.895 4.481 12 4.735 12 5V11ZM8 10.5C6.621 10.5 5.5 9.379 5.5 8C5.5 6.621 6.621 5.5 8 5.5C9.379 5.5 10.5 6.621 10.5 8C10.5 9.379 9.379 10.5 8 10.5ZM8 6.5C7.173 6.5 6.5 7.173 6.5 8C6.5 8.827 7.173 9.5 8 9.5C8.827 9.5 9.5 8.827 9.5 8C9.5 7.173 8.827 6.5 8 6.5Z)PATH";
            break;
        case ResourceKind::Memory:
            data = LR"PATH(M3.5 5H12.5C12.776 5 13 5.224 13 5.5V8.5C13 8.776 12.776 9 12.5 9H3.5C3.224 9 3 8.776 3 8.5V5.5C3 5.224 3.224 5 3.5 5ZM9 8V6H7V8H9ZM6 6H4V8H6V6ZM10 8H12V6H10V8ZM3 3H13C14.103 3 15 3.897 15 5V9C15 9.739 14.597 10.385 14 10.731V11.75C14 12.439 13.439 13 12.75 13H9.5C9.367 13 9.24 12.948 9.146 12.854L8 11.708L6.854 12.854C6.76 12.947 6.633 13 6.5 13H3.25C2.561 13 2 12.439 2 11.75V10.731C1.403 10.385 1 9.739 1 9V5C1 3.897 1.897 3 3 3ZM3.25 12H6.293L7.293 11H3V11.75C3 11.888 3.112 12 3.25 12ZM12.75 12C12.888 12 13 11.888 13 11.75V11H8.707L9.707 12H12.75ZM13 10C13.552 10 14 9.551 14 9V5C14 4.449 13.552 4 13 4H3C2.448 4 2 4.449 2 5V9C2 9.551 2.448 10 3 10H13Z)PATH";
            break;
        case ResourceKind::Commit:
            data = LR"PATH(M6 1C4.89543 1 4 1.89543 4 3V5.20703C4.32228 5.11588 4.65659 5.05337 5 5.02242V3C5 2.44772 5.44772 2 6 2H9V4.5C9 5.32843 9.67157 6 10.5 6H13V13C13 13.5523 12.5523 14 12 14H9.74284C9.42948 14.3794 9.06621 14.7161 8.66308 15H12C13.1046 15 14 14.1046 14 13V5.41421C14 5.01639 13.842 4.63486 13.5607 4.35355L10.6464 1.43934C10.3651 1.15804 9.98361 1 9.58579 1H6ZM12.7929 5H10.5C10.2239 5 10 4.77614 10 4.5V2.20711L12.7929 5ZM10 10.5C10 12.9853 7.98528 15 5.5 15C3.01472 15 1 12.9853 1 10.5C1 8.01472 3.01472 6 5.5 6C7.98528 6 10 8.01472 10 10.5ZM6 8.5C6 8.22386 5.77614 8 5.5 8C5.22386 8 5 8.22386 5 8.5V10H3.5C3.22386 10 3 10.2239 3 10.5C3 10.7761 3.22386 11 3.5 11H5L5 12.5C5 12.7761 5.22386 13 5.5 13C5.77614 13 6 12.7761 6 12.5V11H7.5C7.77614 11 8 10.7761 8 10.5C8 10.2239 7.77614 10 7.5 10H6V8.5Z)PATH";
            break;
        case ResourceKind::Disk:
            data = LR"PATH(M13.854 7.85407L11.885 3.72707V3.72007C11.7857 3.50319 11.6256 3.31976 11.4241 3.19198C11.2227 3.06419 10.9885 2.99753 10.75 3.00007H5.25C5.01163 2.99773 4.77769 3.06448 4.57645 3.19225C4.37521 3.32002 4.21528 3.50335 4.116 3.72007V3.72707L2.146 7.85407C2.04981 8.05584 1.99993 8.27655 2 8.50007V10.5001C2 10.8979 2.15804 11.2794 2.43934 11.5607C2.72064 11.842 3.10218 12.0001 3.5 12.0001H12.5C12.8978 12.0001 13.2794 11.842 13.5607 11.5607C13.842 11.2794 14 10.8979 14 10.5001V8.50007C14.0001 8.27655 13.9502 8.05584 13.854 7.85407V7.85407ZM5.25 4.00007H10.75C10.7973 3.99854 10.8439 4.01101 10.8842 4.03592C10.9244 4.06083 10.9563 4.09706 10.976 4.14007V4.14007L12.339 7.00007H3.661L5.023 4.14307C5.0423 4.09934 5.07421 4.06237 5.11465 4.03689C5.1551 4.01141 5.20222 3.99859 5.25 4.00007V4.00007ZM13 10.5001C13 10.6327 12.9473 10.7599 12.8536 10.8536C12.7598 10.9474 12.6326 11.0001 12.5 11.0001H3.5C3.36739 11.0001 3.24022 10.9474 3.14645 10.8536C3.05268 10.7599 3 10.6327 3 10.5001V8.50007C3 8.36746 3.05268 8.24028 3.14645 8.14652C3.24022 8.05275 3.36739 8.00007 3.5 8.00007H12.5C12.5943 8.00006 12.6867 8.02672 12.7665 8.07697C12.8463 8.12722 12.9103 8.19902 12.951 8.28407C12.983 8.3516 12.9997 8.42535 13 8.50007V10.5001ZM12 9.50007C12 9.59896 11.9707 9.69563 11.9157 9.77785C11.8608 9.86008 11.7827 9.92417 11.6913 9.96201C11.6 9.99985 11.4994 10.0098 11.4025 9.99046C11.3055 9.97117 11.2164 9.92355 11.1464 9.85362C11.0765 9.7837 11.0289 9.69461 11.0096 9.59762C10.9903 9.50062 11.0002 9.40009 11.0381 9.30873C11.0759 9.21737 11.14 9.13928 11.2222 9.08433C11.3044 9.02939 11.4011 9.00007 11.5 9.00007C11.6326 9.00007 11.7598 9.05275 11.8536 9.14652C11.9473 9.24028 12 9.36746 12 9.50007Z)PATH";
            break;
        case ResourceKind::Monitor:
            data = LR"PATH(M8 1C11.866 1 15 4.13401 15 8C15 11.866 11.866 15 8 15C4.13401 15 1 11.866 1 8C1 4.13401 4.13401 1 8 1ZM8 2C4.68629 2 2 4.68629 2 8C2 11.3137 4.68629 14 8 14C11.3137 14 14 11.3137 14 8C14 4.68629 11.3137 2 8 2ZM8 10C8.41421 10 8.75 10.3358 8.75 10.75C8.75 11.1642 8.41421 11.5 8 11.5C7.58579 11.5 7.25 11.1642 7.25 10.75C7.25 10.3358 7.58579 10 8 10ZM8 4.5C8.24546 4.5 8.44985 4.67691 8.49219 4.91016L8.5 5V8.5C8.5 8.77614 8.27614 9 8 9C7.75454 9 7.55015 8.82309 7.50781 8.58984L7.5 8.5V5C7.5 4.72386 7.72386 4.5 8 4.5Z)PATH";
            break;
        case ResourceKind::Gpu:
            data = LR"PATH(M4 2C2.89543 2 2 2.89543 2 4V9.99729C2 11.1019 2.89543 11.9973 4 11.9973H6.005V13.008L4.50598 13.008C4.22984 13.008 4.00598 13.2319 4.00599 13.508C4.00599 13.7841 4.22985 14.008 4.50599 14.008L11.5021 14.0079C11.7783 14.0079 12.0021 13.7841 12.0021 13.5079C12.0021 13.2318 11.7783 13.0079 11.5021 13.0079L10.0029 13.0079V11.9973H12C13.1046 11.9973 14 11.1019 14 9.99729V4C14 2.89543 13.1046 2 12 2H4ZM9.00293 11.9973V13.008L7.005 13.008V11.9973H9.00293ZM3 4C3 3.44772 3.44772 3 4 3H12C12.5523 3 13 3.44772 13 4V9.99729C13 10.5496 12.5523 10.9973 12 10.9973H4C3.44772 10.9973 3 10.5496 3 9.99729V4Z)PATH";
            break;
        case ResourceKind::Vram:
            data = LR"PATH(M14.414 4.586C14.039 4.2109 13.5304 4.00011 13 4V2.5C13 2.36739 12.9473 2.24021 12.8536 2.14645C12.7598 2.05268 12.6326 2 12.5 2C12.3674 2 12.2402 2.05268 12.1464 2.14645C12.0527 2.24021 12 2.36739 12 2.5V4H10V2.5C10 2.36739 9.94732 2.24021 9.85355 2.14645C9.75979 2.05268 9.63261 2 9.5 2C9.36739 2 9.24021 2.05268 9.14645 2.14645C9.05268 2.24021 9 2.36739 9 2.5V4H7V2.5C7 2.36739 6.94732 2.24021 6.85355 2.14645C6.75979 2.05268 6.63261 2 6.5 2C6.36739 2 6.24021 2.05268 6.14645 2.14645C6.05268 2.24021 6 2.36739 6 2.5V4H4V2.5C4 2.36739 3.94732 2.24021 3.85355 2.14645C3.75979 2.05268 3.63261 2 3.5 2C3.36739 2 3.24021 2.05268 3.14645 2.14645C3.05268 2.24021 3 2.36739 3 2.5V4C2.46957 4 1.96086 4.21071 1.58579 4.58579C1.21071 4.96086 1 5.46957 1 6V10C1 10.5304 1.21071 11.0391 1.58579 11.4142C1.96086 11.7893 2.46957 12 3 12V13.5C3 13.6326 3.05268 13.7598 3.14645 13.8536C3.24021 13.9473 3.36739 14 3.5 14C3.63261 14 3.75979 13.9473 3.85355 13.8536C3.94732 13.7598 4 13.6326 4 13.5V12H6V13.5C6 13.6326 6.05268 13.7598 6.14645 13.8536C6.24021 13.9473 6.36739 14 6.5 14C6.63261 14 6.75979 13.9473 6.85355 13.8536C6.94732 13.7598 7 13.6326 7 13.5V12H9V13.5C9 13.6326 9.05268 13.7598 9.14645 13.8536C9.24021 13.9473 9.36739 14 9.5 14C9.63261 14 9.75979 13.9473 9.85355 13.8536C9.94732 13.7598 10 13.6326 10 13.5V12H12V13.5C12 13.6326 12.0527 13.7598 12.1464 13.8536C12.2402 13.9473 12.3674 14 12.5 14C12.6326 14 12.7598 13.9473 12.8536 13.8536C12.9473 13.7598 13 13.6326 13 13.5V12C13.5304 12 14.0391 11.7893 14.4142 11.4142C14.7893 11.0391 15 10.5304 15 10V6C14.9999 5.46961 14.7891 4.96099 14.414 4.586ZM14 10C14 10.2652 13.8946 10.5196 13.7071 10.7071C13.5196 10.8946 13.2652 11 13 11H3C2.73478 11 2.48043 10.8946 2.29289 10.7071C2.10536 10.5196 2 10.2652 2 10V6C2 5.73478 2.10536 5.48043 2.29289 5.29289C2.48043 5.10536 2.73478 5 3 5H13C13.2652 5 13.5196 5.10536 13.7071 5.29289C13.8946 5.48043 14 5.73478 14 6V10ZM12.5 6H3.5C3.36739 6 3.24021 6.05268 3.14645 6.14645C3.05268 6.24021 3 6.36739 3 6.5V9.5C3 9.63261 3.05268 9.75979 3.14645 9.85355C3.24021 9.94732 3.36739 10 3.5 10H12.5C12.6326 10 12.7598 9.94732 12.8536 9.85355C12.9473 9.75979 13 9.63261 13 9.5V6.5C13 6.36739 12.9473 6.24021 12.8536 6.14645C12.7598 6.05268 12.6326 6 12.5 6ZM12 9H4V7H12V9Z)PATH";
            break;
        case ResourceKind::GpuTemperature:
            data = LR"PATH(M5.00015 3.5C5.00015 2.11943 6.11887 1 7.49972 1C8.88042 1 9.9997 2.11927 9.99972 3.49997V3.49997L10.0001 8.95055C10.6181 9.58133 11 10.4463 11 11.4C11 13.333 9.433 14.9 7.5 14.9C5.567 14.9 4 13.333 4 11.4C4 10.4462 4.38202 9.58114 5.00015 8.95034V3.5ZM7.49972 2C6.67143 2 6.00015 2.67143 6.00015 3.5V9.38726L5.83358 9.53632C5.32125 9.99481 5 10.6595 5 11.4C5 12.7807 6.11929 13.9 7.5 13.9C8.88071 13.9 10 12.7807 10 11.4C10 10.6596 9.67882 9.99497 9.1666 9.53649L9.00008 9.38744L8.99972 3.50003C8.99972 2.6716 8.32815 2 7.49972 2ZM8.0002 5.9999C8.0002 5.72376 7.77634 5.4999 7.5002 5.4999C7.22405 5.4999 7.0002 5.72376 7.0002 5.9999V10.0851C6.4174 10.2909 5.9998 10.8466 5.9998 11.4999C5.9998 12.3283 6.67138 12.9999 7.4998 12.9999C8.32823 12.9999 8.9998 12.3283 8.9998 11.4999C8.9998 10.8469 8.58259 10.2914 8.0002 10.0854V5.9999Z)PATH";
            break;
        case ResourceKind::CpuTemperature:
            data = LR"PATH(M5.00015 3.5C5.00015 2.11943 6.11887 1 7.49972 1C8.88042 1 9.9997 2.11927 9.99972 3.49997V3.49997L10.0001 8.95055C10.6181 9.58133 11 10.4463 11 11.4C11 13.333 9.433 14.9 7.5 14.9C5.567 14.9 4 13.333 4 11.4C4 10.4462 4.38202 9.58114 5.00015 8.95034V3.5ZM7.49972 2C6.67143 2 6.00015 2.67143 6.00015 3.5V9.38726L5.83358 9.53632C5.32125 9.99481 5 10.6595 5 11.4C5 12.7807 6.11929 13.9 7.5 13.9C8.88071 13.9 10 12.7807 10 11.4C10 10.6596 9.67882 9.99497 9.1666 9.53649L9.00008 9.38744L8.99972 3.50003C8.99972 2.6716 8.32815 2 7.49972 2ZM8.0002 5.9999C8.0002 5.72376 7.77634 5.4999 7.5002 5.4999C7.22405 5.4999 7.0002 5.72376 7.0002 5.9999V10.0851C6.4174 10.2909 5.9998 10.8466 5.9998 11.4999C5.9998 12.3283 6.67138 12.9999 7.4998 12.9999C8.32823 12.9999 8.9998 12.3283 8.9998 11.4999C8.9998 10.8469 8.58259 10.2914 8.0002 10.0854V5.9999Z)PATH";
            break;
        case ResourceKind::Overview:
            data = LR"PATH(M3.75 2C2.7835 2 2 2.7835 2 3.75V12.25C2 13.2165 2.7835 14 3.75 14H12.25C13.2165 14 14 13.2165 14 12.25V3.75C14 2.7835 13.2165 2 12.25 2H3.75ZM3 3.75C3 3.33579 3.33579 3 3.75 3H12.25C12.6642 3 13 3.33579 13 3.75V12.25C13 12.6642 12.6642 13 12.25 13H3.75C3.33579 13 3 12.6642 3 12.25V3.75ZM6 6.5C6 6.22386 5.77614 6 5.5 6C5.22386 6 5 6.22386 5 6.5V10.5C5 10.7761 5.22386 11 5.5 11C5.77614 11 6 10.7761 6 10.5V6.5ZM8 8C8.27614 8 8.5 8.22386 8.5 8.5V10.5C8.5 10.7761 8.27614 11 8 11C7.72386 11 7.5 10.7761 7.5 10.5V8.5C7.5 8.22386 7.72386 8 8 8ZM11 5.5C11 5.22386 10.7761 5 10.5 5C10.2239 5 10 5.22386 10 5.5V10.5C10 10.7761 10.2239 11 10.5 11C10.7761 11 11 10.7761 11 10.5V5.5Z)PATH";
            break;
        case ResourceKind::DiskActivity:
            data = LR"PATH(M8 1C11.866 1 15 4.13401 15 8C15 11.866 11.866 15 8 15C4.13401 15 1 11.866 1 8C1 4.13401 4.13401 1 8 1ZM8 2C4.68629 2 2 4.68629 2 8C2 11.3137 4.68629 14 8 14C11.3137 14 14 11.3137 14 8C14 4.68629 11.3137 2 8 2ZM8 3C8.1873 3 8.3724 3.01013 8.55469 3.03027C8.82903 3.0606 9.02719 3.30771 8.99707 3.58203C8.96674 3.85642 8.71969 4.05461 8.44531 4.02441C8.29935 4.00828 8.15061 4 8 4C5.79086 4 4 5.79086 4 8C4 9.25635 4.5791 10.3772 5.48633 11.1113C5.70073 11.2851 5.7332 11.5999 5.55957 11.8145C5.38583 12.029 5.07105 12.0623 4.85645 11.8887C3.7248 10.9728 3 9.57081 3 8C3 5.23858 5.23858 3 8 3ZM12.418 7.00293C12.6923 6.97281 12.9394 7.17097 12.9697 7.44531C12.9899 7.6276 13 7.8127 13 8C13 9.57081 12.2752 10.9728 11.1436 11.8887C10.9289 12.0623 10.6142 12.029 10.4404 11.8145C10.2668 11.5999 10.2993 11.2851 10.5137 11.1113C11.4209 10.3772 12 9.25635 12 8C12 7.84938 11.9917 7.70065 11.9756 7.55469C11.9454 7.28031 12.1436 7.03326 12.418 7.00293ZM10.2109 4.93262C10.5019 4.74095 10.7953 4.42643 11.1318 4.5166C11.2138 4.53856 11.2912 4.58219 11.3555 4.64648C11.4207 4.71176 11.4637 4.79071 11.4854 4.87402C11.5722 5.20877 11.261 5.49858 11.0703 5.78711C10.5034 6.645 9.25687 8.50876 8.88379 8.88184C8.39567 9.3698 7.60433 9.3698 7.11621 8.88184C6.6281 8.39373 6.6282 7.60242 7.11621 7.11426C7.48885 6.74161 9.35058 5.49934 10.2109 4.93262Z)PATH";
            break;
        case ResourceKind::DiskWrite:
            data = LR"PATH(M3.5 13H12.5C12.7761 13 13 13.2239 13 13.5C13 13.7455 12.8231 13.9496 12.5899 13.9919L12.5 14H3.5C3.22386 14 3 13.7761 3 13.5C3 13.2545 3.17688 13.0504 3.41012 13.0081L3.5 13H12.5H3.5ZM7.91012 1.00806L8 1C8.24546 1 8.44961 1.17688 8.49194 1.41012L8.5 1.5V10.292L11.182 7.61091C11.3555 7.43735 11.625 7.41806 11.8198 7.55306L11.8891 7.61091C12.0627 7.78448 12.0819 8.0539 11.9469 8.24877L11.8891 8.31802L8.35355 11.8536C8.17999 12.0271 7.91056 12.0464 7.71569 11.9114L7.64645 11.8536L4.11091 8.31802C3.91565 8.12276 3.91565 7.80617 4.11091 7.61091C4.28448 7.43735 4.5539 7.41806 4.74877 7.55306L4.81802 7.61091L7.5 10.292V1.5C7.5 1.25454 7.67688 1.05039 7.91012 1.00806L8 1L7.91012 1.00806Z)PATH";
            break;
    }
    if (!data) return Media::PathGeometry();
    std::wstring xaml = LR"XAML(<Path xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation" Data=")XAML";
    xaml += data;
    xaml += LR"XAML("/>)XAML";
    auto path = Markup::XamlReader::Load(xaml).as<Shapes::Path>();
    auto geometry = path.Data().as<Media::PathGeometry>();
    path.Data(nullptr);  // Detach the parsed geometry before assigning it elsewhere.
    return geometry;
}

struct NativeRow {
    Controls::Grid grid{nullptr};
    Shapes::Path icon{nullptr};
    Controls::TextBlock value{nullptr};
    Controls::ColumnDefinition iconColumn{nullptr};
    Controls::ColumnDefinition gapColumn{nullptr};
    Controls::ColumnDefinition valueColumn{nullptr};
    std::optional<ResourceKind> kind;
};

struct ShiftedChild {
    FrameworkElement element{nullptr};
    winrt::Windows::Foundation::IInspectable oldLocalValue{nullptr};
    int oldColumn = 0;
    bool changed = false;
};

struct NativePanel {
    const wchar_t* operation = L"Create component";
    Controls::Grid host{nullptr};
    Controls::Grid root{nullptr};
    // Index 0 is tray-side; subsequent two-row columns extend to the left.
    std::vector<Controls::StackPanel> columnPanels = std::vector<Controls::StackPanel>(kMaxColumns, nullptr);
    size_t taskbarRowsPerColumn = 2;
    Controls::ColumnDefinition column{nullptr};
    Controls::ColumnDefinition implicitOriginalColumn{nullptr};
    std::vector<ShiftedChild> shifted;
    std::array<NativeRow, kMaxAlerts> rows{};
    Controls::Flyout overviewFlyout{nullptr};
    Controls::Grid overviewContent{nullptr};
    Controls::Grid overviewTable{nullptr};
    Controls::Border overviewCard{nullptr};
    Controls::Viewbox overviewViewport{nullptr};
    Controls::TextBlock overviewEmpty{nullptr};
    size_t overviewRowsPerGroup = 0;
    size_t overviewGroupCount = 0;
    std::vector<std::array<Controls::TextBlock, 3>> overviewCells;
    std::vector<Controls::Grid> overviewLabelHosts;
    std::vector<Shapes::Path> overviewRowIcons;
    std::vector<Controls::TextBlock> overviewExpanders;
    std::vector<std::wstring> overviewKeys;
    std::set<std::wstring> expandedGpuGroups;
    RuntimeSnapshot latest;
    winrt::event_token tappedToken{};
    DispatcherTimer overviewTimer{nullptr};
    winrt::event_token timerToken{};
    DWORD ownerThread = GetCurrentThreadId();

    NativePanel() = default;
    NativePanel(const NativePanel&) = delete;
    NativePanel& operator=(const NativePanel&) = delete;
    ~NativePanel() { Detach(); }

    bool IsAttached() const {
        if (!host || !root || !column) return false;
        uint32_t index = 0;
        return host.Children().IndexOf(root, index) &&
               host.ColumnDefinitions().IndexOf(column, index) && index == 0;
    }

    static Media::SolidColorBrush TextBrush(Severity severity = Severity::Normal, bool muted = false) {
        const bool light = QueryLightTheme();
        const COLORREF rgb = severity == Severity::Critical
            ? (light ? RGB(180,32,28) : RGB(255,104,96))
            : severity == Severity::Warning ? (light ? RGB(142,88,0) : RGB(255,196,74))
            : muted ? (light ? RGB(100,100,100) : RGB(180,180,180))
            : (light ? RGB(32,32,32) : RGB(240,240,240));
        return Media::SolidColorBrush(winrt::Windows::UI::Color{255, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb)});
    }

    void ToggleGpuGroup(const std::wstring& group) {
        if (group.empty()) return;
        if (!expandedGpuGroups.erase(group)) expandedGpuGroups.insert(group);
        overviewKeys.clear();
        RefreshOverview();
    }

    void RefreshOverview() {
        if (!overviewContent) return;
        const bool light = QueryLightTheme();
        overviewCard.RequestedTheme(light ? ElementTheme::Light : ElementTheme::Dark);
        overviewCard.Background(Media::SolidColorBrush(light
            ? winrt::Windows::UI::Color{255, 248, 248, 248}
            : winrt::Windows::UI::Color{255, 32, 32, 32}));
        overviewCard.BorderBrush(Media::SolidColorBrush(light
            ? winrt::Windows::UI::Color{40, 0, 0, 0}
            : winrt::Windows::UI::Color{48, 255, 255, 255}));
        const bool stale = !latest.tick || GetTickCount64() - latest.tick >
            std::max<ULONGLONG>(5000, 3ULL * g_settings.updateIntervalMs.load());
        std::vector<const OverviewRow*> visible;
        if (!stale) for (const auto& row : latest.overview)
            if (row.available && (row.gpuGroup.empty() || row.gpuSummary ||
                                  expandedGpuGroups.find(row.gpuGroup) != expandedGpuGroups.end())) {
                visible.push_back(&row);
            }
        overviewTable.Visibility(visible.empty() ? Visibility::Collapsed : Visibility::Visible);
        overviewEmpty.Visibility(visible.empty() ? Visibility::Visible : Visibility::Collapsed);
        overviewEmpty.Text(stale ? L"采样已过期，暂无可用读数" : L"暂无可用读数");
        overviewEmpty.Foreground(TextBrush(Severity::Normal, true));
        // Grow naturally; long tables continue in adjacent three-column groups.
        // The vector Viewbox only scales down for an unusually small work area.
        const size_t capacity = static_cast<size_t>(std::max(1.0,
            std::floor(overviewViewport.MaxHeight() / 24.0) - 1.0));
        const size_t groups = std::max(size_t{1}, (visible.size() + capacity - 1) / capacity);
        const size_t rowsPerGroup = (visible.size() + groups - 1) / groups;
        bool rebuild = overviewKeys.size() != visible.size() ||
            overviewRowsPerGroup != rowsPerGroup || overviewGroupCount != groups;
        if (!rebuild) for (size_t i = 0; i < overviewKeys.size(); ++i)
            if (overviewKeys[i] != visible[i]->key) { rebuild = true; break; }
        if (rebuild) {
            overviewTable.Children().Clear();
            overviewTable.RowDefinitions().Clear();
            overviewTable.ColumnDefinitions().Clear();
            overviewCells.clear();
            overviewLabelHosts.clear();
            overviewRowIcons.clear();
            overviewExpanders.clear();
            overviewKeys.clear();
            overviewRowsPerGroup = rowsPerGroup;
            overviewGroupCount = groups;
            overviewTable.ClearValue(FrameworkElement::WidthProperty());
            for (size_t group = 0; group < groups; ++group) {
                if (group) {
                    Controls::ColumnDefinition spacer;
                    spacer.Width(GridLength{16, GridUnitType::Pixel});
                    overviewTable.ColumnDefinitions().Append(spacer);
                }
                for (double autoValue : {1.0, 1.0, 1.0}) {
                    Controls::ColumnDefinition definition;
                    definition.Width(GridLength{autoValue, GridUnitType::Auto});
                    overviewTable.ColumnDefinitions().Append(definition);
                }
            }
            for (size_t i = 0; i <= rowsPerGroup; ++i) {
                Controls::RowDefinition definition;
                definition.Height(GridLength{1, GridUnitType::Auto});
                overviewTable.RowDefinitions().Append(definition);
            }
            const std::array<const wchar_t*, 3> headers{L"参数", L"当前值", L"近 15 min 最值"};
            const auto makeCells = [&](size_t rowIndex, size_t group,
                                       const OverviewRow* data = nullptr) {
                std::array<Controls::TextBlock, 3> cells;
                for (size_t j = 0; j < cells.size(); ++j) {
                    auto& text = cells[j];
                    text.FontSize(12);
                    text.IsTextScaleFactorEnabled(false);
                    text.TextWrapping(TextWrapping::NoWrap);
                    text.TextTrimming(TextTrimming::None);
                    text.VerticalAlignment(VerticalAlignment::Center);
                    text.Margin(Thickness{0, 4, j + 1 == cells.size() ? 0.0 : 8.0, 4});
                    text.Foreground(TextBrush());
                    if (j || !data) {
                        Controls::Grid::SetRow(text, static_cast<int>(rowIndex));
                        Controls::Grid::SetColumn(text, static_cast<int>(group * 4 + j));
                        overviewTable.Children().Append(text);
                    }
                    if (rowIndex == 0) {
                        text.Text(headers[j]);
                        text.FontWeight(winrt::Windows::UI::Text::FontWeight{600});
                    }
                }
                if (data) {
                    Controls::Grid labelHost;
                    labelHost.VerticalAlignment(VerticalAlignment::Center);
                    labelHost.Margin(Thickness{0, 0, 8, 0});
                    Controls::ColumnDefinition iconColumn;
                    iconColumn.Width(GridLength{data->icon ? 14.0 : 0.0, GridUnitType::Pixel});
                    Controls::ColumnDefinition gapColumn;
                    gapColumn.Width(GridLength{data->icon ? 4.0 : 0.0, GridUnitType::Pixel});
                    Controls::ColumnDefinition textColumn;
                    textColumn.Width(GridLength{1, GridUnitType::Auto});
                    Controls::ColumnDefinition expanderColumn;
                    expanderColumn.Width(GridLength{data->gpuSummary ? 12.0 : 0.0, GridUnitType::Pixel});
                    Controls::ColumnDefinition trailingColumn;
                    trailingColumn.Width(GridLength{1, GridUnitType::Star});
                    labelHost.ColumnDefinitions().Append(iconColumn);
                    labelHost.ColumnDefinitions().Append(gapColumn);
                    labelHost.ColumnDefinitions().Append(textColumn);
                    labelHost.ColumnDefinitions().Append(expanderColumn);
                    labelHost.ColumnDefinitions().Append(trailingColumn);

                    Controls::TextBlock expander;
                    expander.FontFamily(Media::FontFamily(L"Segoe UI Symbol"));
                    expander.FontSize(10);
                    expander.IsTextScaleFactorEnabled(false);
                    expander.VerticalAlignment(VerticalAlignment::Center);
                    expander.Text(data->gpuSummary
                        ? (expandedGpuGroups.find(data->gpuGroup) != expandedGpuGroups.end()
                            ? L"\u25BE" : L"\u25B8") : L"");
                    Controls::Grid::SetColumn(expander, 3);
                    labelHost.Children().Append(expander);

                    Shapes::Path icon;
                    icon.Width(13);
                    icon.Height(13);
                    icon.Stretch(Media::Stretch::Uniform);
                    icon.VerticalAlignment(VerticalAlignment::Center);
                    if (data->icon) icon.Data(BuildIconGeometry(*data->icon));
                    else icon.Visibility(Visibility::Collapsed);
                    Controls::Grid::SetColumn(icon, 0);
                    labelHost.Children().Append(icon);

                    cells[0].Margin(Thickness{0, 4, data->gpuSummary ? 2.0 : 0.0, 4});
                    Controls::Grid::SetColumn(cells[0], 2);
                    labelHost.Children().Append(cells[0]);
                    Controls::Grid::SetRow(labelHost, static_cast<int>(rowIndex));
                    Controls::Grid::SetColumn(labelHost, static_cast<int>(group * 4));
                    if (data->gpuSummary) {
                        const auto gpuGroup = data->gpuGroup;
                        labelHost.Tapped([this, gpuGroup](auto&&, auto&&) { ToggleGpuGroup(gpuGroup); });
                    }
                    overviewTable.Children().Append(labelHost);
                    overviewLabelHosts.push_back(labelHost);
                    overviewRowIcons.push_back(icon);
                    overviewExpanders.push_back(expander);
                }
                return cells;
            };
            for (size_t group = 0; group < groups; ++group) makeCells(0, group);
            for (size_t i = 0; i < visible.size(); ++i) {
                overviewCells.push_back(makeCells(i % rowsPerGroup + 1, i / rowsPerGroup, visible[i]));
                overviewKeys.push_back(visible[i]->key);
            }
        }
        for (size_t i = 0; i < visible.size(); ++i) {
            const auto& row = *visible[i];
            auto& cells = overviewCells[i];
            cells[0].Text(row.label);
            cells[1].Text(row.value);
            cells[2].Text(row.peak);
            Controls::ToolTipService::SetToolTip(cells[1], winrt::box_value(row.state));
            Automation::AutomationProperties::SetHelpText(cells[1], row.state);
            const auto extremeHelp = row.minimum ? L"近 15 分钟最低值" : L"近 15 分钟最高值";
            Controls::ToolTipService::SetToolTip(cells[2], winrt::box_value(extremeHelp));
            Automation::AutomationProperties::SetHelpText(cells[2], extremeHelp);
            Controls::ToolTipService::SetToolTip(cells[0], winrt::box_value(
                row.label + L"\n" + latest.devices +
                L"\n最值统计最近 15 分钟；运行不足 15 分钟时按已有有效样本统计。"));
            for (size_t j = 0; j < cells.size(); ++j)
                cells[j].Foreground(TextBrush(j < 2 ? row.severity : Severity::Normal));
            overviewExpanders[i].Foreground(TextBrush(row.severity));
            overviewRowIcons[i].Fill(TextBrush(row.severity));
        }
    }

    void EnsureOverview() {
        if (overviewFlyout) return;
        overviewFlyout = Controls::Flyout();
        overviewFlyout.Placement(Controls::Primitives::FlyoutPlacementMode::TopEdgeAlignedRight);
        overviewFlyout.ShouldConstrainToRootBounds(false);
        overviewFlyout.LightDismissOverlayMode(Controls::LightDismissOverlayMode::Off);
        overviewContent = Controls::Grid();
        overviewTable = Controls::Grid();
        overviewContent.Children().Append(overviewTable);
        overviewEmpty = Controls::TextBlock();
        overviewEmpty.FontSize(12);
        overviewEmpty.Visibility(Visibility::Collapsed);
        overviewContent.Children().Append(overviewEmpty);
        overviewViewport = Controls::Viewbox();
        overviewViewport.Stretch(Media::Stretch::Uniform);
        overviewViewport.StretchDirection(Controls::StretchDirection::DownOnly);
        overviewViewport.Child(overviewContent);
        overviewCard = Controls::Border();
        overviewCard.CornerRadius(CornerRadius{10, 10, 10, 10});
        overviewCard.Padding(Thickness{12, 10, 12, 10});
        overviewCard.BorderThickness(Thickness{1, 1, 1, 1});
        overviewCard.Child(overviewViewport);
        // Use the monitor work area, not the taskbar's ~48-DIP XamlRoot height.
        double width = 1200, height = 760;
        if (HWND taskbar = g_taskbarWindow.load()) {
            MONITORINFO monitor{sizeof(monitor)};
            const UINT dpi = GetDpiForWindow(taskbar);
            if (dpi && GetMonitorInfoW(MonitorFromWindow(taskbar, MONITOR_DEFAULTTONEAREST), &monitor)) {
                width = (monitor.rcWork.right - monitor.rcWork.left) * 96.0 / dpi - 64;
                height = (monitor.rcWork.bottom - monitor.rcWork.top) * 96.0 / dpi - 64;
            }
        }
        overviewViewport.MaxWidth(std::max(160.0, width));
        overviewViewport.MaxHeight(std::max(120.0, height));
        // Own the rounded surface explicitly. A custom presenter style must not
        // depend on an implicit theme style supplying the corner radius.
        Style presenter(winrt::xaml_typename<Controls::FlyoutPresenter>());
        const auto set = [&](const DependencyProperty& property, const auto& value) {
            Setter setter;
            setter.Property(property);
            setter.Value(winrt::box_value(value));
            presenter.Setters().Append(setter);
        };
        set(FrameworkElement::MinWidthProperty(), 0.0);
        set(FrameworkElement::MaxWidthProperty(), overviewViewport.MaxWidth() + 32.0);
        set(FrameworkElement::MaxHeightProperty(), overviewViewport.MaxHeight() + 32.0);
        set(Controls::Control::PaddingProperty(), Thickness{});
        set(Controls::Control::BorderThicknessProperty(), Thickness{});
        set(Controls::Control::CornerRadiusProperty(), CornerRadius{10, 10, 10, 10});
        set(Controls::Control::BackgroundProperty(), Media::SolidColorBrush(winrt::Windows::UI::Color{}));
        // The system FlyoutPresenter template can paint an opaque background
        // behind Content, filling the card's rounded corners. Keep the native
        // flyout behavior but let our Border be the only painted surface.
        set(Controls::Control::TemplateProperty(), Markup::XamlReader::Load(LR"XAML(
            <ControlTemplate xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
                             TargetType="FlyoutPresenter">
                <ContentPresenter Content="{TemplateBinding Content}"
                                  HorizontalContentAlignment="Stretch"
                                  VerticalContentAlignment="Stretch" />
            </ControlTemplate>)XAML").as<Controls::ControlTemplate>());
        overviewFlyout.FlyoutPresenterStyle(presenter);
        overviewFlyout.Content(overviewCard);
        overviewTimer = DispatcherTimer();
        overviewTimer.Interval(std::chrono::seconds(1));
        timerToken = overviewTimer.Tick([this](auto&&, auto&&) {
            try {
                if (overviewFlyout && overviewFlyout.IsOpen()) RefreshOverview();
                else overviewTimer.Stop();
            } catch (...) { overviewTimer.Stop(); }
        });
    }

    void ToggleOverview() {
        EnsureOverview();
        if (overviewFlyout.IsOpen()) { overviewFlyout.Hide(); return; }
        RefreshOverview();
        overviewFlyout.ShowAt(root);
        overviewTimer.Start();
    }

    void ShiftNewChildren() {
        auto children = host.Children();
        for (const auto& childElement : children) {
            auto child = childElement.as<FrameworkElement>();
            if (child == root) continue;
            if (std::any_of(shifted.begin(), shifted.end(),
                            [&](const auto& old) { return old.element == child; })) {
                continue;
            }
            const int oldColumn = Controls::Grid::GetColumn(child);
            if (oldColumn < 0 || oldColumn > 128) {
                throw winrt::hresult_error(E_UNEXPECTED, L"Unsupported tray column");
            }
            // Avoid replacing a live binding with a literal integer.
            operation = L"Check native column binding";
            auto binding = child.GetBindingExpression(Controls::Grid::ColumnProperty());
            if (binding) {
                throw winrt::hresult_error(E_UNEXPECTED, L"Bound tray column is unsupported");
            }
            operation = L"Save native column value";
            shifted.push_back({child,
                child.ReadLocalValue(Controls::Grid::ColumnProperty()),
                oldColumn, false});
            operation = L"Shift native column";
            Controls::Grid::SetColumn(child, oldColumn + 1);
            shifted.back().changed = true;
        }
    }

    void Attach(Controls::Grid grid) {
        host = grid;
        root = Controls::Grid();
        root.Name(kComponentName);
        // Fill the taskbar cell so FlyoutPlacementMode::TopEdgeAlignedRight
        // always anchors to the taskbar's top edge. The visible alert stacks
        // stay centered independently below.
        root.VerticalAlignment(VerticalAlignment::Stretch);
        root.HorizontalAlignment(HorizontalAlignment::Right);
        // Alpha-zero fill creates a complete click target without a visible box.
        root.Background(Media::SolidColorBrush(winrt::Windows::UI::Color{0,0,0,0}));
        root.IsHitTestVisible(true);
        root.UseLayoutRounding(true);
        root.Visibility(Visibility::Collapsed);
        root.FlowDirection(FlowDirection::LeftToRight);
        Automation::AutomationProperties::SetName(root, L"System resource alerts");
        Controls::ToolTipService::SetToolTip(root, winrt::box_value(L"点击查看系统资源总览"));
        tappedToken = root.Tapped([this](auto&&, Input::TappedRoutedEventArgs const& args) {
            args.Handled(true);
            try { ToggleOverview(); }
            catch (...) { Wh_Log(L"Could not open the native resource overview"); }
        });

        for (size_t index = 0; index < columnPanels.size(); ++index) {
            Controls::ColumnDefinition nativeColumn;
            nativeColumn.Width(GridLength{1, GridUnitType::Auto});
            root.ColumnDefinitions().Append(nativeColumn);
            auto& stack = columnPanels[index];
            stack = Controls::StackPanel();
            stack.Orientation(Controls::Orientation::Vertical);
            stack.VerticalAlignment(VerticalAlignment::Center);
            Controls::Grid::SetColumn(stack, static_cast<int>(kMaxColumns - 1 - index));
            root.Children().Append(stack);
        }

        for (size_t index = 0; index < rows.size(); ++index) {
            auto& row = rows[index];
            operation = L"Create row columns";
            row.grid = Controls::Grid();
            row.grid.Background(nullptr);
            // A flexible leading column right-aligns the whole icon/value
            // group. The shared value width keeps all icons vertically aligned.
            Controls::ColumnDefinition leading;
            leading.Width(GridLength{1, GridUnitType::Star});
            row.iconColumn = Controls::ColumnDefinition();
            row.iconColumn.Width(GridLength{15, GridUnitType::Pixel});
            row.gapColumn = Controls::ColumnDefinition();
            row.gapColumn.Width(GridLength{static_cast<double>(g_settings.iconValueGap.load()), GridUnitType::Pixel});
            row.valueColumn = Controls::ColumnDefinition();
            row.valueColumn.Width(GridLength{1, GridUnitType::Auto});
            row.grid.ColumnDefinitions().Append(leading);
            row.grid.ColumnDefinitions().Append(row.iconColumn);
            row.grid.ColumnDefinitions().Append(row.gapColumn);
            row.grid.ColumnDefinitions().Append(row.valueColumn);

            operation = L"Create vector icon";
            row.icon = Shapes::Path();
            row.icon.Stretch(Media::Stretch::Uniform);
            row.icon.Stroke(nullptr);
            row.icon.VerticalAlignment(VerticalAlignment::Center);
            Controls::Grid::SetColumn(row.icon, 1);

            operation = L"Create native text";
            row.value = Controls::TextBlock();
            row.value.FontFamily(Media::FontFamily(L"Segoe UI"));
            row.value.FontWeight(winrt::Windows::UI::Text::FontWeight{600});
            row.value.TextWrapping(TextWrapping::NoWrap);
            row.value.TextTrimming(TextTrimming::None);
            row.value.IsTextScaleFactorEnabled(false);
            row.value.VerticalAlignment(VerticalAlignment::Center);
            // Numeric glyphs have no descenders. Center their cap-height-to-
            // baseline box instead of a font line box with empty descender space.
            row.value.TextLineBounds(TextLineBounds::Tight);
            row.value.LineStackingStrategy(LineStackingStrategy::MaxHeight);
            Controls::Grid::SetColumn(row.value, 3);
            row.grid.Children().Append(row.icon);
            row.grid.Children().Append(row.value);
            columnPanels[index / taskbarRowsPerColumn].Children().Append(row.grid);
        }

        operation = L"Insert native column";
        auto columns = host.ColumnDefinitions();
        if (columns.Size() > 32) {
            throw winrt::hresult_error(E_UNEXPECTED, L"Unsupported tray grid");
        }
        if (columns.Size() == 0) {
            // Preserve the grid's original implicit star column.
            for (const auto& childElement : host.Children()) {
                auto child = childElement.as<FrameworkElement>();
                if (Controls::Grid::GetColumn(child) != 0 ||
                    Controls::Grid::GetColumnSpan(child) != 1) {
                    throw winrt::hresult_error(E_UNEXPECTED, L"Unsupported implicit grid");
                }
            }
            implicitOriginalColumn = Controls::ColumnDefinition();
            columns.Append(implicitOriginalColumn);
        }
        column = Controls::ColumnDefinition();
        column.Width(GridLength{1, GridUnitType::Auto});
        columns.InsertAt(0, column);
        ShiftNewChildren();
        Controls::Grid::SetColumn(root, 0);
        Controls::Grid::SetRowSpan(root,
            std::max(1, static_cast<int>(host.RowDefinitions().Size())));
        operation = L"Insert native element";
        host.Children().InsertAt(0, root);
        operation = L"Attached";
    }

    void Detach() noexcept {
        if (!host) return;
        // Callers keep XAML creation, modification, and final release on the
        // taskbar thread. Revoke callbacks before releasing any captured state.
        try {
            if (root && tappedToken.value) { root.Tapped(tappedToken); tappedToken = {}; }
            if (overviewTimer) {
                overviewTimer.Stop();
                if (timerToken.value) overviewTimer.Tick(timerToken);
                timerToken = {};
                overviewTimer = nullptr;
            }
            if (overviewFlyout) {
                overviewFlyout.Hide();
                overviewFlyout.Content(nullptr);
                overviewFlyout = nullptr;
                overviewCard = nullptr;
                overviewViewport = nullptr;
            }
            uint32_t index = 0;
            if (root && host.Children().IndexOf(root, index)) {
                host.Children().RemoveAt(index);
            }
        } catch (...) {
            Wh_Log(L"Could not remove the native alert element");
        }
        for (auto& child : shifted) {
            try {
                uint32_t index = 0;
                if (!child.changed || !host.Children().IndexOf(child.element, index) ||
                    Controls::Grid::GetColumn(child.element) != child.oldColumn + 1) {
                    continue;  // Don't overwrite another mod's later edit.
                }
                if (child.oldLocalValue == DependencyProperty::UnsetValue()) {
                    child.element.ClearValue(Controls::Grid::ColumnProperty());
                } else {
                    child.element.SetValue(Controls::Grid::ColumnProperty(),
                                           child.oldLocalValue);
                }
            } catch (...) {
                Wh_Log(L"Could not restore a native tray column assignment");
            }
        }
        try {
            uint32_t index = 0;
            if (column && host.ColumnDefinitions().IndexOf(column, index)) {
                host.ColumnDefinitions().RemoveAt(index);
            }
            // Only remove the synthetic implicit column if it is still the
            // sole column; leave unrelated layout edits untouched.
            if (implicitOriginalColumn && host.ColumnDefinitions().Size() == 1 &&
                host.ColumnDefinitions().GetAt(0) == implicitOriginalColumn) {
                host.ColumnDefinitions().Clear();
            }
        } catch (...) {
            Wh_Log(L"Could not remove the native alert grid column");
        }
        shifted.clear();
        host = nullptr;
    }

    void Update(const std::vector<AlertItem>& alertItems, double taskbarHeight) {
        operation = L"Update visibility";
        const auto& items = alertItems;
        if (items.empty()) {
            if (overviewFlyout && overviewFlyout.IsOpen()) overviewFlyout.Hide();
            if (overviewTimer) overviewTimer.Stop();
            root.Visibility(Visibility::Collapsed);
            return;
        }
        if (overviewFlyout && overviewFlyout.IsOpen()) RefreshOverview();
        const auto metrics = CalculateNativeRows(items.size(), taskbarHeight);
        if (!metrics) {
            root.Visibility(Visibility::Collapsed);
            return;
        }
        operation = L"Pin flyout anchor to taskbar height";
        root.MinHeight(taskbarHeight);
        ShiftNewChildren();
        const size_t rowsPerColumn = AlertRowsPerColumn(items.size());
        if (taskbarRowsPerColumn != rowsPerColumn) {
            operation = L"Reflow alert rows";
            for (auto& panel : columnPanels) panel.Children().Clear();
            taskbarRowsPerColumn = rowsPerColumn;
            for (size_t index = 0; index < rows.size(); ++index)
                columnPanels[index / taskbarRowsPerColumn].Children().Append(rows[index].grid);
        }
        // Measure as a visible subtree. A child of a collapsed ancestor may
        // report zero desired width even when Measure is called explicitly.
        root.Visibility(Visibility::Visible);
        for (size_t i = 0; i < columnPanels.size(); ++i) {
            columnPanels[i].Visibility(i * rowsPerColumn < items.size() ? Visibility::Visible : Visibility::Collapsed);
            columnPanels[i].Margin(Thickness{0, 0, i > 0 ? static_cast<double>(g_settings.alertColumnGap.load()) : 0, 0});
        }
        double valueWidth = 0;
        for (size_t index = 0; index < rows.size(); ++index) {
            auto& row = rows[index];
            if (index >= items.size()) {
                row.grid.Visibility(Visibility::Collapsed);
                continue;
            }
            const auto& item = items[index];
            operation = L"Update row dimensions";
            row.grid.Visibility(Visibility::Visible);
            row.grid.Height(metrics->height);
            row.grid.Margin(Thickness{0, 0, 0,
                index % rowsPerColumn + 1 < rowsPerColumn && index + 1 < items.size()
                    ? metrics->gap : 0});
            row.icon.Width(metrics->iconSize);
            row.icon.Height(metrics->iconSize);
            row.iconColumn.Width(GridLength{metrics->iconSize, GridUnitType::Pixel});
            const double itemScale = rowsPerColumn == 3 ? 0.84 : 1.0;
            row.gapColumn.Width(GridLength{std::floor(g_settings.iconValueGap.load() * itemScale), GridUnitType::Pixel});
            operation = L"Set vector icon";
            if (row.kind != item.kind) {
                // Geometry cannot be shared/reparented between live Path controls
                // on this XAML runtime. Own one per row, including duplicate GPUs.
                row.icon.Data(BuildIconGeometry(item.kind));
                row.kind = item.kind;
            }

            auto brush = TextBrush(item.severity);
            operation = L"Update native text";
            row.icon.Fill(brush);
            row.value.Foreground(brush);
            row.value.FontSize(metrics->fontSize);
            row.value.Text(item.value);
            row.grid.MinWidth(std::ceil(g_settings.itemWidth.load() * itemScale));
            if (!item.label.empty()) Controls::ToolTipService::SetToolTip(row.grid,
                winrt::box_value(item.label + L" · " + item.value + L"\n点击查看总表"));
            else row.grid.ClearValue(Controls::ToolTipService::ToolTipProperty());
            row.valueColumn.Width(GridLength{1, GridUnitType::Auto});
        }
        operation = L"Measure native text";
        root.Measure(winrt::Windows::Foundation::Size{
            std::numeric_limits<float>::infinity(), static_cast<float>(taskbarHeight)});
        for (size_t index = 0; index < items.size(); ++index) {
            valueWidth = std::max<double>(valueWidth, rows[index].value.DesiredSize().Width);
        }
        operation = L"Align native text columns";
        const double itemScale = rowsPerColumn == 3 ? 0.84 : 1.0;
        for (auto& row : rows) {
            row.valueColumn.Width(GridLength{std::ceil(valueWidth) + 1, GridUnitType::Pixel});
            row.grid.MinWidth(std::ceil(g_settings.itemWidth.load() * itemScale));
        }
        root.Margin(Thickness{2, 0, static_cast<double>(g_settings.componentGap.load()), 0});
        root.Visibility(Visibility::Visible);
        operation = L"Updated";
    }
};

// Accessors are resolved by exact PDB symbols; none of these functions are hooked.
HMODULE g_taskbarModule = nullptr;
void* g_taskBandVtable = nullptr;
using GetTaskbarHost_t = void*(WINAPI*)(void*, void**);
GetTaskbarHost_t g_getTaskbarHost = nullptr;
void* g_frameHeight = nullptr;
using ReleaseSharedHost_t = void(WINAPI*)(void*);
ReleaseSharedHost_t g_releaseSharedHost = nullptr;
size_t g_hostElementOffset = 0;
std::unique_ptr<NativePanel> g_nativePanel;  // Accessed/released only on the UI thread.

template <typename T>
bool ReadLocalMemory(const void* address, T* value) {
    SIZE_T read = 0;
    return address && ReadProcessMemory(GetCurrentProcess(), address, value,
                                        sizeof(T), &read) && read == sizeof(T);
}

std::optional<size_t> DecodeHostElementOffset(const std::array<BYTE, 8>& code) {
    // Verified x64 TaskbarHost::FrameHeight prologue:
    // sub rsp, imm8 ; add rcx, elementOffset.
    if (code[0] != 0x48 || code[1] != 0x83 || code[2] != 0xEC ||
        code[4] != 0x48 || code[5] != 0x83 || code[6] != 0xC1 ||
        code[7] < sizeof(void*) || code[7] > 0x78 ||
        code[7] % alignof(void*) != 0) {
        return std::nullopt;
    }
    return code[7];
}

bool ResolveTaskbarAccessors() {
    g_taskbarModule = LoadLibraryExW(L"taskbar.dll", nullptr,
                                     LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!g_taskbarModule) return false;
    WindhawkUtils::SYMBOL_HOOK symbols[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
            &g_taskBandVtable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
            &g_getTaskbarHost},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
            &g_frameHeight},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
            &g_releaseSharedHost},
    };
    if (!WindhawkUtils::HookSymbols(g_taskbarModule, symbols, ARRAYSIZE(symbols))) {
        Wh_Log(L"Native taskbar accessors are unavailable on this Windows build");
        return false;
    }
    std::array<BYTE, 8> bytes{};
    if (!ReadLocalMemory(g_frameHeight, &bytes)) return false;
    const auto offset = DecodeHostElementOffset(bytes);
    if (!offset) {
        Wh_Log(L"Unrecognized TaskbarHost::FrameHeight layout; refusing guessed offsets");
        return false;
    }
    g_hostElementOffset = *offset;
    return true;
}

XamlRoot GetPrimaryTaskbarXamlRoot(HWND taskbar) {
    HWND taskband = reinterpret_cast<HWND>(GetPropW(taskbar, L"TaskbandHWND"));
    if (!taskband) return nullptr;
    const auto band = reinterpret_cast<const BYTE*>(GetWindowLongPtrW(taskband, 0));
    if (!band) return nullptr;
    void* site = nullptr;
    for (int index = 0; index <= 20; ++index) {
        void* vtable = nullptr;
        const auto slot = band + index * sizeof(void*);
        if (!ReadLocalMemory(slot, &vtable)) return nullptr;
        if (vtable == g_taskBandVtable) {
            site = const_cast<BYTE*>(slot);
            break;
        }
    }
    if (!site) return nullptr;
    struct SharedHost {
        void* parts[2]{};
        ~SharedHost() {
            if (parts[1]) g_releaseSharedHost(parts[1]);
        }
    } sharedHost;
    g_getTaskbarHost(site, sharedHost.parts);
    if (!sharedHost.parts[0] || !sharedHost.parts[1]) return nullptr;
    IUnknown* unknown = nullptr;
    if (!ReadLocalMemory(static_cast<const BYTE*>(sharedHost.parts[0]) +
                          g_hostElementOffset, &unknown) || !unknown) {
        return nullptr;
    }
    FrameworkElement frame{nullptr};
    if (FAILED(unknown->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                      winrt::put_abi(frame)))) {
        return nullptr;
    }
    return frame.XamlRoot();
}

Controls::Grid FindTrayGrid(DependencyObject node, int depth, int* visited) {
    if (!node || depth > 14 || ++*visited > 512) return nullptr;
    auto element = node.try_as<FrameworkElement>();
    if (element && element.Name() == L"SystemTrayFrameGrid") {
        return element.try_as<Controls::Grid>();
    }
    const int count = Media::VisualTreeHelper::GetChildrenCount(node);
    for (int index = 0; index < count; ++index) {
        auto found = FindTrayGrid(Media::VisualTreeHelper::GetChild(node, index),
                                  depth + 1, visited);
        if (found) return found;
    }
    return nullptr;
}

HWND FindPrimaryTaskbar() {
    HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD processId = 0;
    if (!taskbar || !GetWindowThreadProcessId(taskbar, &processId) ||
        processId != GetCurrentProcessId()) return nullptr;
    return taskbar;
}

void ApplyNativePanel(HWND taskbar) noexcept {
    try {
        if (g_unloading) return;
        RuntimeSnapshot snapshot;
        {
            std::lock_guard guard(g_snapshotMutex);
            snapshot = g_snapshot;
        }
        if (g_nativePanel && !g_nativePanel->IsAttached()) g_nativePanel.reset();
        if (!g_nativePanel) {
            if (snapshot.alerts.empty()) return;
            auto xamlRoot = GetPrimaryTaskbarXamlRoot(taskbar);
            if (!xamlRoot) {
                Wh_Log(L"Taskbar XAML root is not ready or uses an unsupported runtime");
                return;
            }
            int visited = 0;
            auto grid = FindTrayGrid(xamlRoot.Content(), 0, &visited);
            if (!grid) {
                Wh_Log(L"SystemTrayFrameGrid was not found; no overlay fallback");
                return;
            }
            auto panel = std::make_unique<NativePanel>();
            panel->Attach(grid);
            g_nativePanel = std::move(panel);
            Wh_Log(L"Native resource alert column attached to SystemTrayFrameGrid");
        }
        RECT rectangle{};
        if (!GetWindowRect(taskbar, &rectangle)) return;
        const UINT dpi = GetDpiForWindow(taskbar);
        if (!dpi) return;
        const double height = (rectangle.bottom - rectangle.top) * 96.0 / dpi;
        g_nativePanel->latest = std::move(snapshot);
        g_nativePanel->Update(g_nativePanel->latest.alerts, height);
    } catch (const winrt::hresult_error& error) {
        Wh_Log(L"Native alert XAML error 0x%08X: %s", error.code().value,
               error.message().c_str());
        g_nativePanel.reset();
    } catch (...) {
        Wh_Log(L"Native alert update failed; removing this mod's controls");
        g_nativePanel.reset();
    }
}

enum class UiCommand : WPARAM { Attach = 1, Detach = 2 };
LRESULT CALLBACK TaskbarSubclass(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);

void DetachOnTaskbarThread(HWND taskbar) noexcept {
    g_nativePanel.reset();
    RemoveWindowSubclass(taskbar, TaskbarSubclass,
                          reinterpret_cast<UINT_PTR>(TaskbarSubclass));
    HWND expected = taskbar;
    g_taskbarWindow.compare_exchange_strong(expected, nullptr);
    g_updatePending = false;
}

void HandleUiCommand(HWND taskbar, UiCommand command) noexcept {
    if (command == UiCommand::Detach || g_unloading) {
        DetachOnTaskbarThread(taskbar);
        return;
    }
    if (!SetWindowSubclass(taskbar, TaskbarSubclass,
                            reinterpret_cast<UINT_PTR>(TaskbarSubclass), 0)) return;
    g_taskbarWindow = taskbar;
    ApplyNativePanel(taskbar);
}

LRESULT CALLBACK DispatchHook(int code, WPARAM wParam, LPARAM lParam) {
    struct CallbackGuard {
        CallbackGuard() { ++g_uiCallbacks; }
        ~CallbackGuard() { --g_uiCallbacks; }
    } callbackGuard;
    if (code == HC_ACTION) {
        const auto* message = reinterpret_cast<const CWPSTRUCT*>(lParam);
        if (message->message == g_dispatchMessage &&
            (message->wParam == static_cast<WPARAM>(UiCommand::Attach) ||
             message->wParam == static_cast<WPARAM>(UiCommand::Detach))) {
            // No stack pointer is carried in this message, even on timeout.
            HandleUiCommand(message->hwnd, static_cast<UiCommand>(message->wParam));
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

bool DispatchToTaskbar(HWND taskbar, UiCommand command) {
    DWORD processId = 0;
    DWORD threadId = GetWindowThreadProcessId(taskbar, &processId);
    if (!threadId || processId != GetCurrentProcessId()) return false;
    if (threadId == GetCurrentThreadId()) {
        HandleUiCommand(taskbar, command);
        return true;
    }
    HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, DispatchHook, nullptr, threadId);
    if (!hook) return false;
    const LRESULT result = SendMessageTimeoutW(
        taskbar, g_dispatchMessage, static_cast<WPARAM>(command), 0,
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, nullptr);
    UnhookWindowsHookEx(hook);
    return result != 0;
}

LRESULT CALLBACK TaskbarSubclass(HWND window, UINT message, WPARAM wParam,
                                 LPARAM lParam, UINT_PTR subclassId,
                                 DWORD_PTR referenceData) {
    if (message == WM_NCDESTROY) {
        DetachOnTaskbarThread(window);
    } else if (message == g_updateMessage) {
        g_updatePending = false;
        if (g_unloading) DetachOnTaskbarThread(window);
        else ApplyNativePanel(window);
        return 0;
    }
    return DefSubclassProc(window, message, wParam, lParam);
}

void UpdateAlertState(GpuCollector& gpuCollector, DiskIoCollector& diskCollector) {
    ResourceSample sample{};
    const int sustain = g_settings.warningSustainSeconds.load();
    const bool coreValid = QueryResourceSample(&sample);
    diskCollector.Sample(&sample);
    sample.gpus = gpuCollector.Sample();
    if (g_settings.enableCpuTemperature && g_cpuTemperatureTick &&
        GetTickCount64() - g_cpuTemperatureTick.load() <= 6000) {
        sample.cpuTemperature = g_cpuTemperature;
        sample.cpuTemperatureSource = g_cpuTemperatureSource;
    }
    const bool gpuFailure = sample.gpus.empty() || std::all_of(sample.gpus.begin(), sample.gpus.end(),
        [](const auto& gpu) { return gpu.load < 0 && gpu.dedicatedUsedGB < 0 && gpu.temperature < 0; });
    UpdateTracker(&Tracker(ResourceKind::Monitor),
        !coreValid || gpuFailure ? Severity::Warning : Severity::Normal, sustain);
    if (coreValid) {
        UpdateAccumulatingWarning(&Tracker(ResourceKind::Cpu), sample.cpuPct,
            CpuSeverity(sample), g_settings.cpuWarningSeconds.load(),
            g_settings.highUsageGapSeconds.load());
        UpdateTracker(&Tracker(ResourceKind::Memory), MemorySeverity(sample), sustain);
        UpdateTracker(&Tracker(ResourceKind::Commit), CommitSeverity(sample), sustain);
        UpdateTracker(&Tracker(ResourceKind::Disk), DiskSeverity(sample), sustain);
        UpdateAccumulatingWarning(&Tracker(ResourceKind::DiskActivity), sample.diskActivityPct,
            DiskActivitySeverity(sample), g_settings.diskActivityWarningSeconds.load(),
            g_settings.highUsageGapSeconds.load());
        UpdateAccumulatingWarning(&Tracker(ResourceKind::DiskWrite), sample.diskWriteBps,
            DiskWriteSeverity(sample), g_settings.diskWriteWarningSeconds.load(),
            g_settings.diskWriteGapSeconds.load());
    }
    UpdateKnownTracker(&Tracker(ResourceKind::CpuTemperature), sample.cpuTemperature,
        ThresholdSeverity(sample.cpuTemperature, g_settings.cpuTemperatureWarning, g_settings.cpuTemperatureCritical), sustain);
    for (const auto& gpu : sample.gpus) {
        auto& trackers = g_gpuTrackers[gpu.id];
        UpdateKnownTracker(&trackers[0], gpu.load,
            ThresholdSeverity(gpu.load, g_settings.gpuWarningPct, 101), g_settings.gpuWarningSeconds);
        UpdateKnownTracker(&trackers[1], gpu.VramPct(), VramSeverity(gpu), sustain);
        UpdateKnownTracker(&trackers[2], gpu.temperature,
            ThresholdSeverity(gpu.temperature, g_settings.gpuTemperatureWarning, g_settings.gpuTemperatureCritical), sustain);
    }
    g_lastSample = sample;
}

DWORD WINAPI MonitorThread(void*) {
    const HRESULT comInitialized = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HANDLE waits[] = {g_stopEvent, g_wakeEvent};
    std::unique_ptr<GpuCollector> gpuCollector;
    std::unique_ptr<DiskIoCollector> diskCollector;
    while (!g_unloading) {
        try {
            if (!gpuCollector) gpuCollector = std::make_unique<GpuCollector>();
            if (!diskCollector) diskCollector = std::make_unique<DiskIoCollector>();
            if (g_resetRequested.exchange(false)) ResetTrackers();
            const ULONGLONG cpuStarted = g_cpuQueryStarted.load();
            if (cpuStarted && GetTickCount64() - cpuStarted > 1500 && g_cpuThreadId)
                CoCancelCall(g_cpuThreadId, 0);
            UpdateAlertState(*gpuCollector, *diskCollector);
            auto snapshot = BuildSnapshot();
            {
                std::lock_guard guard(g_snapshotMutex);
                g_snapshot = std::move(snapshot);
            }
            HWND taskbar = FindPrimaryTaskbar();
            if (taskbar) {
                if (g_taskbarWindow.load() != taskbar) {
                    DispatchToTaskbar(taskbar, UiCommand::Attach);
                }
                if (g_taskbarWindow.load() == taskbar && !g_updatePending.exchange(true)) {
                    if (!PostMessageW(taskbar, g_updateMessage, 0, 0)) {
                        g_updatePending = false;
                    }
                }
            }
        } catch (...) {
            // Resource pressure must not let a C++ exception escape the
            // monitor thread into Explorer's process-wide exception handler.
            Wh_Log(L"Resource sampling/update failed; retaining the last snapshot");
        }
        if (WaitForMultipleObjects(ARRAYSIZE(waits), waits, FALSE,
                                    g_settings.updateIntervalMs.load()) == WAIT_OBJECT_0) {
            break;
        }
    }
    gpuCollector.reset();
    diskCollector.reset();
    if (SUCCEEDED(comInitialized)) CoUninitialize();
    return 0;
}

void ReleaseWorkerHandles() {
    if (g_cpuThread) CloseHandle(g_cpuThread);
    if (g_thread) CloseHandle(g_thread);
    if (g_stopEvent) CloseHandle(g_stopEvent);
    if (g_wakeEvent) CloseHandle(g_wakeEvent);
    g_thread = nullptr;
    g_cpuThread = nullptr;
    g_cpuThreadId = 0;
    g_stopEvent = nullptr;
    g_wakeEvent = nullptr;
}

}  // namespace

BOOL Wh_ModInit() {
    LoadSettings();
    ResetTrackers();
    g_extrema.clear();
    g_updateMessage = RegisterWindowMessageW(L"Windhawk.SystemResourceAlert.Update.0.4");
    g_dispatchMessage = RegisterWindowMessageW(L"Windhawk.SystemResourceAlert.Dispatch.0.4");
    if (!g_updateMessage || !g_dispatchMessage || !ResolveTaskbarAccessors()) {
        if (g_taskbarModule) FreeLibrary(g_taskbarModule);
        g_taskbarModule = nullptr;
        return FALSE;
    }
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_wakeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (g_stopEvent && g_wakeEvent) {
        g_thread = CreateThread(nullptr, 0, MonitorThread, nullptr, 0, nullptr);
    }
    if (!g_thread) {
        ReleaseWorkerHandles();
        FreeLibrary(g_taskbarModule);
        g_taskbarModule = nullptr;
        return FALSE;
    }
    g_cpuThread = CreateThread(nullptr, 0, CpuTemperatureThread, nullptr, 0, &g_cpuThreadId);
    Wh_Log(L"Native taskbar resource monitor started (no overlay window)");
    return TRUE;
}

void Wh_ModUninit() {
    g_unloading = true;
    if (g_stopEvent) SetEvent(g_stopEvent);
    if (g_cpuThreadId) CoCancelCall(g_cpuThreadId, 0);
    const bool workerStopped = !g_thread || WaitForSingleObject(g_thread, 2000) == WAIT_OBJECT_0;
    const bool cpuStopped = !g_cpuThread || WaitForSingleObject(g_cpuThread, 2000) == WAIT_OBJECT_0;
    HWND taskbar = g_taskbarWindow.load();
    if (!taskbar) taskbar = FindPrimaryTaskbar();
    const bool detached = !taskbar ||
        (IsWindow(taskbar) && DispatchToTaskbar(taskbar, UiCommand::Detach));
    if (workerStopped && cpuStopped) ReleaseWorkerHandles();
    if (!detached || !workerStopped || !cpuStopped || g_uiCallbacks.load() != 0) {
        // If Explorer is hung, don't unload code that a surviving subclass
        // could still call. No forced Explorer restart or cross-thread XAML
        // destruction. It can detach on its next queued update.
        HMODULE retained = nullptr;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_PIN,
                           reinterpret_cast<LPCWSTR>(&Wh_ModUninit), &retained);
        Wh_Log(L"A UI/driver/sensor call did not finish cleanup; retaining the module until Explorer exits");
        return;
    }
    if (g_taskbarModule) FreeLibrary(g_taskbarModule);
    g_taskbarModule = nullptr;
}

BOOL Wh_ModSettingsChanged(BOOL* reload) {
    if (reload) *reload = FALSE;
    LoadSettings();
    g_resetRequested = true;
    if (g_wakeEvent) SetEvent(g_wakeEvent);
    return TRUE;
}
