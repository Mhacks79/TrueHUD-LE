#include "DebugDraw.h"
#include <algorithm>
#include <cmath>

namespace TrueHUDLE {
    namespace {
        constexpr float Pi = 3.14159265358979323846f;
        SRWLOCK drawingLock = SRWLOCK_INIT;
        DebugLine lines[MaxDebugLines];
        unsigned count = 0;
        bool Finite(const NiPoint3& v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); }
        NiPoint3 Cross(const NiPoint3& a, const NiPoint3& b) {
            return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
        }
        bool Unit(NiPoint3& v) {
            const float length = std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
            if (!std::isfinite(length) || length < 1e-6f) return false;
            v = v / length; return true;
        }
        bool Basis(NiPoint3 z, NiPoint3& x, NiPoint3& y) {
            if (!Unit(z)) return false;
            const NiPoint3 seed = std::fabs(z.z) < .9f ? NiPoint3(0,0,1) : NiPoint3(0,1,0);
            x = Cross(seed,z); if (!Unit(x)) return false;
            y = Cross(z,x); return true;
        }
        bool Axes(const NiQuaternion& rotation, NiPoint3& x, NiPoint3& y, NiPoint3& z) {
            const float norm = rotation.m_fW*rotation.m_fW+rotation.m_fX*rotation.m_fX+
                rotation.m_fY*rotation.m_fY+rotation.m_fZ*rotation.m_fZ;
            if (!std::isfinite(norm) || norm < 1e-12f) return false;
            const float s = 2/norm, qx=rotation.m_fX, qy=rotation.m_fY, qz=rotation.m_fZ, qw=rotation.m_fW;
            x = {1-s*(qy*qy+qz*qz), s*(qx*qy+qw*qz), s*(qx*qz-qw*qy)};
            y = {s*(qx*qy-qw*qz), 1-s*(qx*qx+qz*qz), s*(qy*qz+qw*qx)};
            z = {s*(qx*qz+qw*qy), s*(qy*qz-qw*qx), 1-s*(qx*qx+qy*qy)};
            return Finite(x) && Finite(y) && Finite(z);
        }
        unsigned Segments(UInt32 value) { return static_cast<unsigned>(std::clamp<UInt32>(value, 4, 64)); }
    }
    void ResetDebugDrawing() {
        AcquireSRWLockExclusive(&drawingLock); count = 0; ReleaseSRWLockExclusive(&drawingLock);
    }
    unsigned CopyDebugDrawing(DebugLine* output, unsigned capacity, ULONGLONG now) {
        if (!output || !capacity) return 0;
        unsigned copied = 0, kept = 0;
        AcquireSRWLockExclusive(&drawingLock);
        for (unsigned i = 0; i < count; ++i) {
            const DebugLine line = lines[i];
            if (line.expires && now >= line.expires) continue;
            if (copied < capacity) output[copied++] = line;
            // Zero duration means exactly one rendering attempt. Durable lines
            // keep their absolute deadline, so new lines cannot reset old TTLs.
            if (line.expires) lines[kept++] = line;
        }
        count = kept;
        ReleaseSRWLockExclusive(&drawingLock); return copied;
    }
    void DebugLine3D(const NiPoint3& a, const NiPoint3& b, float duration, UInt32 color, float thickness, float pointSize) {
        if (!Finite(a) || !Finite(b) || !std::isfinite(duration) || !std::isfinite(thickness) ||
            !std::isfinite(pointSize) || thickness < 0 || pointSize < 0) return;
        DebugLine line; line.start = a; line.end = b; line.color = color;
        line.thickness = std::clamp(thickness, .1f, 100.f); line.pointSize = std::min(pointSize, 1000.f);
        line.expires = duration > 0 ? GetTickCount64() + static_cast<ULONGLONG>(std::min(duration, 3600.f)*1000) + 1 : 0;
        AcquireSRWLockExclusive(&drawingLock);
        if (count < MaxDebugLines) lines[count++] = line;
        ReleaseSRWLockExclusive(&drawingLock);
    }
    void DebugArrow(const NiPoint3& a, const NiPoint3& b, float size, float duration, UInt32 color, float thickness) {
        if (!std::isfinite(size) || size < 0) return;
        DebugLine3D(a,b,duration,color,thickness);
        NiPoint3 z=b-a, x, y; if (!Unit(z) || !Basis(z,x,y)) return;
        DebugLine3D(b,b-z*size+x*(size*.5f),duration,color,thickness);
        DebugLine3D(b,b-z*size-x*(size*.5f),duration,color,thickness);
    }
    void DebugBox(const NiPoint3& center, const NiPoint3& extent, const NiQuaternion& rotation, float duration, UInt32 color, float thickness) {
        NiPoint3 x,y,z;
        if (!Finite(center) || !Finite(extent) || !Axes(rotation,x,y,z)) return;
        NiPoint3 points[8];
        for (unsigned i=0;i<8;++i) points[i]=center+x*((i&1)?extent.x:-extent.x)+
            y*((i&2)?extent.y:-extent.y)+z*((i&4)?extent.z:-extent.z);
        for (unsigned i=0;i<8;++i) for (unsigned bit=1;bit<=4;bit<<=1)
            if (!(i&bit)) DebugLine3D(points[i],points[i|bit],duration,color,thickness);
    }
    void DebugCircle(const NiPoint3& center, const NiPoint3& x, const NiPoint3& y, float radius, UInt32 segments,
                     float duration, UInt32 color, float thickness, bool half) {
        if (!std::isfinite(radius) || radius < 0) return;
        const unsigned n=Segments(segments); const float total=half?Pi:2*Pi;
        NiPoint3 previous=center+x*radius;
        for (unsigned i=1;i<=n;++i) {
            const float angle=total*float(i)/n;
            const NiPoint3 next=center+(x*std::cos(angle)+y*std::sin(angle))*radius;
            DebugLine3D(previous,next,duration,color,thickness); previous=next;
        }
    }
    void DebugSphere(const NiPoint3& center, float radius, UInt32 segments, float duration, UInt32 color, float thickness) {
        // Latitude and longitude rings preserve the wire-sphere semantics.
        const unsigned n=Segments(segments);
        for (unsigned j=0;j<n/2;++j) {
            const float angle=Pi*2*float(j)/n;
            DebugCircle(center,{std::cos(angle),std::sin(angle),0},{0,0,1},radius,n,duration,color,thickness);
        }
        for (unsigned j=1;j<n/2;++j) {
            const float angle=Pi*float(j)/float(n/2);
            DebugCircle(center+NiPoint3(0,0,std::cos(angle)*radius),{1,0,0},{0,1,0},
                std::sin(angle)*radius,n,duration,color,thickness);
        }
    }
    void DebugCylinder(const NiPoint3& a, const NiPoint3& b, float radius, UInt32 segments, float duration, UInt32 color, float thickness) {
        NiPoint3 x,y; if (!Basis(b-a,x,y) || !std::isfinite(radius) || radius < 0) return;
        const unsigned n=Segments(segments);
        DebugCircle(a,x,y,radius,n,duration,color,thickness); DebugCircle(b,x,y,radius,n,duration,color,thickness);
        for (unsigned i=0;i<n;++i) {
            const float angle=2*Pi*float(i)/n;
            const NiPoint3 offset=(x*std::cos(angle)+y*std::sin(angle))*radius;
            DebugLine3D(a+offset,b+offset,duration,color,thickness);
        }
    }
    void DebugCone(const NiPoint3& origin, const NiPoint3& direction, float length, float width, float height,
                   UInt32 segments, float duration, UInt32 color, float thickness) {
        NiPoint3 z=direction,x,y;
        if (!Unit(z) || !Basis(z,x,y) || !std::isfinite(length) || !std::isfinite(width) || !std::isfinite(height) || length < 0) return;
        const float sx=std::sin(std::clamp(height,.0001f,Pi-.0001f)*.5f);
        const float sy=std::sin(std::clamp(width,.0001f,Pi-.0001f)*.5f);
        const unsigned n=Segments(segments); NiPoint3 previous;
        for (unsigned i=0;i<=n;++i) {
            const float angle=2*Pi*float(i%n)/n;
            const float phi=std::atan2(std::sin(angle)*sy,std::cos(angle)*sx), sp=std::sin(phi), cp=std::cos(phi);
            const float r2=sx*sx*sy*sy/(sx*sx*sp*sp+sy*sy*cp*cp), r=std::sqrt(r2), q=std::sqrt(1-r2);
            const NiPoint3 next=origin+(z*(1-2*r2)+x*(2*q*r*cp)+y*(2*q*r*sp))*length;
            if (i<n) DebugLine3D(origin,next,duration,color,thickness);
            if (i) DebugLine3D(previous,next,duration,color,thickness);
            previous=next;
        }
    }
    void DebugCapsule(const NiPoint3& a, const NiPoint3& b, float radius, float duration, UInt32 color, float thickness) {
        if (!std::isfinite(radius) || radius < 0) return;
        NiPoint3 z=b-a,x,y;
        if (!Unit(z)) { DebugSphere(a,radius,16,duration,color,thickness); return; }
        if (!Basis(z,x,y)) return;
        DebugCylinder(a,b,radius,16,duration,color,thickness);
        DebugCircle(a,x,-z,radius,16,duration,color,thickness,true);
        DebugCircle(a,y,-z,radius,16,duration,color,thickness,true);
        DebugCircle(b,x,z,radius,16,duration,color,thickness,true);
        DebugCircle(b,y,z,radius,16,duration,color,thickness,true);
    }
    void DebugCapsule(const NiPoint3& origin, float halfHeight, float radius, const NiQuaternion& rotation,
                      float duration, UInt32 color, float thickness) {
        NiPoint3 x,y,z;
        if (!std::isfinite(halfHeight) || !std::isfinite(radius) || !Axes(rotation,x,y,z)) return;
        const float offset=std::max(0.f,halfHeight-radius);
        DebugCapsule(origin-z*offset,origin+z*offset,radius,duration,color,thickness);
    }
}
