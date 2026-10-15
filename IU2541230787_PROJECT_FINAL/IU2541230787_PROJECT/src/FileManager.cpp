#include "elm/FileManager.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include "elm/io.h"

namespace fs = std::filesystem;

namespace elm {

void FileManager::ensureDir() const {
    std::error_code ec;
    fs::create_directories(dir_, ec);
    if (ec) throw FileError("cannot create folder '" + dir_ + "': " + ec.message());
}

unsigned long FileManager::checksum(const std::vector<std::string>& lines) {   // FNV-1a
    unsigned long h = 1469598103UL;
    for (const std::string& l : lines) {
        for (char c : l) { h ^= static_cast<unsigned char>(c); h *= 16777619UL; h &= 0xFFFFFFFFUL; }
        h ^= 10UL; h *= 16777619UL; h &= 0xFFFFFFFFUL;
    }
    return h;
}

void FileManager::writeLines(const std::string& file, const std::string& kind, const std::vector<std::string>& lines) const {
    ensureDir();
    std::string final_ = path(file), tmp = final_ + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) throw FileError("cannot write '" + tmp + "'");
        out << "#ELM1\t" << kind << "\t" << lines.size() << "\t" << checksum(lines) << "\n";
        for (const std::string& l : lines) out << l << "\n";
        out.flush();
        if (!out) throw FileError("disk write failed for '" + tmp + "'");
    }
    std::error_code ec;
    fs::rename(tmp, final_, ec);                       // atomic replace: never a half-written file
    if (ec) throw FileError("cannot replace '" + final_ + "': " + ec.message());
}

std::vector<std::string> FileManager::readLines(const std::string& file, const std::string& kind, bool& ok,
                                                std::vector<std::string>& warnings) const {
    std::vector<std::string> lines;
    std::ifstream in(path(file));
    ok = static_cast<bool>(in);
    if (!ok) return lines;
    std::string header;
    std::getline(in, header);
    std::vector<std::string> h = io::split(header, '\t');
    if (h.size() != 4 || h[0] != "#ELM1" || h[1] != kind)
        throw FileError("'" + file + "' is not a valid " + kind + " data file (bad header)");
    for (std::string l; std::getline(in, l);) if (!l.empty()) lines.push_back(l);
    if (lines.size() != static_cast<size_t>(io::toInt(h[2], "count")))
        warnings.push_back(file + ": header says " + h[2] + " records but " + std::to_string(lines.size()) + " found");
    else if (std::to_string(checksum(lines)) != h[3])
        warnings.push_back(file + ": checksum mismatch (file edited outside the program?)");
    return lines;
}

std::string FileManager::backup(const std::string& stamp) const {
    std::string target = dir_ + "/backup/" + stamp;
    std::error_code ec;
    fs::create_directories(target, ec);
    if (ec) throw FileError("cannot create backup folder: " + ec.message());
    for (const auto& e : fs::directory_iterator(dir_, ec))
        if (e.path().extension() == ".dat" || e.path().extension() == ".log")
            fs::copy_file(e.path(), target + "/" + e.path().filename().string(), fs::copy_options::overwrite_existing, ec);
    return target;
}

}  // namespace elm
