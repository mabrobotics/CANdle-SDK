#include <cstdlib>
#include <iostream>
#include <functional>
#include "md_types.hpp"

int main()
{
    mab::MDRegisters_S regs;

    auto fn = []<typename T>(mab::MDRegisterEntry_S<T>& reg) { std::cout << reg.m_name << "\n"; };

    regs.forEachRegister(fn);

    return EXIT_SUCCESS;
}
