#include <filesystem>
#include <sstream>
#include "curl_handler.hpp"
#include "logger.hpp"
#include "utilities.hpp"

namespace mab
{
    // No overall time limit (slow links are fine), but give up on a connection that can't be
    // made in 10 s or a transfer that stalls completely for 30 s.
    // An already downloaded file is fetched again only if the server has a newer one (-z), so
    // e.g. updating drives in series reuses it. -R keeps the server file time for that check.
    bool CurlHandler::download(std::string_view url, const std::filesystem::path& outputPath)
    {
        std::error_code   ec;
        std::stringstream command;
        command << "curl --fail -L -R --connect-timeout 10 --speed-time 30 ";
        if (std::filesystem::exists(outputPath, ec))
            command << "-z \"" << outputPath.string() << "\" ";
        command << "-o \"" << outputPath.string() << "\" \"" << url << "\"";
        if (executeCommand(command.str()))
        {
            // A partial file has local time, it would pass the -z check next time
            std::filesystem::remove(outputPath, ec);
            return false;
        }
        return true;
    }

    bool CurlHandler::loadIndex(mINI::INIStructure& index)
    {
        Logger                log(Logger::ProgramLayer_E::LAYER_2, "CurlHandler");
        std::filesystem::path indexPath = std::filesystem::temp_directory_path() / FW_INDEX_FILE;
        if (!download(std::string(FW_SERVER_ROOT) + FW_INDEX_FILE, indexPath))
        {
            log.error("Could not download firmware index!");
            return false;
        }
        if (!mINI::INIFile(indexPath.string()).read(index))
        {
            log.error("Could not read firmware index [ %s ]", indexPath.string().c_str());
            return false;
        }
        return true;
    }

    std::string CurlHandler::findIndexEntry(const mINI::INIStructure& index,
                                            const std::string&        prefix,
                                            const std::string&        version,
                                            const char*               key)
    {
        const std::string name   = prefix + version;
        const bool        latest = version == "latest";
        for (const auto& entry : index)
        {
            const std::string& section = entry.first;
            if (section.compare(0, prefix.size(), prefix) != 0)
                continue;

            bool match = false;
            if (latest)
                match =
                    section.size() >= 7 && section.compare(section.size() - 7, 7, "_latest") == 0;
            else
                match = section.compare(0, name.size(), name) == 0 &&
                        (section.size() == name.size() || section[name.size()] == '_');

            if (match && entry.second.has(key))
                return entry.second.get(key);
        }
        return "";
    }
}  // namespace mab
