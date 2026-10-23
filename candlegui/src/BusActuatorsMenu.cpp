#include "BusActuatorsMenu.hpp"
#include "GuiStyle.hpp"
#include "imgui.h"

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
    if (ImGui::BeginChild("Found On the Bus",
                          ImVec2(GuiStyle::middleWindowWidth, 0),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        drawFoundOnBusMenu();
    }
    ImGui::EndChild();

    if (chosenAction != ActionType_E::NONE)
    {
        ImGui::SameLine();
        drawActionMenu();
    }
}

/*

    Bus & Actuators draw functions

*/

void BusActuatorMenu::drawCANdleInfoMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("CANdle Info Menu",
                          ImVec2(GuiStyle::leftMenuBarWidth, 0.0f),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY,
                          GuiStyle::flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;

        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::PushFont(nullptr, 18.0f);
        ImGui::Text("CANdle");
        ImGui::PopFont();

        ImGui::Text("Device");
        candleInfo();

        if (ImGui::BeginChild(
                "##LeftColumn", ImVec2(candleInfoButtonWidth.x, 0.0f), ImGuiChildFlags_AutoResizeY))
        {
            ImGui::Text("Interface");
            interfaceInfo();
            drawScanBusButton();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("##RightColumn",
                              ImVec2(candleInfoButtonWidth.x, 0.0f),
                              ImGuiChildFlags_AutoResizeY))
        {
            ImGui::Text("Baudrate");
            baudrateInfo();
        }
        ImGui::EndChild();

        if (testOngoing)
            ImGui::EndDisabled();
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void BusActuatorMenu::candleInfo()
{
    bool candleAvailable = m_data->candleAvailable;

    GuiStyle::childInnerStyle();
    if (ImGui::BeginChild("Interface",
                          ImVec2(GuiStyle::leftMenuWideButtonWidth, 0),
                          ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
    {
        float  radius = 5.0f;
        ImVec2 pos    = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(pos.x + radius, pos.y + ImGui::GetTextLineHeight() * 0.5f);

        ImVec4      statusColor = candleAvailable ? GuiStyle::greenColor : GuiStyle::redColor;
        const char* statusText  = candleAvailable ? "CONNECTED" : "DISCONNECTED";
        ImU32       colorU32    = ImGui::GetColorU32(statusColor);

        ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, colorU32);

        ImGui::Dummy(ImVec2(radius * 1.5f, 0.0f));

        ImGui::SameLine();
        ImGui::Text("CANdle");

        ImGui::SameLine();
        ImGui::Text("|");

        ImGui::SameLine();
        ImGui::TextColored(statusColor, "%s", statusText);
    }
    ImGui::EndChild();
    GuiStyle::endChildInnerStyle();
}

void BusActuatorMenu::interfaceInfo()
{
    GuiStyle::childInnerStyle();
    if (ImGui::BeginChild(
            "Interface", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
    {
        ImGui::TextUnformatted(m_data->busTypeToString(m_data->busType));
    }
    ImGui::EndChild();
    GuiStyle::endChildInnerStyle();
}

void BusActuatorMenu::baudrateInfo()
{
    GuiStyle::childInnerStyle();
    if (ImGui::BeginChild(
            "Baudrate", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
    {
        ImGui::TextUnformatted(m_data->baudrateToString(m_data->candleDatarate));
    }
    ImGui::EndChild();
    GuiStyle::endChildInnerStyle();
}

void BusActuatorMenu::drawTestGroupsMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("Test Groups Menu",
                          ImVec2(GuiStyle::leftMenuBarWidth, -1),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;
        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::PushFont(nullptr, 18.0f);
        ImGui::Text("Test Groups");
        ImGui::PopFont();

        drawGroupButtons();
        drawAddGroupButton();
        drawAddGroupPopup();

        GuiStyle::buttonStyle();
        if (ImGui::Button("Select All",
                          ImVec2(candleInfoButtonWidth.x, GuiStyle::mediumButtonHeight)))
            for (auto& group : MDGroups)
                group.isSelected = true;
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(candleInfoButtonWidth.x, GuiStyle::mediumButtonHeight)))
            for (auto& group : MDGroups)
                group.isSelected = false;
        GuiStyle::endButtonStyle();

        if (testOngoing)
        {
            ImGui::EndDisabled();
        }
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void BusActuatorMenu::drawFoundOnBusMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild("##Found On Bus Menu",
                          ImVec2(GuiStyle::middleWindowWidth, -1),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        ImGui::PushFont(nullptr, 18.0f);
        ImGui::Text("Found On the Bus");
        ImGui::PopFont();

        ImGui::Separator();

        std::vector<MDStatus_S> mdsOnTheBus;
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            mdsOnTheBus = m_data->mdStatus;
        }

        GuiStyle::tableStyle();
        if (ImGui::BeginTable(
                "##FoundTable", 7, GuiStyle::flagsTable, ImGui::GetContentRegionAvail()))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Group", ImGuiTableColumnFlags_WidthStretch, 1.2f);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthStretch, 0.6f);
            ImGui::TableSetupColumn("Model", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Firmware", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Calibration", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Notes", ImGuiTableColumnFlags_WidthStretch, 0.8f);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            for (const MDStatus_S& md : mdsOnTheBus)
            {
                ImGui::PushID(static_cast<int>(md.id));
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("—");  // Group

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%u", static_cast<unsigned>(md.id));  // ID

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("—");  // Model

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("—");  // Firmware

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("—");  // Calibration

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(md.ok ? "OK" : "Error");  // Notes

                ImGui::TableNextColumn();
                GuiStyle::comboStyle(nullptr, 3.0f);
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##Choose Action", "Select"))
                {
                    if (ImGui::Selectable("Add to group"))
                    {
                        chosenAction    = ActionType_E::ADD_TO_GROUP;
                        chosenGroupName = defaultGroupName;
                        idCopy          = md.id;
                    }
                    if (ImGui::Selectable("Set Limits"))
                    {
                        chosenAction    = ActionType_E::SET_LIMITS;
                        chosenGroupName = defaultGroupName;
                        idCopy          = md.id;
                    }
                    ImGui::EndCombo();
                }
                GuiStyle::endComboStyle();

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        GuiStyle::endTableStyle();
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();
}

void BusActuatorMenu::drawActionMenu()
{
    GuiStyle::childOuterStyle();
    if (ImGui::BeginChild(
            "##Action Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, GuiStyle::flagsBackMenu))
    {
        ImGui::PushFont(nullptr, 18.0f);
        ImGui::Text("Action Menu");
        ImGui::PopFont();

        switch (chosenAction)
        {
            case ActionType_E::ADD_TO_GROUP:
                drawAddToGroupMenu();
                break;
            case ActionType_E::SET_LIMITS:
                break;
            default:
                break;
        }
    }
    ImGui::EndChild();
    GuiStyle::endChildOuterStyle();

    actionMenuWidth = ImGui::GetItemRectSize().x;
}

void BusActuatorMenu::drawAddToGroupMenu()
{
    ImGui::Text("Chosen MD");
    GuiStyle::childInnerStyle();
    if (ImGui::BeginChild(
            "##Chosen ID", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
    {
        ImGui::Text("MD%u", static_cast<unsigned>(idCopy));
    }
    ImGui::EndChild();

    GuiStyle::comboStyle("Select Group");
    ImGui::SetCursorPosX(GuiStyle::leftMenuPadding);
    ImGui::SetNextItemWidth(actionMenuWidth - 2 * GuiStyle::leftMenuPadding);
    if (ImGui::BeginCombo("##Group select", chosenGroupName.c_str()))
    {
        if (ImGui::Selectable("None"))
        {
            chosenGroupName = defaultGroupName;
        }
        for (size_t i = 0; i < MDGroups.size(); i++)
        {
            MDGroup_S group = MDGroups[i];

            if (ImGui::Selectable(group.groupName.c_str()))
            {
                chosenGroupName = group.groupName;
            }
        }
        ImGui::EndCombo();
    }
    GuiStyle::endChildInnerStyle();
    GuiStyle::endComboStyle();
}

/*

Draw buttons

*/

void BusActuatorMenu::drawScanBusButton()
{
    std::string label = "";

    if (m_data->discoverOngoing)
    {
        ImGui::BeginDisabled();
        double  time       = ImGui::GetTime();
        uint8_t dotCounter = (uint8_t)(time * 2.5) % 4;

        label = "Scanning" + std::string("...").substr(0, dotCounter);
    }
    else
    {
        label = "Scan Bus";
    }

    GuiStyle::buttonStyle();
    if (ImGui::Button(label.c_str(), candleInfoButtonWidth))
    {
        m_data->buttonDiscoverMdPressed = true;
    }
    GuiStyle::endButtonStyle();
    if (m_data->discoverOngoing)
    {
        ImGui::EndDisabled();
    }
}

void BusActuatorMenu::drawGroupButtons()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, GuiStyle::roundingFrameButton);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));

    for (size_t i = 0; i < MDGroups.size(); ++i)
    {
        MDGroup_S& group = MDGroups[i];
        ImGui::PushID(static_cast<int>(i));

        if (group.isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, GuiStyle::mabColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GuiStyle::mabColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, GuiStyle::mabColor);
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, GuiStyle::buttonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GuiStyle::buttonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, GuiStyle::buttonColor);
        }

        std::string labelForButton =
            MDGroups[i].groupName + " (" + std::to_string(MDGroups[i].mdIds.size()) + ")";

        if (ImGui::Button(labelForButton.c_str(),
                          ImVec2(GuiStyle::leftMenuWideButtonWidth, GuiStyle::largeButtonHeight)))
            group.isSelected = !group.isSelected;

        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    ImGui::PopStyleVar(2);
}

void BusActuatorMenu::drawAddGroupButton()
{
    GuiStyle::buttonInvisibleStyle();
    if (ImGui::Button("+ Add group",
                      ImVec2(GuiStyle::leftMenuWideButtonWidth, GuiStyle::largeButtonHeight)))
    {
        newGroupName[0] = '\0';
        creatingGroupError.clear();
        addGroupPopupOpen = true;
        ImGui::OpenPopup("Create MDs Group");
    }
    GuiStyle::endButtonInvisibleStyle();
}

/*

Helper Functions

*/

bool BusActuatorMenu::groupNameCheck(const std::string& rawName)
{
    const auto first = rawName.find_first_not_of(" \t");
    if (first == std::string::npos)
    {
        creatingGroupError = "Name cannot be empty";
        return false;
    }
    const auto  last = rawName.find_last_not_of(" \t");
    std::string name = rawName.substr(first, last - first + 1);

    bool exists = std::any_of(MDGroups.begin(),
                              MDGroups.end(),
                              [&](const MDGroup_S& group) { return group.groupName == name; });
    if (exists)
    {
        creatingGroupError = "Group \"" + name + "\" already exists";
        return false;
    }

    MDGroups.push_back({name, {}});
    creatingGroupError.clear();
    return true;
}

void BusActuatorMenu::drawAddGroupPopup()
{
    GuiStyle::popupStyle();
    if (ImGui::BeginPopupModal(
            "Create MDs Group", &addGroupPopupOpen, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Group name:");

        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();

        bool submit = ImGui::InputText("##groupName",
                                       newGroupName,
                                       sizeof(newGroupName),
                                       ImGuiInputTextFlags_EnterReturnsTrue);

        if (!creatingGroupError.empty())
            ImGui::TextColored(GuiStyle::redColor, "%s", creatingGroupError.c_str());

        ImGui::Separator();

        GuiStyle::buttonStyle();
        if (ImGui::Button("OK", ImVec2(GuiStyle::decisionButtonWidth, 0.0f)))
            submit = true;

        if (submit && groupNameCheck(newGroupName))
            ImGui::CloseCurrentPopup();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(GuiStyle::decisionButtonWidth, 0.0f)) ||
            ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseCurrentPopup();

        GuiStyle::endButtonStyle();
        ImGui::EndPopup();
    }
    GuiStyle::endPopupStyle();
}
