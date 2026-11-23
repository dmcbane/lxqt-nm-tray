# lxqt-nm-tray

A native LXQt panel plugin that hosts [nm-tray](https://github.com/palinek/nm-tray)'s
NetworkManager menus directly in `lxqt-panel`.

## Why

On LXQt under Wayland (sway, labwc, kwin_wayland, …) nm-tray's connection menu opens in the
wrong place or not at all. nm-tray is a separate tray process: it opens its menu at
`QCursor::pos()`, which Wayland doesn't expose, and it has no parent surface for the compositor
to position the popup against. As a panel plugin, the menu is anchored to the panel's own
window and positioned by the panel, so it opens next to the icon.

The networking logic (model, connection/Wi-Fi menu, info dialogs) is nm-tray's own code,
built from the pinned submodule in `third_party/nm-tray`. Only the tray-icon glue is replaced.

## Use

- **Left click**: connection / Wi-Fi menu
- **Right click**: enable networking / Wi-Fi, notifications, connection info, debug info,
  Wi-Fi scan, edit connections, about
- Panel options (move/remove) are on the plugin's handle, as with the system tray plugin.

Settings are shared with standalone nm-tray (`~/.config/nm-tray/nm-tray.conf`), e.g.
`connectionsEditor=nm-connection-editor`.

Don't run standalone `nm-tray` at the same time, or you'll get two icons.

## Build & install

Requires Qt 6, KF6 NetworkManagerQt, and lxqt-panel's development headers
(`/usr/include/lxqt/ilxqtpanelplugin.h`, shipped in Arch's `lxqt-panel` package).

```sh
git submodule update --init
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build && ctest --test-dir build
sudo cmake --install build
```

Installs `/usr/lib/lxqt-panel/libnmtray.so` and `/usr/share/lxqt/lxqt-panel/nmtray.desktop`.
Restart the panel, then *Configure Panel → Widgets → Add → "Network (nm-tray)"*.

The plugin uses lxqt-panel's in-process plugin interface, which has no stability promise:
**rebuild after lxqt-panel upgrades.**

## License

GPL-2.0-or-later (it compiles nm-tray's GPL-2.0-or-later sources). See `LICENSE`.
