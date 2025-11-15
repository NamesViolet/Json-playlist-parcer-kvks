// json_parser.cpp
// Reads .json files in a directory and extracts `playlistName` and `shareCode`.
// Uses nlohmann::json when available, otherwise falls back to safe regex extraction.

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <regex>
#include <sstream>
#include <set>

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
    std::string authorName;
    std::string authorSteamId;
    std::string description;
};

static std::string readFileToString(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return {};
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

static PlaylistData parseJsonFile(const std::string& filepath, bool includeAuthor, bool includeDescription) {
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
        if (includeAuthor) {
            if (j.contains("authorName") && j["authorName"].is_string())
                data.authorName = j["authorName"].get<std::string>();
            if (j.contains("authorSteamId") && j["authorSteamId"].is_string())
                data.authorSteamId = j["authorSteamId"].get<std::string>();
        }
        if (includeDescription) {
            if (j.contains("description") && j["description"].is_string())
                data.description = j["description"].get<std::string>();
        }
    } catch (const std::exception&) {
        // fall back to regex below
    }
#endif

    if (data.playlistName.empty() || data.shareCode.empty() || 
        (includeAuthor && (data.authorName.empty() || data.authorSteamId.empty())) ||
        (includeDescription && data.description.empty())) {
        std::regex playlistPattern("\"playlistName\"\\s*:\\s*\"([^\"]*)\"");
        std::regex shareCodePattern("\"shareCode\"\\s*:\\s*\"([^\"]*)\"");
        std::smatch match;
        if (std::regex_search(content, match, playlistPattern))
            data.playlistName = match[1].str();
        if (std::regex_search(content, match, shareCodePattern))
            data.shareCode = match[1].str();
        if (includeAuthor) {
            std::regex authorNamePattern("\"authorName\"\\s*:\\s*\"([^\"]*)\"");
            std::regex authorSteamIdPattern("\"authorSteamId\"\\s*:\\s*\"([^\"]*)\"");
            if (std::regex_search(content, match, authorNamePattern))
                data.authorName = match[1].str();
            if (std::regex_search(content, match, authorSteamIdPattern))
                data.authorSteamId = match[1].str();
        }
        if (includeDescription) {
            std::regex descriptionPattern("\"description\"\\s*:\\s*\"([^\"]*)\"");
            if (std::regex_search(content, match, descriptionPattern))
                data.description = match[1].str();
        }
    }

    std::cout << "File: " << fs::path(filepath).filename().string() << std::endl;
    std::cout << "  playlistName: " << (data.playlistName.empty() ? "(not found)" : data.playlistName) << std::endl;
    std::cout << "  shareCode: " << (data.shareCode.empty() ? "(not found)" : data.shareCode) << std::endl;
    if (includeAuthor) {
        std::cout << "  authorName: " << (data.authorName.empty() ? "(not found)" : data.authorName) << std::endl;
        std::cout << "  authorSteamId: " << (data.authorSteamId.empty() ? "(not found)" : data.authorSteamId) << std::endl;
    }
    if (includeDescription) {
        std::cout << "  description: " << (data.description.empty() ? "(not found)" : data.description) << std::endl;
    }

    return data;
}

static void writeResultsToFile(const std::vector<PlaylistData>& results, const std::string& outputFile, bool includeAuthor, bool includeDescription) {
    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Failed to open output file: " << outputFile << std::endl;
        return;
    }

    for (const auto& result : results) {
        out << "Playlist Name: " << (result.playlistName.empty() ? "(not found)" : result.playlistName) << std::endl;
        out << "Share Code: " << (result.shareCode.empty() ? "(not found)" : result.shareCode) << std::endl;
        if (includeAuthor && !result.authorName.empty() && !result.authorSteamId.empty()) {
            out << "Author: " << result.authorName << " SID: " << result.authorSteamId << std::endl;
        }
        if (includeDescription && !result.description.empty()) {
            out << "Description: " << result.description << std::endl;
        }
        out << std::endl;
    }

    out.close();
    std::cout << "Results written to " << outputFile << std::endl;
}

