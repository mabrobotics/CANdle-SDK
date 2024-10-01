#pragma once

#include <stdint.h>
#include "pds_types.hpp"

namespace mab
{

    class PdsMessage
    {
      public:
        PdsMessage()  = delete;
        ~PdsMessage() = default;

      protected:
        /* ModuleType / socket AKA who / where */
        PdsMessage(moduleType_E moduleType, socketIndex_E socket);

      private:
        const moduleType_E  m_moduleType;
        const socketIndex_E m_socketIndex;
    };

    class SetPropertyMessage : public PdsMessage
    {
      public:
        SetPropertyMessage(moduleType_E moduleType, socketIndex_E socket);
    };

}  // namespace mab