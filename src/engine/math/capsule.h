#ifndef HOB_ENGINE_CAPSULE_H
#define HOB_ENGINE_CAPSULE_H
#include "vector2.h"


namespace hob {
    struct Capsule {
        Vector2 center_a;
        Vector2 center_b;
        float radius;

        Capsule(const Vector2& center_a_, const Vector2& center_b_, float radius_);

        float get_height() const;
    };
}


#endif //HOB_ENGINE_CAPSULE_H
