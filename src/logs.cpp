#include "../vendor/imgui/imgui.h"
#include "logs.h"
#include <iostream>

ImGuiTextBuffer Buf;
bool AutoScroll; // Keep scrolling if already at the bottom.

bool Logs::Visible = true;

void Clear()
{
    Buf.clear();
}

void Logs::Push(const char* fmt, ...) IM_FMTARGS(2)
{
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
}

void Logs::Draw()
{
    if (!ImGui::Begin("Logs"))
    {
        ImGui::End();
        return;
    }

    // Options menu
    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::EndPopup();
    }

    // Main window
    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
    ImGui::SameLine();

    bool clear = ImGui::Button("Clear");
    ImGui::SameLine();

    bool copy = ImGui::Button("Copy");

    ImGui::Separator();

    if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (clear) Clear();
        if (copy) ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::TextUnformatted(Buf.begin());
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }
    }

    ImGui::EndChild();
    ImGui::End();
}