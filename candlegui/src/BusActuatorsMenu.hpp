#pragma once
#include <memory>
#include <vector>

#include "commonMemory.hpp"
#include "GuiStyle.hpp"
#include "imgui.h"
#include "mab_types.hpp"

class BusActuatorMenu
{
  public:
    BusActuatorMenu(std::shared_ptr<commonMemory_S> commonMemory, ImGuiIO& io);

    void drawBusActuatorMenu();

  private:
    struct MDGroup_S
    {
        std::string               groupName;
        bool                      isSelected = false;
        std::vector<mab::canId_t> mdIds;
    };

    enum class ActionType_E
    {
        NONE,
        ADD_TO_GROUP,
        SET_LIMITS
    };
    ActionType_E chosenAction = ActionType_E::NONE;

    std::shared_ptr<commonMemory_S> m_data;
    ImGuiIO&                        m_io;

    std::string defaultGroupName = "Select MD Group";
    std::string chosenGroupName  = defaultGroupName;

    // Dimensions of windows
    float candleInfoMenuHeight = 240.0f;
    float actionMenuWidth;

    // Button Dimension
    ImVec2 candleInfoButtonWidth =
        ImVec2((GuiStyle::leftMenuWideButtonWidth - (GuiStyle::insideWindowPadding_x)) / 2,
               GuiStyle::largeButtonHeight);

    // Groups
    std::vector<MDGroup_S> MDGroups;
    mab::canId_t           idCopy = 0;

    // Add Group Popup
    char        newGroupName[64] = "";
    std::string creatingGroupError;
    bool        addGroupPopupOpen = false;

    // Bus & Actuators draw functions
    void candleInfo();
    void interfaceInfo();
    void baudrateInfo();

    // Draw Menu
    void drawCANdleInfoMenu();
    void drawTestGroupsMenu();
    void drawFoundOnBusMenu();
    void drawActionMenu();
    void drawAddToGroupMenu();

    // Draw Buttons
    void drawScanBusButton();
    void drawGroupButtons();
    void drawAddGroupButton();

    // Helper Functions
    bool groupNameCheck(const std::string& name);
    void drawAddGroupPopup();
};