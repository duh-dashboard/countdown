# Countdown Widget

Shows the number of days remaining until a user-specified target date, with a configurable label (e.g. "Until vacation"). Both the date and label persist across restarts.

## Requirements

- Linux
- Qt 6.2+ (Widgets)
- CMake 3.21+
- C++20 compiler
- [`widget-sdk`](https://github.com/duh-dashboard/widget-sdk) installed

## Build

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=~/.local
cmake --build build
cmake --install build --prefix ~/.local
```

The plugin installs to `~/.local/lib/dashboard/plugins/`.

## Configuration

| Setting | Description |
|---|---|
| **Label** | Display name shown above the day count |
| **Target date** | ISO 8601 date (`YYYY-MM-DD`) to count down to |

Both are set directly in the widget UI and saved automatically.

## License

GPL-3.0-or-later — see [LICENSE](LICENSE).
