#include "sprites.h"
#include <ctime>
#include <random>

extern std::mt19937 gen;

extern LiteMath::float3 cameraPos;

extern float health;

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

void Explosion::new_boom() {
    step = 0;
}


Asteroid::Asteroid(Sprite &astro, Sprite &boom) : SpriteAnimator(astro), boom(boom), is_alive(true) {
    std::uniform_real_distribution<float> dis(-5, 5);
    position = LiteMath::float3(dis(gen), dis(gen), -100.0f + dis(gen));
    direction = normalize(cameraPos - position);
}

LiteMath::float3x3 Asteroid::animate() {
    if (is_alive) {
        position += direction * 10 * deltaTime;
        if (position.z > cameraPos.z + 1){
            health -= 25;
            this->kill();
        }
        return animation();
    } else {
        return boom.animation();
    }
}

void Asteroid::kill() {
    is_alive = false;
    time_of_death = time(0);
}

void Asteroid::respawn() {
    is_alive = true;
    std::uniform_real_distribution<float> dis(-5, 5);
    position = LiteMath::float3(dis(gen), dis(gen), -100.0f + dis(gen));
    direction = normalize(cameraPos - position);
    boom.new_boom();
}

Bullet::Bullet(LiteMath::float3 enemypos) : actual(true) {
    position = cameraPos + LiteMath::float3(0.0, -1.0, -1.0);
    direction = normalize(enemypos - position) * 10;
}

void Bullet::move() {
    position += direction * 10 * deltaTime;
    if (position.z < -100){
        actual = false;
    }
}
