#pragma once

#include "bagel.h"
#include <SDL3/SDL_stdinc.h>
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

    // Structs Definitions
    using Position =struct { float x, y; };
    using Keys = struct { SDL_Scancode up,down,right,left; };
    using Intent = struct { bool up,down,right,left; };
    using Movement =struct{ float vx, vy; };
    using ChangeLives = struct {
        int lives = 3;
        int credits = 1;
    };
    using Score = struct {
        int points = 0;
    };
    using Drawable = struct { SDL_FRect part; SDL_FPoint size; SDL_Texture *texture; };
    enum class AIType { CHASER, RUNNER };
    using AI =struct{
        bool active = true;
        float speed = 1.2f;
        AIType type = AIType::CHASER; // default chaser
    };
    using Hit =struct {
        bool is_attacking = false;
        int damage = 1;
    };

    //Can be useful to differ between warrior and enemy
    using FlaskUsage = struct {
        int current_flasks = 0;
        int goal = 5;
    };
    using Collider = struct { b2BodyId b; };
    using Animation = struct {
        int frame = 0;
        float timer = 0.f;
    };

    using State = struct {
        enum Type {
            IDLE,
            WALK,
            ATTACK,
            HIT
        } type = IDLE;
    };


    // For Score System
    using EnemyKilledEvent = struct {};
    using FlaskCollectedEvent = struct {};

    // Entities Creation
    static ent_type CreateHero(float x, float y) {
        ent_type hero = World::createEntity();
        World::addComponent(hero, Position{x, y});
        World::addComponent(hero, Movement{0, 0});
        World::addComponent(hero, ChangeLives{3, 1});
        World::addComponent(hero, Hit{false, 1});
        World::addComponent(hero, FlaskUsage{0, 5});
        World::addComponent(hero, Score{0});
        return hero;
    }

    static ent_type CreateEnemy(float x, float y) {
        ent_type enemy = World::createEntity();
        World::addComponent(enemy, Position{x, y});
        World::addComponent(enemy, Movement{0, 0});
        World::addComponent(enemy, ChangeLives{1, 0});
        World::addComponent(enemy, AI{true, 1.0f});
        return enemy;
    }

    static ent_type CreateSanta(float x, float y) {
        ent_type santa = World::createEntity();
        World::addComponent(santa, Position{x, y});
        World::addComponent(santa, Movement{0, 0});
        World::addComponent(santa, AI{true, 2.0f, AIType::RUNNER}); //santa is faster and running from player
        return santa;
    }

   /* // Systems Implementations
    class GameplaySystems {
    public:
        static void UpdateAI(ent_type player_entity) {
            auto& player_pos = World::getComponent<Position>(player_entity);

            for (int i = 0; i <= World::maxId(); ++i) {
                ent_type e{i};
                if (World::mask(e).test(Component<AI>::Bit) &&
                    World::mask(e).test(Component<Movement>::Bit)) {

                    auto& enemy_pos = World::getComponent<Position>(e);
                    auto& enemy_mov = World::getComponent<Movement>(e);
                    auto& enemy_ai = World::getComponent<AI>(e);

                    if (enemy_pos.x < player_pos.x) {
                        enemy_mov.vx = (enemy_ai.type == AIType::CHASER) ? enemy_ai.speed : -enemy_ai.speed;
                    } else {
                        enemy_mov.vx = (enemy_ai.type == AIType::CHASER) ? -enemy_ai.speed : enemy_ai.speed;
                    }
                }
            }
        }

        static void useMagic(ent_type player) {
            auto& flasks = World::getComponent<FlaskUsage>(player);

            if (flasks.current_flasks >= flasks.goal) {
                std::cout << "!!! CASTING ULTIMATE MAGIC !!!" << std::endl;

                for (int i = 0; i <= World::maxId(); ++i) {
                    ent_type e{i};
                    if (World::mask(e).test(Component<AI>::Bit)) {
                        auto& lives = World::getComponent<ChangeLives>(e);
                        if (lives.lives > 0) {
                            lives.lives = 0;
                            World::addComponent(e, EnemyKilledEvent{});
                        }
                    }
                }

                flasks.current_flasks = 0;
            } else {
                std::cout << "Not enough flasks for magic! Current: "
                          << flasks.current_flasks << "/" << flasks.goal << std::endl;
            }
        }

        static void collectFlask(ent_type player) {
            auto& flasks = World::getComponent<FlaskUsage>(player);

            if (flasks.current_flasks < flasks.goal) {
                flasks.current_flasks++;
                World::addComponent(player, FlaskCollectedEvent{});

                std::cout << "Flask Collected! Current Magic: "
                          << flasks.current_flasks << "/" << flasks.goal << std::endl;
            }
        }
    };

    class CombatSystem {
    public:
        static void checkAttack(ent_type attacker, ent_type victim) {
            auto& hit = World::getComponent<Hit>(attacker);
            auto& victim_stats = World::getComponent<ChangeLives>(victim);
            auto& attacker_pos = World::getComponent<Position>(attacker);
            auto& victim_pos = World::getComponent<Position>(victim);

            if (hit.is_attacking) {
                float dist = std::abs(attacker_pos.x - victim_pos.x);
                if (dist < 50.0f) {
                    victim_stats.lives -= hit.damage;
                    std::cout << "Hit! Victim lives left: " << victim_stats.lives << std::endl;

                    if (victim_stats.lives <= 0 && !World::mask(victim).test(Component<EnemyKilledEvent>::Bit)) {                        World::addComponent(victim, EnemyKilledEvent{});
                        World::addComponent(victim, EnemyKilledEvent{});
                    }

                    hit.is_attacking = false;
                }
            }
        }
    };

    class ScoreSystem {
    public:
        static void update(ent_type player) {
            auto& score = World::getComponent<Score>(player);

            for (int i = 0; i <= World::maxId(); ++i) {
                ent_type e{i};
                auto mask = World::mask(e);

                if (mask.test(Component<EnemyKilledEvent>::Bit)) {
                    score.points += 100;
                    //World::removeComponent<EnemyKilledEvent>(e);
                }

                if (mask.test(Component<FlaskCollectedEvent>::Bit)) {
                    score.points += 50;
                    //World::removeComponent<FlaskCollectedEvent>(e);
                }
            }
        }
    };*/

    //Settings for positions (from images)
    inline constexpr SDL_FRect HERO_IDLE = {
        185, 20,
        45, 70
    };

    inline constexpr SDL_FRect HERO_WALK[] = {
        { 10, 90, 45, 70 },
        { 60, 90, 45, 70 },
        {110, 90, 45, 70 },
        {160, 90, 45, 70 }
    };

    inline constexpr SDL_FRect HERO_ATTACK = {
        180, 170,
        70, 70
    };

    inline constexpr SDL_FRect HERO_HIT = {
        420, 120,
        60, 60
    };

    inline constexpr SDL_FRect ENEMY_IDLE = {
        10, 10,
        55, 70
    };

    inline constexpr SDL_FRect ENEMY_WALK[] = {
        { 10, 10, 55, 70 },
        { 65, 10, 55, 70 },
        {120, 10, 55, 70 },
        {175, 10, 55, 70 }
    };

    inline constexpr SDL_FRect ENEMY_ATTACK = {
        180, 90,
        80, 70
    };

    inline constexpr SDL_FRect SANTA = {
        95, 70,
        120, 140
    };

    inline constexpr SDL_FRect FLASK = {
        0, 0,
        70, 70
    };

    class GoldenAxe {
     private:

        static constexpr int	WIN_W = 800;
        static constexpr int	WIN_H = 600;

        //Placements for the characters
        static float constexpr upperStartingPosition = 120;
        static float constexpr bottomStartingPosition = WIN_H - 120;
        static float constexpr leftStartingPosition = 150;
        static float constexpr rightStartingPosition = WIN_W - 150;
        static float constexpr speed=1;

        static constexpr int	FPS = 60;
        static constexpr Uint64	GAME_FRAME = 1000/FPS;
        static constexpr float	RAD_TO_DEG = 57.2958f;


        static constexpr float TEX_SCALE = 1.f;
        static constexpr float BOX_SCALE = 10.f;
        //static constexpr float FLOOR_Y = 420.f;


        SDL_Texture*		characterstex = nullptr;
        SDL_Texture*		enemiestex = nullptr;
        SDL_Texture*		flasktex = nullptr;
        SDL_Texture*		santatex = nullptr;
        SDL_Texture*		stagetex = nullptr;
        SDL_Renderer*		ren = nullptr;
        SDL_Window*			win = nullptr;
        //b2WorldId box = b2_nullWorldId;

        void box_system() const;
        void input_system() const;
        void move_system() const;
        void score_system() const;
        void draw_system() const;
        void hit_system() const;
        void resetStage() const;

        static constexpr Drawable makeDrawable(SDL_FRect part, SDL_Texture* texture);

     public:
        GoldenAxe();
        ~GoldenAxe(){};
        /*bool valid() const {
            return b2World_IsValid(this->box);
        }*/
        void run();
    };



}