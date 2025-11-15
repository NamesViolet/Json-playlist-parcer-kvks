// json_parser.cpp
// Reads .json files in a directory and extracts `playlistName` and `shareCode`.
// Uses nlohmann::json when available, otherwise falls back to safe regex extraction.

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>
#include <sstream>

#if defined(__has_include)
#  if __has_include(<nlohmann/json.hpp>)
#    include <nlohmann/json.hpp>
#    define HAVE_NLOHMANN_JSON 1
#    using json = nlohmann::json;
#  endif
#endif

namespace fs = std::filesystem;

struct PlaylistData {
    std::string playlistName;
    std::string shareCode;
};

static std::string readFileToString(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return {};
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

static PlaylistData parseJsonFile(const std::string& filepath) {
    std::string content = readFileToString(filepath);
    PlaylistData data;
    if (content.empty()) {
        std::cerr << "Failed to open or empty file: " << filepath << std::endl;
        return data;
    }

#if defined(HAVE_NLOHMANN_JSON)
    try {
        auto j = json::parse(content);
        if (j.contains("playlistName") && j["playlistName"].is_string())
            data.playlistName = j["playlistName"].get<std::string>();
        if (j.contains("shareCode") && j["shareCode"].is_string())
            data.shareCode = j["shareCode"].get<std::string>();
    } catch (const std::exception&) {
        // fall back to regex below
    }
#endif

    if (data.playlistName.empty() || data.shareCode.empty()) {
        std::regex playlistPattern("\"playlistName\"\\s*:\\s*\"([^\"]*)\"");
        std::regex shareCodePattern("\"shareCode\"\\s*:\\s*\"([^\"]*)\"");
        std::smatch match;
        if (std::regex_search(content, match, playlistPattern))
            data.playlistName = match[1].str();
        if (std::regex_search(content, match, shareCodePattern))
            data.shareCode = match[1].str();
    }

    std::cout << "File: " << fs::path(filepath).filename().string() << std::endl;
    std::cout << "  playlistName: " << (data.playlistName.empty() ? "(not found)" : data.playlistName) << std::endl;
    std::cout << "  shareCode: " << (data.shareCode.empty() ? "(not found)" : data.shareCode) << std::endl;

    return data;
}

static void writeResultsToFile(const std::vector<PlaylistData>& results, const std::string& outputFile) {
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }

    for (const auto& result : results) {
        out << "Playlist Name: " << (result.playlistName.empty() ? "(not found)" : result.playlistName) << std::endl;
        out << "Share Code: " << (result.shareCode.empty() ? "(not found)" : result.shareCode) << std::endl;
        out << std::endl;
    }

    out.close();
    std::cout << "Results written to " << outputFile << std::endl;
}

int main(int argc, char* argv[]) {
    std::string folderPath = (argc > 1) ? argv[1] : ".";

    if (!fs::exists(folderPath)) {
        std::cerr << "Error: Path does not exist: " << folderPath << std::endl;
        return 1;
    }

    std::cout << "Scanning folder: " << folderPath << std::endl;
    std::cout << std::endl;

    std::vector<PlaylistData> results;
    int fileCount = 0;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            PlaylistData data = parseJsonFile(entry.path().string());
            results.push_back(data);
            ++fileCount;
        }
    }

    if (fileCount == 0) {
        std::cout << "No .json files found in the directory." << std::endl;
    } else {
        std::cout << std::endl;
        std::cout << "Processed " << fileCount << " JSON file(s)." << std::endl;
        
        std::string outputFile = (fs::path(folderPath).parent_path() / "results.txt").string();
        writeResultsToFile(results, outputFile);
    }

    return 0;
}