int main(int argc, char* argv[]) {
    bool includeAuthor = false;
    bool includeDescription = false;
    bool skipStats = false;
    std::string folderPath = ".";
    std::string outputPath = "";
    std::string outputFilename = "results.txt";

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-a" || arg == "--author") {
            includeAuthor = true;
        } else if (arg == "-d" || arg == "--description") {
            includeDescription = true;
        } else if (arg == "-q" || arg == "--quiet") {
            skipStats = true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputPath = argv[++i];
            } else {
                std::cerr << "Error: -o/--output requires a directory path" << std::endl;
                return 1;
            }
        } else if (arg == "-n" || arg == "--name") {
            if (i + 1 < argc) {
                outputFilename = argv[++i];
            } else {
                std::cerr << "Error: -n/--name requires a filename" << std::endl;
                return 1;
            }
        } else {
            folderPath = arg;
        }
    }

    if (!fs::exists(folderPath)) {
        std::cerr << "Error: Path does not exist: " << folderPath << std::endl;
        return 1;
    }

    if (!fs::is_directory(folderPath)) {
        std::cerr << "Error: Path is not a directory: " << folderPath << std::endl;
        return 1;
    }

    // Validate output path if provided
    if (!outputPath.empty()) {
        if (!fs::exists(outputPath)) {
            std::cerr << "Error: Output path does not exist: " << outputPath << std::endl;
            return 1;
        }
        if (!fs::is_directory(outputPath)) {
            std::cerr << "Error: Output path is not a directory: " << outputPath << std::endl;
            return 1;
        }
    }

    std::cout << "Scanning folder: " << folderPath << std::endl;
    std::cout << std::endl;

    std::vector<PlaylistData> results;
    std::set<std::string> seenShareCodes;
    std::set<std::string> seenPlaylistNames;
    int fileCount = 0;
    int successfulParses = 0;
    int failedParses = 0;
    int duplicateShareCodes = 0;
    int duplicateNames = 0;

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            ++fileCount;
            PlaylistData data = parseJsonFile(entry.path().string(), includeAuthor, includeDescription);
            
            if (!data.playlistName.empty() && !data.shareCode.empty()) {
                ++successfulParses;
                
                // Check for duplicate share codes
                if (seenShareCodes.count(data.shareCode)) {
                    ++duplicateShareCodes;
                    std::cout << "  [WARNING] Duplicate share code detected: " << data.shareCode << std::endl;
                } else {
                    seenShareCodes.insert(data.shareCode);
                }
                
                // Check for duplicate playlist names
                if (seenPlaylistNames.count(data.playlistName)) {
                    ++duplicateNames;
                    std::cout << "  [WARNING] Duplicate playlist name detected: " << data.playlistName << std::endl;
                } else {
                    seenPlaylistNames.insert(data.playlistName);
                }
                
                results.push_back(data);
            } else {
                ++failedParses;
            }
        }
    }

    if (!skipStats) {
        std::cout << std::endl;
        std::cout << "=== STATISTICS ===" << std::endl;
        std::cout << "Total files processed: " << fileCount << std::endl;
        std::cout << "Successful parses: " << successfulParses << std::endl;
        std::cout << "Failed parses: " << failedParses << std::endl;
        std::cout << "Duplicate share codes: " << duplicateShareCodes << std::endl;
        std::cout << "Duplicate playlist names: " << duplicateNames << std::endl;
        std::cout << "==================" << std::endl;
    }

    if (fileCount == 0) {
        std::cout << "\nNo .json files found in the directory." << std::endl;
    } else if (successfulParses > 0) {
        std::cout << std::endl;
        std::string outputFile;
        if (!outputPath.empty()) {
            outputFile = (fs::path(outputPath) / outputFilename).string();
        } else {
            outputFile = (fs::path(folderPath).parent_path() / outputFilename).string();
        }
        writeResultsToFile(results, outputFile, includeAuthor, includeDescription);
    } else {
        std::cout << "\nNo valid results to write." << std::endl;
    }

    return 0;
}
