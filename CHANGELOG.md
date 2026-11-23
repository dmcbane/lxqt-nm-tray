# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [0.1.2] - 2026-11-22

### Fixed
- Wi-Fi password prompt: the SSID is now HTML-escaped (the prompt's QLabel auto-detects rich
  text). The fix is in nm-tray's code, so the submodule now tracks the
  `escape-connection-names` branch of https://github.com/dmcbane/nm-tray (upstream master plus
  palinek/nm-tray#77). Switch back to upstream once that PR is merged.

### Added
- Test that fails if the submodule is moved to a revision without the password-prompt escaping.

## [0.1.1] - 2026-11-22

### Fixed
- Connection names (e.g. Wi-Fi SSIDs from nearby access points) are HTML-escaped before
  going into the rich-text tooltip and notification body. Inherited from nm-tray 0.5.1.
- `%N` sequences in a connection name are no longer expanded as placeholders.

## [0.1.0] - 2026-11-22

### Added
- lxqt-panel plugin (`nmtray`) hosting nm-tray 0.5.1's connection menu, context actions,
  connection-info and debug dialogs, status icon, tooltip and notifications.
- Menus anchored to the panel window (transient parent) and placed with the panel's
  `calculatePopupWindowPos`, so they open correctly on Wayland.
- Settings read from / written to nm-tray's own config file, never the panel's.
- Tests: popup anchoring, settings isolation, plugin loads via `QPluginLoader` with the
  lxqt-panel IID.
