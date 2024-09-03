#pragma once

namespace Logs {
    extern bool Visible;
    void Push(const char* fmt, ...) IM_FMTARGS(2);
    void Draw();
}