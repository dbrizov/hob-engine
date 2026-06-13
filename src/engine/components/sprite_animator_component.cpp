#include "sprite_animator_component.h"

#include <format>
#include <utility>

#include "engine/entity/entity.h"
#include "sprite_component.h"

namespace hob {
    SpriteAnimatorComponent::SpriteAnimatorComponent(Entity& entity)
        : Component(entity) {
    }

    void SpriteAnimatorComponent::enter_play() {
        m_sprite = get_entity().get_component<SpriteComponent>();

        if (!m_default_clip_name.empty() && m_clips.contains(m_default_clip_name)) {
            play(m_default_clip_name);
        }
    }

    void SpriteAnimatorComponent::tick(float delta_time) {
        if (!m_playing || m_current_clip == nullptr || m_sprite == nullptr) {
            return;
        }

        const int frame_count = static_cast<int>(m_current_clip->frames.size());
        if (frame_count == 0) {
            return;
        }

        const float fps = m_current_clip->fps;
        if (fps <= 0.0f) {
            return;
        }

        const float step = 1.0f / fps;
        m_elapsed += delta_time;

        bool frame_changed = false;
        while (m_elapsed >= step) {
            m_elapsed -= step;
            m_frame_index += 1;
            frame_changed = true;

            if (m_frame_index >= frame_count) {
                if (m_current_clip->looping) {
                    m_frame_index = 0;
                }
                else {
                    m_frame_index = frame_count - 1;
                    m_playing = false;
                    m_elapsed = 0.0f;
                    break;
                }
            }
        }

        if (frame_changed) {
            m_sprite->set_texture(m_current_clip->frames[m_frame_index].texture);
        }
    }

    std::string SpriteAnimatorComponent::to_string() const {
        return std::format("SpriteAnimatorComponent(entity_id = {})", get_entity().get_id());
    }

    void SpriteAnimatorComponent::add_clip(const std::string& name, AnimationClipRef clip) {
        m_clips[name] = std::move(clip);
    }

    const AnimationClips& SpriteAnimatorComponent::get_clips() const {
        return m_clips;
    }

    void SpriteAnimatorComponent::set_clips(AnimationClips clips) {
        m_clips = std::move(clips);
    }

    void SpriteAnimatorComponent::clear_clips() {
        m_clips.clear();
    }

    void SpriteAnimatorComponent::set_default_clip(const std::string& name) {
        m_default_clip_name = name;
    }

    const std::string& SpriteAnimatorComponent::get_default_clip() const {
        return m_default_clip_name;
    }

    const std::string& SpriteAnimatorComponent::get_current_clip() const {
        return m_current_clip_name;
    }

    int SpriteAnimatorComponent::get_current_frame() const {
        return m_frame_index;
    }

    void SpriteAnimatorComponent::play(const std::string& name) {
        auto it = m_clips.find(name);
        if (it == m_clips.end()) {
            return;
        }

        m_current_clip = it->second;
        m_current_clip_name = name;
        m_frame_index = 0;
        m_elapsed = 0.0f;
        m_playing = true;

        if (m_sprite != nullptr && !m_current_clip->frames.empty()) {
            m_sprite->set_texture(m_current_clip->frames[0].texture);
        }
    }

    void SpriteAnimatorComponent::resume() {
        if (m_current_clip != nullptr) {
            m_playing = true;
        }
    }

    void SpriteAnimatorComponent::pause() {
        m_playing = false;
    }

    void SpriteAnimatorComponent::stop() {
        m_playing = false;
        m_frame_index = 0;
        m_elapsed = 0.0f;
    }

    bool SpriteAnimatorComponent::is_playing() const {
        return m_playing;
    }
}
