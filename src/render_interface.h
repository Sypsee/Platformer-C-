#pragma once

#include "assets.h"
#include "game_lib.h"


constexpr int MAX_TRANSFORMS = 1000;



struct OrthographicCamera2D
{
    float zoom = 1.f;
    Vec2 dimensions;
    Vec2 pos;
};



struct Transform
{
    IVec2 atlasOffset;
    IVec2 spriteSize;
    Vec2 pos;
    Vec2 size;
};

struct RenderData
{
    OrthographicCamera2D gameCamera;
    OrthographicCamera2D uiCamera;

    int transformCount;
    Transform transforms[MAX_TRANSFORMS];
};


static RenderData* renderData;



void draw_sprite(SpriteID spriteID, Vec2 pos)
{
    Sprite sprite = get_sprite(spriteID);

    Transform transform = {};
    transform.pos = pos - vec_2(sprite.spriteSize) / 2.f;
    transform.size = vec_2(sprite.spriteSize);
    transform.atlasOffset = sprite.atlasOffset;
    transform.spriteSize = sprite.spriteSize;

    renderData->transforms[renderData->transformCount++] = transform;
}