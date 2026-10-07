#include "MonitorMenu.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "mab_types.hpp"

MonitorMenu::MonitorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io)
    : m_data(commonMemory), m_io(io)
{
}

/*

Monitor Menu Draw Functions

*/

void MonitorMenu::drawMonitorMenu()
{
    if (ImGui::BeginChild("Left Column Monitor",
                          ImVec2(actuatorsMenuBarWidth, 0),
                          ImGuiChildFlags_None,
                          m_data->flagsBackMenu))
    {
        drawActuatorMenu();
        drawCanBusInfoMenu();
    }
    ImGui::EndChild();
}

void MonitorMenu::drawActuatorMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(m_data->leftMenuPadding, m_data->windowsPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m_data->insideWindowPadding / 2, m_data->insideWindowPadding));
    if (ImGui::BeginChild("Actuators Menu",
                          ImVec2(actuatorsMenuBarWidth, -canBusInfoBarHeight),
                          ImGuiChildFlags_Borders,
                          m_data->flagsBackMenu))
    {
        bool                      testOngoing = m_data->testOngoing;
        std::vector<mab::canId_t> mdIds       = m_data->mdIDs;

        std::string chosenIDname = "";

        if (testOngoing)
        {
            ImGui::BeginDisabled();
        }

        ImGui::Text("Actuators");

        if (!mdIds.empty())
        {
            for (const mab::canId_t& id : m_data->mdIDs)
            {
                chosenIDname = "ID " + std::to_string(uint16_t((id)));

                ImGui::TextUnformatted(chosenIDname.c_str());

                ImGui::SameLine();
            }
        }

        /*
            Content
        */

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawCanBusInfoMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(m_data->leftMenuPadding, m_data->insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m_data->insideWindowPadding / 2, m_data->insideWindowPadding));
    if (ImGui::BeginChild(
            "Can Bus Info Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, m_data->flagsBackMenu))
    {
        ImGui::Text("CAN Bus Info");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}
