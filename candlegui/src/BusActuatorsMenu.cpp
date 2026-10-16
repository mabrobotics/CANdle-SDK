#include "BusActuatorsMenu.hpp"

BusActuatorMenu::BusActuatorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io)
    : m_data(commonMemory), m_io(io)
{
}

void BusActuatorMenu::drawBusActuatorMenu()
{
    if (ImGui::BeginChild("Left Column BusActuator",
                          ImVec2(GuiStyle::leftMenuBarWidth, 0),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        drawCANdleInfoMenu();
        drawTestGroupsMenu();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("Middle Column BusActuator",
                          ImVec2(GuiStyle::middleWindowWidth, 0),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        drawFoundOnBusMenu();
    }
    ImGui::EndChild();
}

/*

    Bus & Actuators draw functions

*/

void BusActuatorMenu::drawCANdleInfoMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding / 2, GuiStyle::insideWindowPadding));
    if (ImGui::BeginChild("CANdle Info Menu",
                          ImVec2(GuiStyle::leftMenuBarWidth, candleInfoMenuHeight),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;
        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::Text("CANdle");

        ImGui::PushFont(nullptr, 18.0f);

        ImGui::PopFont();

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void BusActuatorMenu::drawTestGroupsMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding / 2, GuiStyle::insideWindowPadding));
    if (ImGui::BeginChild("Test Groups Menu",
                          ImVec2(GuiStyle::leftMenuBarWidth, -1),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;
        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::Text("Test Groups");

        GuiStyle::buttonInvisibleStyle();
        if (ImGui::Button("+ Add group",
                          ImVec2(GuiStyle::leftMenuWideButtonWidth, GuiStyle::largeButtonHeight)))
        {
        }
        GuiStyle::endButtonInvisibleStyle();

        ImGui::PushFont(nullptr, 18.0f);

        ImGui::PopFont();

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void BusActuatorMenu::drawFoundOnBusMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding, GuiStyle::insideWindowPadding));

    if (ImGui::BeginChild("##Found On Bus Menu",
                          ImVec2(GuiStyle::middleWindowWidth, -1),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        ImGui::Text("Found On the Bus");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}