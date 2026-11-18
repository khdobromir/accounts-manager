# Using AccountsManager on Linux

## Build and launch

Install GNU Make, Clang with `-std=c++26`, CMake 4.4.5 or later and Qt 6.8 or later with Core, DBus, Network, SQL, Quick and Quick Controls 2. Install the QSQLITE driver and the Qt Quick Controls, Material and Layouts QML modules. The build uses vcpkg for nlohmann-json and ayush272002-dotenv. Install system packages separately.

Run these commands from the project root. The Makefile looks for vcpkg in `$HOME/vcpkg`. Override the location with `make VCPKG_ROOT=/path/to/vcpkg run release`.

| Build | Build and launch | Output directory |
| --- | --- | --- |
| `make debug` | `make run debug` | `build/clang-debug` |
| `make release` | `make run release` | `build/clang-release` |

The executable includes its QML resources. The application interface is in Russian.

## API access

Create a personal key and allow your public IP address in the [Clash of Clans Developer Portal](https://developer.clashofclans.com/). The application reads the key at startup, in this order:

1. The `API_KEY` environment variable.
2. The `api-key` file in the application's configuration directory, normally `~/.config/AccountsManager/AccountsManager/api-key`. It must be a regular file owned by your user, with mode `0600`, containing only the key on one line.
3. On a normal launch, `API_KEY=your_key` in `.env` in the working directory. Git ignores this file.

For login reminders, use the private configuration file or set the environment variable in your graphical session. To prepare a private file in a shell:

```sh
config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/AccountsManager/AccountsManager"
mkdir -p "$config_dir"
(umask 077 && touch "$config_dir/api-key")
chmod 600 "$config_dir/api-key"
```

Open the file in your editor and paste the key. Keep it out of shell history, Git and shared files. SQLite stores account settings and API snapshots. Without a key you can still manage accounts, view cached data and receive scheduled transfer reminders. For HTTP 403, check the key, allowed IP and resource access.

## Accounts and dashboard

On the accounts tab, add the player's public tag including the leading `#`. Profile lookup runs asynchronously. An unavailable account stays in the list with its error. Use the account controls to set an alias, change the order or remove an account.

For regular Clan Wars, leave the clan field empty to follow the current clan or pin a specific clan by tag. Set the CWL clan independently so that the league round remains visible after the player moves. To plan a transfer, enter the destination tag and optionally a local date and time in `YYYY-MM-DDTHH:mm` format. Clear the destination to cancel. Make the transfer in the game and clear the plan when finished.

The overview opens with all accounts in saved order. Select the attention filter for outstanding attacks and due transfer plans. The nearest deadline comes first. Equal deadlines retain the saved order and actions without a deadline come last. The card shows the mode, clan, available count, deadline where known, source and last update. A fresh player profile allows a return-to-clan hint. Cached results remain visible with a stale label.

Refresh runs every five minutes by default. In settings, enter 1–1440 minutes or 0 for manual refresh. The refresh button respects the API cooldown after HTTP 429.

## Background reminders

In settings, enable background reminders and save. The executable must remain at the path recorded when you enable the option. Closing the window leaves one instance running. Launch the application again to open its window. **Quit application** in settings ends the process for this session. On the next graphical login, XDG Autostart starts it with `--background` and the window hidden. Disable the option and save to remove its autostart entry.

Your desktop session needs a session D-Bus, a desktop notification server and XDG Autostart support. During the final hour of a regular war or CWL round, a fresh snapshot showing a remaining attack triggers a reminder. A timed transfer triggers a reminder when due, including when the application next runs after its scheduled time. An undated plan appears only on the dashboard. Accepted reminders are recorded in SQLite across refreshes and restarts. Desktop notification settings control presentation.

## Storage and appearance

Settings shows the exact database path. On a typical Linux setup it is `~/.local/share/AccountsManager/AccountsManager/accounts.sqlite`. A lock protects the database from concurrent instances. SQLite uses a single connection in a worker thread and transactions for account updates and schema migrations. The application prunes unused API resources and reports corrupt or newer databases without replacing them.

On Omarchy the application follows `~/.local/state/omarchy/current/theme/colors.toml` for its background, text, accent, selection, warnings and errors. It accepts quoted or bare `#RRGGBB` colors, trailing comments and aliases such as `bg`, `fg`, `color0` and `color7`. It reads `mode` or `theme_type`, then the `light.mode` marker, then background luminance. File events update the palette even when the theme directory or symlink is replaced. Unreadable, oversized or incomplete palettes retain the last valid colors. The default palette is dark. [Third-party notices](third_party_notices.md)
