#include "mab_types.hpp"
#include "candle.hpp"
#include "MD.hpp"
#include "pds.hpp"

namespace mab
{
    class FileHandler
    {
        enum class FileHandlerError_E
        {
            UNKNOWN_ERROR,
            OK
        };

        struct CANdleTree
        {
            std::shared_ptr<CandleBuilder>    candleBuilder;
            std::shared_ptr<std::vector<MD>>  MdVector;
            std::shared_ptr<std::vector<Pds>> PdsVector;
        };
    };
}  // namespace mab