#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "component.h"
#include "engine/animation/animation_clip.h"

namespace hob {
    class SpriteComponent;

    class SpriteAnimatorComponent : public Component {
        SpriteComponent* m_sprite = nullptr;
        std::unordered_map<std::string, std::shared_ptr<AnimationClip>> m_clips;
        std::shared_ptr<AnimationClip> m_current_clip;
        std::string m_current_clip_name;
        std::string m_default_clip_name;
        float m_elapsed = 0.0f;
        int m_frame_index = 0;
        bool m_playing = false;

    public:
        explicit SpriteAnimatorComponent(Entity& entity);

        std::string to_string() const override;

        void enter_play() override;
        void tick(float delta_time) override;

        void add_clip(const std::string& name, std::shared_ptr<AnimationClip> clip);
        void set_default_clip(const std::string& name);
        const std::string& get_default_clip() const;
        const std::string& get_current_clip() const;
        int get_current_frame() const;

        void play(const std::string& name);
        void resume();
        void pause();
        void stop();

        bool is_playing() const;
    };
}
