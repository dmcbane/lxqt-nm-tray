# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [0.1.0] - 2026-11-22

### Added
- lxqt-panel plugin (`nmtray`) hosting nm-tray 0.5.1's connection menu, context actions,
  connection-info and debug dialogs, status icon, tooltip and notifications.
- Menus anchored to the panel window (transient parent) and placed with the panel's
  `calculatePopupWindowPos`, so they open correctly on Wayland.
- Settings read from / written to nm-tray's own config file, never the panel's.
- Tests: popup anchoring, settings isolation, plugin loads via `QPluginLoader` with the
  lxqt-panel IID.
