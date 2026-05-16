#pragma once
#include "bagel.h"
#include <string>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>

/// @file GoldenAxe.h
/// @brief Core definitions for the Golden Axe game demo using the Bagel engine and SDL3.

#define CHARACTERS_FILE "external/characters.png"
#define ENEMIES_FILE "external/enemies.png"
#define SANTA_FILE "external/santa.png"
#define FIRE_FILE "external/fire.png"
#define STAGE_FILE "external/longstage.jpg"
#define FONTS_FILE "external/fonts.png"
#define FLASKBAR_FILE "external/flaskBar.png"

using namespace bagel;

namespace goldenaxe {

    // --- Components Definitions ---

    /// @struct Position
    /// @brief Represents the 2D coordinates of an entity in the world
    struct Position {
        float x, y;
    };

    /// @struct Movement
    /// @brief Represents the velocity components of an entity.
    struct Movement {
        float vx, vy;
    };

    /// @struct Collider
    /// @brief Wraps a Box2D body ID for physics and collision handling.
    struct Collider {
        b2BodyId body;
    };

    /// @struct Keys
    /// @brief Maps SDL scan codes to specific game actions for a player.
    struct Keys {
        SDL_Scancode up, down, right, left, hit, magic;
    };

    /// @struct Intent
    /// @brief Boolean flags representing the current desired actions of an entity (AI or Player).
    struct Intent {
        bool up, down, right, left, hit, magic;
    };

    /// @struct Drawable
    /// @brief Contains rendering data including texture part and display size.
    struct Drawable {
        SDL_FRect part;
        SDL_FPoint size;
        SDL_Texture* texture;
    };

    /// @struct Animation
    /// @brief Handles sprite animation states, frame timing, and specialized combat frames.
    struct Animation {
        int numFrames = 1;  ///Number of frames in the run cycle
        int currentFrame = 0;  ///Current active frame index
        float frameTime = 0.15f;   ///Duration of each frame
        float elapsed = 0.0f;  ///Timer for frame switching

        float idleX, idleY, idleW, idleH; ///Source rect for idle state
        float runX, runY, runW, runH;   ///Source rect for running state

        float attackX, attackY, attackW, attackH; ///Source rect for attack state
        int attackFrames; ///Total frames in attack animation

        float hitX, hitY, hitW, hitH; ///Source rect for being hit
        int hitFrames;   ///Total frames in hit animation

        float frameWidth;  ///Width to offset for multi-frame animations
        float flipOffsetX;  ///Offset applied when the sprite is flipped
        bool defaultLookLeft = false;  ///True if the sprite faces left by default in the sheet
        float hitTimer = 0.0f;  ///Duration remaining for hit freeze/animation
        float lieDeadTimer = 0.0f;  ///Duration the entity remains on ground before removal
    };

    /// @struct ChangeLives
    /// @brief Manages health, lives, and invulnerability status.
    struct ChangeLives {
        int lives = 3;
        int credits = 1;
        float invulnTimer = 0.0f;  ///Temporary protection timer after taking damage
    };

    /// @struct Score
    /// @brief Tracks points earned by the player.
    struct Score {
        int points = 0;
    };

    /// @enum AIType
    /// @brief Defines behaviors: CHASER follows player, RUNNER flees/keeps distance.
    enum class AIType { CHASER, RUNNER };

    /// @enum AIState
    /// @brief States for the AI finite state machine.
    enum class AIState { APPROACH, ATTACK, WAIT, COOLDOWN };

    /// @struct AI
    /// @brief Component controlling NPC behavior logic.
    struct AI {
        bool active = true;
        float speed = 1.2f;
        AIType type = AIType::CHASER;
        AIState state = AIState::APPROACH;
        float timer = 0.0f;
        static bool is_anyone_attacking;   ///Synchronizes enemy attacks
        static float global_attack_cooldown;   ///Global timer to prevent enemy spamming
        bool is_player_in_range = false;    ///Set via Box2D sensors
        bool lastFacingRight = false;   ///Used for directional logic
    };

    /// @struct Hit
    /// @brief Manages attack and damage.
    struct Hit {
        bool is_attacking = false;
        int damage = 1;
    };

    /// @struct FlaskUsage
    /// @brief Manages magic resource collection and activation.
    struct FlaskUsage {
        int current_flasks = 0;
        int goal = 4;
        bool magic_active = false;
        float magic_timer = 0.0f;
    };

