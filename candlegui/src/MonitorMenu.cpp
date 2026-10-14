#include "MonitorMenu.hpp"
#include "commonMemory.hpp"
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
        refreshMDStatus();
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
        bool testOngoing = m_data->testOngoing;

        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::Text("Actuators");

        if (!MonitorMenuMDStatus.empty())
        {
            for (size_t i = 0; i < MonitorMenuMDStatus.size(); i++)
            {
                const MDStatus_S& status = MonitorMenuMDStatus[i];

                if (std::isnan(status.driverTemp))
                    ImGui::Text("ID %u   --", (unsigned)status.id);
                else
                    ImGui::Text("ID %u   %.1f C", (unsigned)status.id, status.driverTemp);
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

/*

 Help functions

*/
void MonitorMenu::refreshMDStatus()
{
    std::lock_guard<std::mutex> lock(m_data->mtx);
    MonitorMenuMDStatus = m_data->mdStatus;
}
