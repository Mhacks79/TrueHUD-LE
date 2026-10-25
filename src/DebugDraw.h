#pragma once
#include "Runtime.h"
namespace TrueHUDLE {
    constexpr unsigned MaxDebugLines = 4096;
    struct DebugLine {
        NiPoint3 start, end;
        float thickness = 1, pointSize = 0;
        UInt32 color = 0xFF0000FF;
        ULONGLONG expires = 0;
    };
    void ResetDebugDrawing();
    unsigned CopyDebugDrawing(DebugLine* output, unsigned capacity, ULONGLONG now);
    void DebugLine3D(const NiPoint3& a, const NiPoint3& b, float duration, UInt32 color, float thickness, float pointSize = 0);
    void DebugArrow(const NiPoint3& a, const NiPoint3& b, float size, float duration, UInt32 color, float thickness);
    void DebugBox(const NiPoint3& center, const NiPoint3& extent, const NiQuaternion& rotation, float duration, UInt32 color, float thickness);
    void DebugCircle(const NiPoint3& center, const NiPoint3& x, const NiPoint3& y, float radius, UInt32 segments,
                     float duration, UInt32 color, float thickness, bool half = false);
    void DebugSphere(const NiPoint3& center, float radius, UInt32 segments, float duration, UInt32 color, float thickness);
    void DebugCylinder(const NiPoint3& a, const NiPoint3& b, float radius, UInt32 segments, float duration, UInt32 color, float thickness);
    void DebugCone(const NiPoint3& origin, const NiPoint3& direction, float length, float width, float height,
                   UInt32 segments, float duration, UInt32 color, float thickness);
    void DebugCapsule(const NiPoint3& a, const NiPoint3& b, float radius, float duration, UInt32 color, float thickness);
    void DebugCapsule(const NiPoint3& origin, float halfHeight, float radius, const NiQuaternion& rotation,
                      float duration, UInt32 color, float thickness);
}
