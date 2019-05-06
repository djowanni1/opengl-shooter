//#ifndef MAIN_SPRITES_H
//#define MAIN_SPRITES_H
//
//class SpriteAnimator {
//private:
//    int count_frames;
//    int stride_x, stride_y;
//    float scale_x, scale_y;
//    float fps;
//    float step;
//    bool onetime;
//    void UpdateStep() {
//        step += deltaTime * fps;
//        if (step > count_frames) {
//            if (onetime){
//                step = count_frames - 1;
//                return;
//            }
//            step -= count_frames;
//        }
//    }
//
//public:
//    GLuint texture;
//    SpriteAnimator(const char *path, int stride_x, int stride_y, int count_frames, bool onetime = false)
//            : texture(loadTexture(path)), stride_x(stride_x), stride_y(stride_y), scale_x(1.0 / stride_x),
//              scale_y(1.0 / stride_y), count_frames(count_frames), step(0), onetime(onetime) {
//        std::uniform_real_distribution<> dis(10, 25);
//        fps = dis(gen);
//    }
//
//    SpriteAnimator(const SpriteAnimator &other, bool onetime)
//            : texture(other.texture), stride_x(other.stride_x), stride_y(other.stride_y), scale_x(other.scale_x),
//              scale_y(other.scale_y), count_frames(other.count_frames), step(0), onetime(onetime) {
//        std::uniform_real_distribution<> dis(10, 25);
//        fps = dis(gen);
//    }
//
//    float3x3 animation() {
//        UpdateStep();
//        int off = int(step);
//        float arr[] = {
//                scale_x, 0.0, off % stride_x * scale_x,
//                0.0, scale_y, off / stride_y * scale_y,
//                0.0, 0.0, 1.0
//        };
//        return float3x3(arr);
//    }
//};
//
//class Explosion : public SpriteAnimator{
//
//};
//
//class Enemy {
//private:
//
//public:
//    SpriteAnimator anim;
//    SpriteAnimator explosion;
//    bool is_alive;
//    float3 direction;
//    float3 position;
//
//    Enemy(SpriteAnimator &anim, SpriteAnimator &explosion)
//            : anim(anim, false), explosion(explosion, true), is_alive(true) {
//        std::uniform_real_distribution<float> dis(-10, 10);
//        position = float3(dis(gen), dis(gen), -50.0f + dis(gen));
//        direction = cameraPos - position;
//    };
//
//    float3x3 animate() {
//        if (is_alive){
//            GLfloat speed = 0.1f * deltaTime;
//            position += direction * speed;
//            return anim.animation();
//        } else {
//            return explosion.animation();
//        }
//
//    }
//};
//
//
//#endif //MAIN_SPRITES_H
#include "sprites.h"
#include <ctime>
#include <random>

std::mt19937 gen(time(0));

extern LiteMath::float3 cameraPos;

SpriteAnimator::SpriteAnimator(Sprite &init)
        : texture(init.texture), stride_x(init.stride_x), stride_y(init.stride_y),
          count_frames(init.count_frames), scale_x(1.0 / init.stride_x),
          scale_y(1.0 / init.stride_y), step(0) {
    std::uniform_real_distribution<> dis(10, 25);
    fps = dis(gen);
}

Explosion::Explosion(Sprite &init) : SpriteAnimator(init) {
    fps = 35.0;
}


Asteroid::Asteroid(Sprite &astro, Sprite &boom) : SpriteAnimator(astro), boom(boom), is_alive(true) {
    std::uniform_real_distribution<float> dis(-10, 10);
    position = LiteMath::float3(dis(gen), dis(gen), -50.0f + dis(gen));
    direction = cameraPos - position;
}

LiteMath::float3x3 Asteroid::animate() {
    if (is_alive) {
        position += direction * 0.1f * deltaTime;
        return animation();
    } else {
        return boom.animation();
    }
}
