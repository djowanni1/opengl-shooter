#ifndef MAIN_SPRITES_H
#define MAIN_SPRITES_H

#include "LiteMath.h"

extern float deltaTime;

struct Sprite{
    unsigned texture;
    int stride_x, stride_y;
    int count_frames;
    Sprite(unsigned texture, int stride_x, int stride_y, int count_frames)
        : texture(texture), stride_x(stride_x), stride_y(stride_y), count_frames(count_frames){}
};

class SpriteAnimator {
protected:
    int count_frames;
    int stride_x, stride_y;
    float scale_x, scale_y;
    float fps;
    float step;
    virtual void UpdateStep() = 0;

public:
    unsigned texture;
    explicit SpriteAnimator(Sprite &init);

    LiteMath::float3x3 animation() {
        UpdateStep();
        int off = int(step);
        float arr[] = {
                scale_x, 0.0, off % stride_x * scale_x,
                0.0, scale_y, off / stride_x * scale_y,
                0.0, 0.0, 1.0
        };
        return LiteMath::float3x3(arr);
    }
};

class Explosion : public SpriteAnimator{
    void UpdateStep () override {
        step += deltaTime * fps;
        if (step > count_frames) {
            step = count_frames - 1;
        }
    }
public:
    explicit Explosion(Sprite &init);
    void new_boom();
};

class Asteroid : public SpriteAnimator{
private:
    Explosion boom;
    LiteMath::float3 direction;
    void UpdateStep () override {
        step += deltaTime * fps;
        if (step > count_frames) {
            step -= count_frames;
        }
    }

public:
    bool is_alive;
    time_t time_of_death = 0;
    LiteMath::float3 position;

    Asteroid(Sprite &astro, Sprite &boom);

    LiteMath::float3x3 animate();

    void kill();
    void respawn();
};

#endif //MAIN_SPRITES_H
