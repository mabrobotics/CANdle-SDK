#include "mainWindow.hpp"

#include "imgui.h"
#include "implot.h"
#include <cmath>
#include <cstdio>

#define SAMPLES 512
float ref[SAMPLES];
float act[SAMPLES];
float kp = 0.;
float kd = 0.;

float getKp(mab::Candle& candle, u16 id)
{
    float value = 0.;
    candle.readMd80Register(id, mab::Md80Reg_E::motorImpPidKp, value);
    return value;
}
float getKd(mab::Candle& candle, u16 id)
{
    float value = 0.;
    candle.readMd80Register(id, mab::Md80Reg_E::motorImpPidKd, value);
    return value;
}
bool setKp(mab::Candle& candle, u16 id, float value)
{
    return candle.writeMd80Register(id, mab::Md80Reg_E::motorImpPidKp, value);
}
bool setKd(mab::Candle& candle, u16 id, float value)
{
    return candle.writeMd80Register(id, mab::Md80Reg_E::motorImpPidKd, value);
}
bool enable(mab::Candle& candle, u16 id)
{
    return (candle.controlMd80Mode(id, mab::Md80Mode_E::IMPEDANCE) &&
            candle.controlMd80Enable(id, true));
}
float getPosition(mab::Candle& candle, u16 id)
{
    float pos = 0.;
    candle.readMd80Register(id, mab::Md80Reg_E::mainEncoderPosition, pos);
    return pos;
}
bool setTargetPosition(mab::Candle& candle, u16 id, float target)
{
    return candle.writeMd80Register(id, mab::Md80Reg_E::targetPosition, target);
}

void doStep(mab::Candle& candle, u16 id)
{
    float startPos  = getPosition(candle, id);
    float targetPos = startPos + 1.0;
    enable(candle, id);
    for (u32 i = 0; i < SAMPLES; i++)
    {
        float target = startPos;
        if (i > 50)
            target = targetPos;
        ref[i] = target;
        act[i] = getPosition(candle, id);
        setTargetPosition(candle, id, target);
    }
}

void drawMainWindow(mab::Candle& candle)
{
    static u16              idSelected = 0;
    static std::vector<u16> idsPinged;

    ImGui::Begin("CANdleGUI");
    ImGui::BeginTable("split", 2);
    ImGui::TableNextColumn();
    if (ImGui::Button("PING"))
        idsPinged = candle.ping();
    ImGui::BeginListBox("", {-1, 200});
    for (auto& id : idsPinged)
    {
        char idText[16];
        sprintf(idText, "%d", id);
        if (ImGui::Button(idText))
        {
            candle.addMd80(id);
            kp         = getKp(candle, id);
            kd         = getKd(candle, id);
            idSelected = id;
        }
    }
    ImGui::EndListBox();
    ImGui::TableNextColumn();
    ImGui::Text("MD with ID: %d", idSelected);
    if (idSelected > 10)
    {
        if (ImGui::InputFloat("Kp", &kp, 0.1f, ImGuiInputTextFlags_EnterReturnsTrue))
            setKp(candle, idSelected, kp);
        if (ImGui::InputFloat("Kd", &kd, 0.001f))
            setKd(candle, idSelected, kd);
        if (ImGui::Button("STEP!"))
            doStep(candle, idSelected);
        ImPlot::BeginPlot("Tuning");
        ImPlot::PlotLine("Reference", ref, SAMPLES);
        ImPlot::PlotLine("Actual", act, SAMPLES);
        ImPlot::EndPlot();
    }

    ImGui::EndTable();
    ImGui::End();
}
