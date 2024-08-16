#pragma once

namespace soft_tissues::shadows {

class ShadowMap {
public:
    unsigned int fbo;
    unsigned int texture;

    ShadowMap();

    void load();
    void unload();
};

void load();
void unload();

void draw();

}  // namespace soft_tissues::shadows
