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
#define STAGE_FILE "external/longstage.jpg"

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

        float attackX, attackY, attackW, attackH;
        int attackFrames;

        float hitX, hitY, hitW, hitH;
        int hitFrames;

        float frameWidth;
        float flipOffsetX;
        bool defaultLookLeft = false;
        float hitTimer = 0.0f;
        float lieDeadTimer = 0.0f;
    };

    struct ChangeLives {
        int lives = 3;
        int credits = 1;
        float invulnTimer = 0.0f;
    };

    struct Score {
        int points = 0;
    };

    enum class AIType { CHASER, RUNNER };
    enum class AIState { APPROACH, ATTACK, WAIT, COOLDOWN };
    struct AI {
        bool active = true;
        float speed = 1.2f;
        AIType type = AIType::CHASER;
        AIState state = AIState::APPROACH;
        float timer = 0.0f;
        static bool is_anyone_attacking;
        static float global_attack_cooldown;
        bool is_player_in_range = false;
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

    //Stage indicator
    enum class STAGE_INDEX{STAGE1,STAGE2,STAGE3,STAGE4};

    struct StageBounds {

        float topY;
        float bottomY;

        float leftTop;
        float rightTop;

        float leftBottom;
        float rightBottom;
    };

    inline constexpr SDL_FRect STAGE_FRAMES[] = {

        {   0, 0, 104, 121 }, // Stage 1
        { 104, 0, 104, 121 }, // Stage 2
        { 208, 0, 104, 121 }, // Stage 3
        { 312, 0, 104, 121 }  // Stage 4
    };

    inline constexpr StageBounds STAGE_BOUNDS[] = {

        // Stage 1
        {
            20, 70,
            5, 240,
            0, 250
        },

        // Stage 2
        {
            30, 90,
            120, 175,
            104, 208
        },

        // Stage 3
        {
            45, 90,
            220, 295,
            208, 312
        },

        // Stage 4
        {
            35, 85,
            325, 395,
            312, 416
        }
    };

    constexpr float SCALE_X = 800.0f / 416.0f;

    constexpr float SCALE_Y = 600.0f / 121.0f;

    struct SpawnSet {

        SDL_FPoint hero1;
        SDL_FPoint hero2;

        SDL_FPoint enemy1;
        SDL_FPoint enemy2;
    };

    inline constexpr SpawnSet SPAWNS[] = {

        // Stage 1
        {
            {25,120},
            {25,200},
            {400,120},
            {400,200}
        },

        // Stage 2
        {
            {125,45},
            {140,75},
            {170,45},
            {185,75}
        },

        // Stage 3
        {
            {230,50},
            {245,80},
            {280,50},
            {295,80}
        },

        // Stage 4
        {
            {325,45},
            {340,75},
            {385,45},
            {398,75}
        }
    };


    inline const StageBounds& getStageBounds(STAGE_INDEX s) {
        return STAGE_BOUNDS[
            static_cast<int>(s)
        ];
    }

    inline float lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    inline float leftBound(
    STAGE_INDEX s,
    float y)
    {

        auto b =
          STAGE_BOUNDS[
            static_cast<int>(s)
          ];

        float top =
          b.topY * SCALE_Y;

        float bottom =
          b.bottomY * SCALE_Y;

        float t =
          (y - top) /
          (bottom - top);

        return lerp(
            b.leftTop * SCALE_X,
            b.leftBottom * SCALE_X,
            t
        );
    }

    inline float rightBound(STAGE_INDEX s,float y)
    {

        auto b =
          STAGE_BOUNDS[
            static_cast<int>(s)
          ];

        float top =
          b.topY * SCALE_Y;

        float bottom =
          b.bottomY * SCALE_Y;

        float t =
          (y - top) /
          (bottom - top);

        return lerp(
            b.rightTop * SCALE_X,
            b.rightBottom * SCALE_X,
            t
        );
    }


    //2D Logic
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
        static constexpr int WIN_W = 416;
        static constexpr int WIN_H = 121;

        static constexpr int SCREEN_W = 800;
        static constexpr int SCREEN_H = 600;

        // static float constexpr upperStartingPosition = WIN_H - 240;
        // static float constexpr bottomStartingPosition = WIN_H - 120;
        // static float constexpr leftStartingPosition = 150;
        // static float constexpr rightStartingPosition = WIN_W - 150;
        static float constexpr speed = 2.0f;
        static STAGE_INDEX currStage;

        static constexpr int FPS = 60;
        static constexpr Uint64 GAME_FRAME = 1000 / FPS;

        static constexpr float TEX_SCALE = 1.8f;
        static constexpr float BOX_SCALE = 10.0f;

        SDL_FRect stageFrame = {
            0, 0,
            150, 224
        };

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
        void ai_system() const;
        void move_system() const;
        void score_system() const;
        void draw_system() const;
        void animation_system(float deltaTime) const;
        void combat_system(float deltaTime) const;
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

template <> struct bagel::Storage<goldenaxe::ChangeLives> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::ChangeLives>;
};

template <> struct bagel::Storage<goldenaxe::Score> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::Score>;
};