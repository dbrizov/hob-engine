#ifndef HOB_ENGINE_CAMERACOMPONENT_H
#define HOB_ENGINE_CAMERACOMPONENT_H
#include <cstdint>

#include "Component.h"
#include "engine/math/Vector2.h"


namespace hob {
    class CameraComponent : public Component {
        uint32_t m_logical_resolution_width = 0;
        uint32_t m_logical_resolution_height = 0;

        // EntitySpawner is a friend class.
        // There is only one camera, and it is spawned when the EntitySpawner is initialized
        friend class EntitySpawner;
        void init(uint32_t logical_resolution_width, uint32_t logical_resolution_height);

    public:
        explicit CameraComponent(Entity& entity);

        uint32_t get_logical_resolution_width() const;
        uint32_t get_logical_resolution_height() const;

        Vector2 world_to_screen(const Vector2& world_position) const;
        Vector2 world_to_screen(const Vector2& world_position, const Vector2& camera_position) const;
    };
}


#endif //HOB_ENGINE_CAMERACOMPONENT_H