    // Events (Tags)
    struct EnemyKilledEvent {};
    struct FlaskCollectedEvent {};
    struct SantaTag {};
    struct FlaskTag {};
    struct FireMagicTag {};

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
        {0, 0,150, 224}, // Stage 1
        { 104, 0, 150, 224 }, // Stage 2
        { 208, 0, 150, 224 }, // Stage 3
        { 312, 0, 150, 224 }  // Stage 4
    };

    inline constexpr StageBounds STAGE_BOUNDS[] = {
        { // Stage 1
            20, 70,
            5, 240,
            0, 250
        },

        { // Stage 2
          40,90,   // topY, bottomY
          0,200,    // leftTop, rightTop
          0,430     // leftBottom, rightBottom
        },

        { // Stage 3
            60, 95,
            20, 400,
            0, 400
        },

        { // Stage 4
            40, 95,
            20, 400,
            0, 406
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
            {20,120},
            {25,200},
            {400,120},
            {400,200}
        },

        // Stage 2
        {
            {140,300},
            {140,250},
            {400,300},
            {500,360}
        },

        // Stage 3
        {
            {0,350},
            {10,80},
            {600,400},
            {650,370}
        },

        // Stage 4
        {
            {0,350},
            {340,75},
            {600,400},
            {700,450}
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

    /// @class GoldenAxe
    /// @brief Main game controller class handling ECS systems and the game loop.
    class GoldenAxe {
    private:

        //source Picture's definitions
        static constexpr int WIN_W = 416;
        static constexpr int WIN_H = 121;

        //dest Picture's definitions
        static constexpr int SCREEN_W = 800;
        static constexpr int SCREEN_H = 600;

        static float constexpr speed = 2.0f;
        static STAGE_INDEX currStage; //initialized in ctor

        static constexpr int FPS = 60;
        static constexpr Uint64 GAME_FRAME = 1000 / FPS;

        static constexpr float TEX_SCALE = 1.8f;
        static constexpr float BOX_SCALE = 10.0f;

        SDL_FRect stageFrame;

        //for transitioning
        bool transitioning = false;
        bool forwardtransition = false;

        bool battleFinished = false;

        float transitionTargetX = 0.f;

        float cameraSpeed = 1.5f;

        Entity transitionHero = Entity::first();

        SDL_Window* win = nullptr;
        SDL_Renderer* ren = nullptr;
        SDL_Texture* characterstex = nullptr;
        SDL_Texture* enemiestex = nullptr;
        SDL_Texture* flasktex = nullptr;
        SDL_Texture* santatex = nullptr;
        SDL_Texture* firetex = nullptr;
        SDL_Texture* stagetex = nullptr;
        SDL_Texture* fontstex = nullptr;
        SDL_Texture* flaskbartex = nullptr;

        b2WorldId world = b2_nullWorldId;
        int totalKills = 0;
        bool santaSpawned = false;
        static constexpr int KILLS_REQUIRED = 6;
        bool waveInProgress = true;
        float spawnTimer = 0.0f;

        //void box_system() const;
        void input_system() const;  /// @brief Updates the input state for all entities with Keys.
        void ai_system() const;   /// @brief Executes AI logic for enemies and NPCs.
        void move_system() const;   /// @brief Handles spatial movement and stage boundary clamping.
        //void score_system() const;
        void draw_system() const;   /// @brief Renders all drawable entities and HUD elements.
        void animation_system(float deltaTime) const;  /// @brief Updates animation frames based on entity state (hit, move, idle).
        void combat_system(float deltaTime) ;   /// @brief Processes hit detection between attackers and victims.
        void magic_system(float dt) const;  /// @brief Manages magic effects, screen shakes, and global damage.
        void resetStage(bool spawnHero);  /// @brief Resets stage variables and spawns initial entities.
        void startStageTransition();   /// @brief Initiates visual and logical transition between stages.
        void transition_system();
        bool battleOverStagePassed();   /// @brief Checks if all enemies are defeated to pass the stage.
        bool battleOverStageFailed();
        void gameplay_system(float dt, bool spawnHero);  /// @brief Core logic for entity spawning and win/loss conditions.
        void static drawFlaskBar(SDL_Renderer* renderer, SDL_Texture* barTex, int current, int max);


        static constexpr Drawable makeDrawable(SDL_FRect part, SDL_Texture* texture);
        static constexpr SDL_FRect colliderRect(const Position& p, const Drawable& d);
        static constexpr float upperStartingPosition = 300.0f;
        static constexpr float bottomStartingPosition = 500.0f;

    public:
        GoldenAxe();
        ~GoldenAxe();
        static STAGE_INDEX getCurrStage() { return currStage; }
        void run();  /// @brief Starts the main game loop.
    };

    // --- Factory Functions ---

    static ent_type CreateHero(b2WorldId world, float x, float y, SDL_Texture* texture);  /// @brief Factory function to create a player character.
    static ent_type CreateEnemy(b2WorldId world, float x, float y, float w, float h, int frames, SDL_Texture* texture);  /// @brief Factory function to create an enemy.
    static ent_type CreateSanta(b2WorldId world, float x, float y, SDL_Texture* texture);
    static ent_type CreateFlask(b2WorldId world, float x, float y, SDL_Texture* texture);

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

template <> struct bagel::Storage<goldenaxe::SantaTag> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::SantaTag>;
};

template <> struct bagel::Storage<goldenaxe::FlaskTag> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::FlaskTag>;
};

template <> struct bagel::Storage<goldenaxe::FlaskUsage> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::FlaskUsage>;
};

template <> struct bagel::Storage<goldenaxe::FireMagicTag> final : bagel::NoInstance {
    using type = bagel::PackedStorage<goldenaxe::FireMagicTag>;
};