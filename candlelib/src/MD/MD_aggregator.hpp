#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_frame_adapter.hpp"

#include <memory>
#include <chrono>
#include <deque>
#include <unordered_map>
namespace mab
{
    class MDAggregator
    {
      public:
        struct MDFrame
        {
            const MdFrameId_E                            m_frameType;
            const canId_t                                m_canId;
            std::chrono::high_resolution_clock::duration m_timeout = std::chrono::microseconds(1);
            std::vector<u8>                              m_frame;

            MDFrame(MdFrameId_E frameType, canId_t canId) : m_frameType(frameType), m_canId(canId)
            {
                m_frame = getHeader();
            }

            inline std::vector<u8> getHeader() const noexcept
            {
                return {(u8)(u16)m_frameType, (u8)((u16)m_frameType << 8), 0x0};
            }

            template <typename T>
            inline bool insertRegister(MDRegisterEntry_S<T> reg)
            {
                auto serializedReg = reg.getSerializedRegister();
                if (m_frame.size() + serializedReg.size() > 64)
                    return true;
                m_frame.return false;
            }
        };

        MDAggregator(std::shared_ptr<CANdleFrameAdapter> cfAdapter) : m_cfAdapter(cfAdapter)
        {
        }

      private:
        const std::shared_ptr<CANdleFrameAdapter>            m_cfAdapter;
        std::unordered_map<MdFrameId_E, std::deque<MDFrame>> m_frameTypeMap;
    };
}  // namespace mab