# nm-tray menu misplaced or missing on LXQt/Wayland

**Symptom:** Under LXQt on sway (Wayland), left-clicking nm-tray's tray icon opens the
connection menu in the wrong spot, or nothing appears.

**Root cause:** nm-tray (`src/tray.cpp`, `Tray::onActivated`) does
`menu->popup(QCursor::pos())`. Wayland clients can't read the global pointer position, and
the menu has no transient parent (nm-tray owns no window), so there is no surface for the
compositor to position the `xdg_popup` against. The right-click menu is *probably* unaffected, since
the panel's statusnotifier plugin renders it itself via DBusMenu (not yet confirmed).

**Fix:** Run nm-tray's code inside lxqt-panel as a plugin. `popup::showAnchored()` sets the
panel's top-level window as the menu's transient parent (forcing native handles with
`winId()` first) and pops it at `ILXQtPanelPlugin::calculatePopupWindowPos()`.

**Verified:** 2026-11-22, by hand on sway with a bottom panel: the left-click menu opens next to the icon.

**Gotchas found on the way:**
- nm-tray uses a default `QSettings{}`. Inside the panel that resolves to the panel's config,
  so every settings access names `("nm-tray", "nm-tray")` explicitly (`NmTraySettings`).
- nm-tray's `translate.cpp` installs a global Qt translator from a
  `Q_COREAPP_STARTUP_FUNCTION`; it is excluded from the build.
- Destruction order: `NmTrayStatus` is a QObject child, so it would be deleted after the
  `NmModel` member it references. The plugin destructor deletes it explicitly.
- `QPluginLoader`-loaded panel plugins have no stable ABI; rebuild after lxqt-panel upgrades.
- Smoke-testing without touching the live panel: build a config with only `plugins=nmtray`,
  then run `LXQTPANEL_PLUGIN_PATH=$PWD/build LXQT_PANEL_PLUGINS_DIR=<dir with nmtray.desktop>
  lxqt-panel -c <conf>`. Check `/proc/<pid>/maps` for `libnmtray.so`, and use `pgrep -n -x
  lxqt-panel`, not `pgrep -f`, which matches the `timeout` wrapper.
- Adding the plugin by editing `~/.config/lxqt/panel.conf`: putting `nmtray` in `plugins=`
  isn't enough. The panel silently skips any entry with no `[<name>]` section containing
  `type=<desktop id>`. Built-in plugins get that from `/etc/xdg/lxqt/panel.conf`. Stop the
  panel before editing, or it may overwrite the file.
