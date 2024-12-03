#pragma once
#include <functional>
#include <mab_types.hpp>
#include <receiver_pipe.h>
#include <transmitter_pipe.h>

namespace mab
{

    class I_CommunicationDevice
    {
      public:
        mab::ReceiverPipe    receiverPipe;
        mab::TransmitterPipe transmitterPipe;
    };
}  // namespace mab