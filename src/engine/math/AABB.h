#ifndef HOB_ENGINE_AABB_H
#define HOB_ENGINE_AABB_H
#include "Vector2.h"


namespace hob {
    struct AABB {
        Vector2 center;
        Vector2 extents;

        AABB(const Vector2& center_, const Vector2& extents_);

        Vector2 min() const;
        Vector2 max() const;
        Vector2 size() const;
    };
}


#endif //HOB_ENGINE_AABB_H
