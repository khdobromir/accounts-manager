#include "config.hpp"

#include <QFile>
#include <QStandardPaths>
#include <dotenv/dotenv.hpp>

#include <exception>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
QByteArray ConfiguredKey() {
    const auto path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/api-key";
    const int fd = ::open(QFile::encodeName(path).constData(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) return {};
    QFile file;
    if (!file.open(fd, QIODevice::ReadOnly, QFileDevice::AutoCloseHandle)) {
        ::close(fd);
        return {};
    }
    struct stat info{};
    if (::fstat(fd, &info) || !S_ISREG(info.st_mode) || info.st_uid != ::geteuid() ||
        (info.st_mode & 0777) != 0600 || info.st_size > 16384) return {};
    return file.read(16384).trimmed();
}
}

QByteArray ReadApiKey(bool background) {
    auto key = qgetenv("API_KEY").trimmed();
    if (key.isEmpty()) key = ConfiguredKey();
    if (key.isEmpty() && !background) {
        try {
            dotenv::load(".env");
            key = qgetenv("API_KEY").trimmed();
        } catch (const std::exception&) {
            return {};
        }
    }
    if (key.contains('\r') || key.contains('\n')) return {};
    return key;
}
