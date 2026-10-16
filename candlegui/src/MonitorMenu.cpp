#include "MonitorMenu.hpp"
#include "BusActuatorsMenu.hpp"
#include "commonMemory.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
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
                          ImVec2(GuiStyle::leftMenuBarWidth, 0),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        refreshMDStatus();
        drawActuatorMenu();
        drawCanBusInfoMenu();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    if (ImGui::BeginChild("Middle Column Monitor",
                          ImVec2(GuiStyle::middleWindowWidth, 0),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        refreshMDStatus();
        drawPlotsMenu();
        drawLiveValuesMenu();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    drawPlaceHolderMenu();
}

void MonitorMenu::drawActuatorMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding / 2, GuiStyle::insideWindowPadding));
    if (ImGui::BeginChild("Actuators Menu",
                          ImVec2(GuiStyle::leftMenuBarWidth, -canBusInfoBarHeight),
                          ImGuiChildFlags_Borders,
                          GuiStyle::flagsBackMenu))
    {
        bool testOngoing = m_data->testOngoing;

        if (testOngoing)
            ImGui::BeginDisabled();

        ImGui::Text("Actuators");

        ImGui::PushFont(nullptr, 18.0f);
        float statusX = ImGui::GetFontSize() * 8.0f;

        if (!MonitorMenuMDStatus.empty())
        {
            for (size_t i = 0; i < MonitorMenuMDStatus.size(); i++)
            {
                const MDStatus_S& status = MonitorMenuMDStatus[i];

                if (std::isnan(status.driverTemp))
                    ImGui::Text("ID %u   --", (unsigned)status.id);
                else
                    ImGui::Text("ID %u   %.1f C", (unsigned)status.id, status.driverTemp);

                ImGui::SameLine(statusX);

                if (!status.valid)
                    ImGui::TextDisabled("--");
                else if (status.ok)
                    ImGui::TextColored(GuiStyle::greenColor, "OK");
                else
                    ImGui::TextColored(GuiStyle::redColor, "ERR");
            }
        }
        ImGui::PopFont();

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
                        ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding / 2, GuiStyle::insideWindowPadding));
    if (ImGui::BeginChild(
            "Can Bus Info Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, GuiStyle::flagsBackMenu))
    {
        ImGui::Text("CAN Bus Info");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawPlotsMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding, GuiStyle::insideWindowPadding));

    if (ImGui::BeginChild("##Plots Menu",
                          ImVec2(GuiStyle::middleWindowWidth, -liveValuesMenuHeight),
                          ImGuiChildFlags_None,
                          GuiStyle::flagsBackMenu))
    {
        const ImVec2 avail   = ImGui::GetContentRegionAvail();
        const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
        const ImVec2 cellSize((avail.x - spacing.x) * 0.5f, (avail.y - spacing.y) * 0.5f);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                            ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding));

        if (ImGui::BeginChild("##Plot1", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("Velocity");
            ImGui::PopFont();
            drawVelocityPlots();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("##Plot2", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("Position");
            ImGui::PopFont();
            drawPositionPlots();
        }
        ImGui::EndChild();

        if (ImGui::BeginChild("##Plot3", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("Torque");
            ImGui::PopFont();
            drawTorquePlots();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("##Plot4", cellSize, ImGuiChildFlags_Borders))
        {
            ImGui::PushFont(nullptr, 18.0f);
            ImGui::Text("MOSFET temperature");
            ImGui::PopFont();
            drawMosfetTempPlots();
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawLiveValuesMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding / 2, GuiStyle::insideWindowPadding));

    if (ImGui::BeginChild(
            "##Live Values Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, GuiStyle::flagsBackMenu))
    {
        ImGui::Text("Live Values");

        ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;

        ImVec2 tableSize = ImVec2(0.0f, ImGui::GetContentRegionAvail().y);

        if (ImGui::BeginTable("##LiveValuesTable", 7, flags, tableSize))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Pos rad");
            ImGui::TableSetupColumn("Vel rad/s");
            ImGui::TableSetupColumn("Torque Nm");
            ImGui::TableSetupColumn("T motor C");
            ImGui::TableSetupColumn("T MOSFET C");
            ImGui::TableSetupColumn("Status");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < MonitorMenuMDStatus.size(); i++)
            {
                const MDStatus_S& status = MonitorMenuMDStatus[i];

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%u", (unsigned)status.id);

                ImGui::TableNextColumn();
                cellFloat(status.position, "%.3f");
                ImGui::TableNextColumn();
                cellFloat(status.velocity, "%.2f");
                ImGui::TableNextColumn();
                cellFloat(status.torque, "%.2f");
                ImGui::TableNextColumn();
                cellFloat(status.motorTemp, "%.1f");
                ImGui::TableNextColumn();
                cellFloat(status.driverTemp, "%.1f");

                ImGui::TableNextColumn();
                if (!status.valid)
                    ImGui::TextDisabled("--");
                else if (status.ok)
                    ImGui::TextColored(GuiStyle::greenColor, "OK");
                else
                    ImGui::TextColored(redColor, "ERR");
            }
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void MonitorMenu::drawPlaceHolderMenu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(GuiStyle::leftMenuPadding, GuiStyle::insideWindowPadding));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(GuiStyle::insideWindowPadding / 2, GuiStyle::insideWindowPadding));
    if (ImGui::BeginChild(
            "##Placeholder Menu", ImVec2(0, 0), ImGuiChildFlags_Borders, GuiStyle::flagsBackMenu))
    {
        ImGui::Text("Placeholders");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

/*

 Draw Plots

*/

void MonitorMenu::drawDevicePlot(PlotSignal_E signal, const char* plotId, const char* yLabel)
{
    const int devices = m_data->devicePlotsCount.load(std::memory_order_acquire);

    float startTime   = 0.0f;
    float currentTime = 0.0f;
    bool  first       = true;

    for (int i = 0; i < devices; ++i)
    {
        const DevicePlot_S& p = m_data->devicePlots[i];
        const uint32_t      n = p.writeData.load(std::memory_order_acquire);
        if (n == 0)
            continue;

        const float oldest = (n < DevicePlot_S::SIZE) ? p.time[0] : p.time[n % DevicePlot_S::SIZE];
        const float newest = p.time[(n - 1) % DevicePlot_S::SIZE];

        startTime   = first ? oldest : std::min(startTime, oldest);
        currentTime = first ? newest : std::max(currentTime, newest);
        first       = false;
    }
    if (currentTime <= startTime)
        currentTime = startTime + 1.0f;

    ImPlot::PushStyleColor(ImPlotCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    if (ImPlot::BeginPlot(plotId, ImGui::GetContentRegionAvail()))
    {
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        ImPlot::SetupAxis(ImAxis_X1, "Time [s]", ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisLimits(ImAxis_X1, startTime, currentTime, ImPlotCond_Always);

        ImPlot::SetupAxis(ImAxis_Y1, yLabel, ImPlotAxisFlags_NoHighlight | ImPlotAxisFlags_AutoFit);

        for (int i = 0; i < devices; ++i)
        {
            const DevicePlot_S& p = m_data->devicePlots[i];
            const uint32_t      n = p.writeData.load(std::memory_order_acquire);
            if (n == 0)
                continue;

            const int pointsCount = static_cast<int>(std::min(n, DevicePlot_S::SIZE));

            ImPlotSpec spec;
            spec.Offset = (n < DevicePlot_S::SIZE) ? 0 : static_cast<int>(n % DevicePlot_S::SIZE);

            char label[16];
            std::snprintf(label, sizeof(label), "ID %u", static_cast<unsigned>(p.id));

            ImPlot::PlotLine(label, p.time, p.signal(signal), pointsCount, spec);
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleColor();
}

/*

 Help functions

*/

void MonitorMenu::refreshMDStatus()
{
    std::lock_guard<std::mutex> lock(m_data->mtx);
    MonitorMenuMDStatus = m_data->mdStatus;
}

void MonitorMenu::cellFloat(float v, const char* fmt)
{
    if (std::isnan(v))
        ImGui::TextDisabled("--");
    else
        ImGui::Text(fmt, v);
}