
#include <unistd.h>
#include <csignal>
#include "candle.hpp"
#include "MD.hpp"

mab::MD* pmd;
f32 vel = 150;

void handle_sigint(int sig)
{
    for (f32 tmpvel = vel; tmpvel > 0.; tmpvel -= 0.01 * vel)
    {
        pmd->setTargetVelocity(tmpvel);
        usleep(10000);
    }
    pmd->setTargetVelocity(0);
    usleep(200000);

    pmd->disable();
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, handle_sigint);

    mab::Candle* candle =
        mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB);
    mab::MD md(100, candle);
    pmd = &md;

    md.zero();
    md.setMaxTorque(3.);
    md.setCurrentLimit(50);
    md.setMotionMode(mab::MdMode_E::IMPEDANCE);
    md.setImpedanceParams(0., 0.1);
    md.enable();

    for (f32 tmpvel = 0.; tmpvel < vel; tmpvel += 0.01 * vel)
    {
        md.setTargetVelocity(tmpvel);
        usleep(50000);
    }
    for (u16 i = 0; i < 1000; i++)
    {
        md.setTargetVelocity(vel);
        printf("%.2f, %.2f\n", md.getPosition().first, md.getVelocity().first);
        usleep(100'000);
    }
    handle_sigint(0);
    mab::detachCandle(candle);
}
