#include "sprites.h"
#include <ctime>
#include <random>

extern std::mt19937 gen;

extern LiteMath::float3 cameraPos;

extern float health;

float zfar = -100.0f;

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


Enemy::Enemy(Sprite &astro, Sprite &boom, bool is_ship)
        : SpriteAnimator(astro), boom(boom),
          is_alive(true), is_ship(is_ship),
          time_of_shoot(time(0)) {
    std::uniform_real_distribution<float> dis(-15, 15);
    position = LiteMath::float3(dis(gen), dis(gen) + 5, zfar + dis(gen));
    direction = normalize(cameraPos - position);
}

LiteMath::float3x3 Enemy::animate() {
    if (is_alive) {
        position += direction * 10 * deltaTime;
        if (LiteMath::length(position - cameraPos) < 1.0) {
            health -= 25;
            this->respawn();
        } else if (position.z > cameraPos.z){
            this->respawn();
        }
        return animation();
    } else {
        return boom.animation();
    }
}

void Enemy::kill() {
    is_alive = false;
    time_of_death = time(0);
}

void Enemy::respawn() {
    is_alive = true;
    std::uniform_real_distribution<float> dis(-15, 15);
    position = LiteMath::float3(dis(gen), dis(gen) / 2 + 5, zfar + dis(gen));
    direction = normalize(cameraPos - position);
    boom.new_boom();
}

Bullet Enemy::shoot() {
    time_of_shoot = time(0);
    return Bullet(position, true);
}

Bullet::Bullet(LiteMath::float3 enemypos) : actual(true), enemy_strike(false) {
    position = cameraPos + LiteMath::float3(0.0, -1.0, -1.0);
    direction = normalize(enemypos - position) * 10;
}

Bullet::Bullet(LiteMath::float3 enemypos, bool enemy_strike) : actual(true), enemy_strike(enemy_strike) {
    if (enemy_strike){
        position = enemypos + LiteMath::float3(0.0, 0.0, 0.5);
        direction = normalize(cameraPos + LiteMath::float3(0.0, -1.0, 0.0) - position) * 10;
    } else {
        position = cameraPos + LiteMath::float3(0.0, -1.0, -1.0);
        direction = normalize(enemypos - position) * 10;
    }

}

void Bullet::move() {
    position += direction * 10 * deltaTime;
    if (position.z < zfar) {
        actual = false;
    } else if (position.z > cameraPos.z){
        actual = false;
    }
}



Fog::Fog(int tex) : texture(tex), actual(true) {
    std::uniform_real_distribution<> dis(5, 20);
    std::uniform_int_distribution<> diss(0, 1);
    float x = diss(gen) == 0 ? dis(gen) : -dis(gen);
    float y = diss(gen) == 0 ? dis(gen) : -dis(gen);
    float z = -dis(gen) * 7;
    position = LiteMath::float3(x, y, zfar + z);

    speed = dis(gen) - 12.5;
}

void Fog::move() {
    position += LiteMath::float3(0.0, 0.0, 1.0) * 50 * deltaTime;
    if (position.z > -5) {
        actual = false;
    }
}

