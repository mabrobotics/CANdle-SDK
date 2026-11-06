#pragma once
#include "commonMemory.hpp"
#include "imgui.h"

class TestScript
{
  public:
    TestScript(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void drawTestScriptMenu();

  private:
    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    // Test Script Draw Functions
    void drawSelectBlocksMenu();
    void drawMainScriptMenu();
};
