#include "omarchy_theme.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>

#include <cmath>
#include <optional>
#include <utility>

// Portions adapted from Omacut's Theme pattern; Copyright (c) 2026 David Heinemeier Hansson.
// SPDX-License-Identifier: MIT
// See docs/third_party_notices.md.
namespace {
struct Palette {
    QVariantMap colors;
    bool light = false;
};

QVariantMap FallbackColors() {
    return {{"background", QColor("#0e0e10")}, {"foreground", QColor("#e6e6e9")},
            {"accent", QColor("#ffd60a")}, {"selection", QColor("#3a3a40")},
            {"muted", QColor("#b0b0b8")}, {"yellow", QColor("#e0af68")}, {"red", QColor("#f7768e")}};
}

std::optional<Palette> ReadColors(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return std::nullopt;
    const auto bytes = file.read(64 * 1024 + 1);
    if (file.error() != QFileDevice::NoError || bytes.size() > 64 * 1024) return std::nullopt;
    static const QRegularExpression entry("^\\s*([a-z0-9_]+)\\s*=\\s*([\"']?)(#[0-9a-fA-F]{6}|light|dark)\\2\\s*(?:#.*)?$");
    Palette palette;
    QHash<QString, QString> modes;
    for (const auto& line : QString::fromUtf8(bytes).split('\n')) {
        const auto match = entry.match(line);
        if (!match.hasMatch()) continue;
        const auto key = match.captured(1);
        const auto value = match.captured(3);
        if (value.startsWith('#')) palette.colors.insert(key, QColor(value));
        else modes.insert(key, value);
    }
    const QList<QStringList> aliases = {
        {"background", "bg", "color0"}, {"foreground", "fg", "color7"},
        {"red", "color1"}, {"yellow", "color3"}, {"accent", "blue", "color4"},
        {"muted", "dark_foreground", "color8"}, {"selection", "selection_background", "color8"}
    };
    for (const auto& names : aliases) {
        for (const auto& name : names.mid(1)) {
            if (!palette.colors.contains(names.first()) && palette.colors.contains(name))
                palette.colors.insert(names.first(), palette.colors.value(name));
        }
    }
    for (const auto& key : {"background", "foreground", "accent"})
        if (!palette.colors.contains(key)) return std::nullopt;
    for (const auto& key : {"muted", "yellow", "red"})
        if (!palette.colors.contains(key)) palette.colors.insert(key, palette.colors.value("foreground"));
    if (!palette.colors.contains("selection")) palette.colors.insert("selection", palette.colors.value("accent"));
    auto mode = modes.value("mode", modes.value("theme_type"));
    if (mode.isEmpty() && QFileInfo::exists(QFileInfo(path).absolutePath() + "/light.mode")) mode = "light";
    if (mode.isEmpty()) {
        const auto background = palette.colors.value("background").value<QColor>();
        mode = background.red() + background.green() + background.blue() > 382 ? "light" : "dark";
    }
    palette.light = mode == "light";
    return palette;
}

QColor ContrastText(const QColor& color) {
    const auto linear = [](double value) {
        return value <= .04045 ? value / 12.92 : std::pow((value + .055) / 1.055, 2.4);
    };
    const auto luminance = .2126 * linear(color.redF()) + .7152 * linear(color.greenF()) + .0722 * linear(color.blueF());
    return luminance > .179 ? QColor("black") : QColor("white");
}
}

OmarchyTheme::OmarchyTheme(const QString& currentDirectory, QObject* parent)
    : QObject(parent), directory_(currentDirectory.isEmpty() ? QDir::homePath() + "/.local/state/omarchy/current" : currentDirectory),
      colors_(FallbackColors()) {
    debounce_.setSingleShot(true);
    debounce_.setInterval(80);
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, [this] { debounce_.start(); });
    connect(&watcher_, &QFileSystemWatcher::fileChanged, this, [this] { debounce_.start(); });
    connect(&debounce_, &QTimer::timeout, this, &OmarchyTheme::reload);
    reload();
}

QColor OmarchyTheme::background() const { return colors_.value("background").value<QColor>(); }
QColor OmarchyTheme::foreground() const { return colors_.value("foreground").value<QColor>(); }
QColor OmarchyTheme::accent() const { return colors_.value("accent").value<QColor>(); }
QColor OmarchyTheme::selectionForeground() const { return ContrastText(colors_.value("selection").value<QColor>()); }
QVariantMap OmarchyTheme::colors() const { return colors_; }
bool OmarchyTheme::light() const { return light_; }

void OmarchyTheme::reload() {
    const auto watched = watcher_.files() + watcher_.directories();
    if (!watched.isEmpty()) watcher_.removePaths(watched);
    auto ancestor = directory_;
    while (!QFileInfo::exists(ancestor)) {
        const auto parent = QFileInfo(ancestor).absolutePath();
        if (parent == ancestor) break;
        ancestor = parent;
    }
    auto candidates = QStringList{ancestor, QFileInfo(ancestor).absolutePath(), directory_,
                                 directory_ + "/theme", directory_ + "/theme/colors.toml"};
    candidates.removeDuplicates();
    for (const auto& path : std::as_const(candidates))
        if (QFileInfo::exists(path)) watcher_.addPath(path);
    const auto palette = ReadColors(directory_ + "/theme/colors.toml");
    // A switch briefly removes the directory; keep the last complete palette until its replacement arrives.
    if (!palette || (palette->colors == colors_ && palette->light == light_)) return;
    colors_ = palette->colors;
    light_ = palette->light;
    emit changed();
}
