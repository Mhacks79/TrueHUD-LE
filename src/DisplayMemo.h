#pragma once
namespace TrueHUDLE {
    // Identity token only; never dereference a previous movie/clip address.
    struct DisplayMemo {
        const void* object = nullptr;
        double x = 0, y = 0, scale = 0;
        bool visible = false, known = false;
        bool Matches(const void* current, double px, double py, double size, bool shown) const {
            return known && object == current && x == px && y == py && scale == size && visible == shown;
        }
        void Remember(const void* current, double px, double py, double size, bool shown) {
            object = current; x = px; y = py; scale = size; visible = shown; known = true;
        }
        void Reset() { known = false; object = nullptr; }
    };
}
