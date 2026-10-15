// FileManager.h - saves/loads record files. Format: "#ELM1<TAB>kind<TAB>count<TAB>checksum" header
// followed by one tab-separated line per record. Saves are atomic (write .tmp, then rename).
#pragma once
#include <string>
#include <vector>
#include "PtrList.h"
#include "errors.h"

namespace elm {

class FileManager {
    std::string dir_;
public:
    explicit FileManager(const std::string& dir) : dir_(dir) {}
    const std::string& dir() const { return dir_; }
    std::string path(const std::string& file) const { return dir_ + "/" + file; }
    void ensureDir() const;

    // Writes every record of the list; returns number of records written.
    template <class T>
    int save(const std::string& file, const PtrList<T>& list, const std::string& kind) const {
        std::vector<std::string> lines;
        for (int i = 0; i < list.size(); ++i) lines.push_back(list[i]->serialize());
        writeLines(file, kind, lines);
        return list.size();
    }
    // Reads a file into raw lines (header verified). Missing file -> empty vector, ok=false.
    std::vector<std::string> readLines(const std::string& file, const std::string& kind,
                                       bool& ok, std::vector<std::string>& warnings) const;
    // Copies all .dat files into <dir>/backup/<stamp>/ ; returns the folder path.
    std::string backup(const std::string& stamp) const;
private:
    void writeLines(const std::string& file, const std::string& kind,
                    const std::vector<std::string>& lines) const;
    static unsigned long checksum(const std::vector<std::string>& lines);
};

}  // namespace elm
