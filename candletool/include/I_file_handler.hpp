#include <filesystem>

namespace mab
{
    class I_FileHandler
    {
      public:
        enum class FileHandler_E
        {
            UNKNOWN_ERROR,
            OK,
            ERROR_PERFORMING_ACTION,
            FILE_NOT_FOUND
        };

        virtual FileHandler_E perform(std::filesystem::path path) = 0;
    };
}  // namespace mab