#include "shadows.hpp"

#include "component/component.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include <array>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace soft_tissues::shadows {

class ShadowMap {
private:
    unsigned int fbo;
    unsigned int texture;

public:
    ShadowMap() {}

    void load() {
        this->fbo = rlLoadFramebuffer();
        this->texture = rlLoadTextureDepth(
            globals::SHADOW_MAP_SIZE, globals::SHADOW_MAP_SIZE, true
        );
        rlFramebufferAttach(
            this->fbo, this->texture, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0
        );

        if (!rlFramebufferComplete(this->fbo)) {
            throw std::runtime_error("Failed to create shadow map fbo");
        }
    }

    void unload() {
        rlUnloadFramebuffer(this->fbo);
        rlUnloadTexture(this->texture);
    }
};

static std::array<ShadowMap, globals::MAX_N_SHADOW_MAPS> SHADOW_MAPS;

void load() {
    for (auto &shadow_map : SHADOW_MAPS) {
        shadow_map.load();
    }
}

void unload() {
    for (auto &shadow_map : SHADOW_MAPS) {
        shadow_map.unload();
    }
}

void draw() {
    static std::vector<component::Light *> lights;
    lights.clear();

    // collect lights-shadow casters
    for (auto entity : globals::registry.view<component::Light>()) {
        if (lights.size() == SHADOW_MAPS.size()) break;

        auto &light = globals::registry.get<component::Light>(entity);
        if (!light.is_enabled || !light.casts_shadows) continue;

        lights.push_back(&light);
    }

    // draw shadows
    for (auto light : lights) {
        printf("TODO: Draw shadow maps");
    }
}

}  // namespace soft_tissues::shadows
