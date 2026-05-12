#pragma once
#include "bagel.h"
#include <string>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

#define CHARACTERS_FILE "external/characters.png"
#define ENEMIES_FILE "external/enemies.png"
#define FLASK_FILE "external/flask.jpg"
#define SANTA_FILE "external/santa.png"
#define STAGE_FILE "external/stage.jpg"

using namespace bagel;

namespace goldenaxe {

    // --- Components Definitions ---

    struct Position {
        float x, y;
    };

    struct Movement {
        float vx, vy;
    };

    struct Collider {
        b2BodyId body;
    };

    struct Keys {
        SDL_Scancode up, down, right, left, hit;
    };

    struct Intent {
        bool up, down, right, left, hit;
    };

    struct Drawable {
        SDL_FRect part;
        SDL_FPoint size;
        SDL_Texture* texture;
    };

    struct Animation {
        int numFrames = 1;
        int currentFrame = 0;
        float frameTime = 0.15f;
        float elapsed = 0.0f;

        float idleX, idleY, idleW, idleH;
        float runX, runY, runW, runH;
        float frameWidth;
        float flipOffsetX;
    };

    struct ChangeLives {
        int lives = 3;
        int credits = 1;
    };

    struct Score {
        int points = 0;
    };

    enum class AIType { CHASER, RUNNER };
    struct AI {
        bool active = true;
        float speed = 1.2f;
        AIType type = AIType::CHASER;
    };

    struct Hit {
        bool is_attacking = false;
        int damage = 1;
    };

    struct FlaskUsage {
        int current_flasks = 0;
        int goal = 5;
    };

    // Events (Tags)
    struct EnemyKilledEvent {};
    struct FlaskCollectedEvent {};

    inline bool outofbounds(SDL_FRect rect) {
        return rect.x <= 0 || rect.x + rect.w >= 800 || rect.y <= 0 || rect.y + rect.h >= 600;
    }

    inline bool overlap(SDL_FRect a, SDL_FRect b) {
        return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
    }

    // --- Sprite Settings ---

    inline constexpr SDL_FRect HERO_IDLE = { 0, 55, 25, 55 };
    inline constexpr SDL_FRect HERO_ATTACK = { 180, 170, 70, 70 };
    inline constexpr SDL_FRect ENEMY_IDLE = { 0, 10, 55, 75 };
    inline constexpr SDL_FRect SANTA_RECT = { 95, 70, 120, 140 }; // renamed from SANTA to avoid conflict

    // --- Main Game Class ---

    class GoldenAxe {
    private:
        static constexpr int WIN_W = 800;
        static constexpr int WIN_H = 600;

        static float constexpr upperStartingPosition = 120;
        static float constexpr bottomStartingPosition = WIN_H - 120;
        static float constexpr leftStartingPosition = 150;
        static float constexpr rightStartingPosition = WIN_W - 150;
        static float constexpr speed = 2.0f;

        static constexpr int FPS = 60;
        static constexpr Uint64 GAME_FRAME = 1000 / FPS;

        static constexpr float TEX_SCALE = 1.8f;
        static constexpr float BOX_SCALE = 10.0f;

        SDL_Window* win = nullptr;
        SDL_Renderer* ren = nullptr;
        SDL_Texture* characterstex = nullptr;
        SDL_Texture* enemiestex = nullptr;
        SDL_Texture* flasktex = nullptr;
        SDL_Texture* santatex = nullptr;
        SDL_Texture* stagetex = nullptr;

        b2WorldId world = b2_nullWorldId;

        void box_system() const;
        void input_system() const;
        void move_system() const;
        void score_system() const;
        void draw_system() const;
        void animation_system(float deltaTime) const;
        void resetStage();

        static constexpr Drawable makeDrawable(SDL_FRect part, SDL_Texture* texture);
        static constexpr SDL_FRect colliderRect(const Position& p, const Drawable& d);

    public:
        GoldenAxe();
        ~GoldenAxe();
        void run();
    };

    // --- Factory Functions ---

    static ent_type CreateHero(b2WorldId world, float x, float y, SDL_Texture* texture);
    static ent_type CreateEnemy(b2WorldId world, float x, float y, float w, float h, int frames, SDL_Texture* texture);
    static ent_type CreateSanta(b2WorldId world, float x, float y);

    // --- Specialized Systems ---

    class AISystem {
    public:
        static void update(Entity player);
    };

    class DrawingSystem {
    public:
        static void updateAnimation(float deltaTime);
        static void draw(SDL_Renderer* ren);
    };

}

template <> struct bagel::Storage<goldenaxe::AI> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::AI>;
};

template <> struct bagel::Storage<goldenaxe::Animation> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::Animation>;
};

template <> struct bagel::Storage<goldenaxe::Collider> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::Collider>;
};