#pragma once
#include "bagel.h"
#include <string>
#include <iostream>
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
    using Keys = struct { SDL_Scancode up,down,right,left,hit; };
    using Intent = struct { bool up,down,right,left,hit; };
    using Collider = struct {SDL_FRect part;};
    using Movement =struct{ float vx, vy; };
    using ChangeLives = struct {
    using Drawable = struct {
        SDL_FRect part;
        SDL_FRect dest;
        float a;
    };

    struct Animation {
        int numFrames = 1;
        int currentFrame = 0;
        float frameTime = 0.1f;
        float elapsed = 0.0f;
    };

    struct Position { float x, y; };
    struct Movement { float vx, vy; };

    struct ChangeLives {
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


    struct Collider {
        b2BodyId body;
    };

    // For Score System
    using EnemyKilledEvent = struct {};
    using FlaskCollectedEvent = struct {};

    // Entities Creation
    static ent_type CreateHero(b2WorldId world, float x, float y) {
        ent_type hero = World::createEntity();

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {x / 10.0f, y / 10.0f};
        bodyDef.userData = (void*)(uintptr_t)hero.id;
        b2BodyId body = b2CreateBody(world, &bodyDef);

        b2Polygon box = b2MakeBox(3.2f, 3.2f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shapeDef, &box);

        World::addComponent(hero, Position{x, y});
        World::addComponent(hero, Movement{0, 0});
        World::addComponent(hero, Collider{body});
        World::addComponent(hero, ChangeLives{3, 1});
        World::addComponent(hero, Hit{false, 1});
        World::addComponent(hero, FlaskUsage{0, 5});
        World::addComponent(hero, Score{0});
        World::addComponent(hero, Drawable{{0,0,64,64}, {x,y,64,64}, 0});
        return hero;
    }

    static ent_type CreateEnemy(b2WorldId world, float x, float y, float w, float h, int frames) {
        ent_type enemy = World::createEntity();

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {x / 10.0f, y / 10.0f};
        bodyDef.userData = (void*)(uintptr_t)enemy.id;
        b2BodyId body = b2CreateBody(world, &bodyDef);

        b2Polygon box = b2MakeBox((w*1.5f)/20.0f, (h*1.5f)/20.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shapeDef, &box);

        World::addComponent(enemy, Position{x, y});
        World::addComponent(enemy, Movement{0, 0});
        World::addComponent(enemy, Collider{body});
        World::addComponent(enemy, ChangeLives{1, 0});
        World::addComponent(enemy, AI{true, 1.0f});
        World::addComponent(enemy, Drawable{{0, 0, w, h}, {x, y, w * 1.5f, h * 1.5f}, 0.0f});
        World::addComponent(enemy, Animation{frames, 0, 0.15f, 0.0f});

        return enemy;
    }

    static ent_type CreateSanta(float x, float y) {
        ent_type santa = World::createEntity();
        World::addComponent(santa, Position{x, y});
        World::addComponent(santa, Movement{0, 0});
        World::addComponent(santa, AI{true, 2.0f, AIType::RUNNER}); //santa is faster and running from player
        World::addComponent(santa, Drawable{{0,0,32,32}, {x,y,32,32}, 0});
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

    static constexpr int	WIN_W = 800;
    static constexpr int	WIN_H = 600;

    inline bool overlap(SDL_FRect a, SDL_FRect b) {
        bool widthoverlap=false;
        bool heightoverlap=false;
        if ((b.x<=a.x && a.x<=b.x+b.w)||((a.x<=b.x && b.x<=a.x+a.w)))
         widthoverlap=true;
        if ((b.y<=a.y && a.y<=b.y+b.h)||((a.y<=b.y && b.y<=a.y+a.h)))
            heightoverlap=true;

        return widthoverlap && heightoverlap;
    }

    inline bool outofbounds(SDL_FRect rect) {
        return rect.x<=0 || rect.x+rect.w >= WIN_W || rect.y<=0 || rect.y+rect.h >= WIN_H;
    }

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
        static constexpr  SDL_FRect colliderRect(const Position& p,const Drawable& d) ;

     public:
        GoldenAxe();
        ~GoldenAxe(){};
        /*bool valid() const {
            return b2World_IsValid(this->box);
        }*/
        void run();
    };



}
    class DrawingSystem {
    public:
        static void updateAnimation(float deltaTime) {
            static const Mask mask = MaskBuilder()
                .set<Animation>()
                .set<Drawable>()
                .build();

            for (Entity e = Entity::first(); !e.eof(); e.next()) {
                if (e.test(mask)) {
                    auto& anim = e.get<Animation>();
                    auto& draw = e.get<Drawable>();

                    anim.elapsed += deltaTime;
                    if (anim.elapsed >= anim.frameTime) {
                        anim.elapsed = 0.0f;
                        anim.currentFrame = (anim.currentFrame + 1) % anim.numFrames;
                        draw.part.x = anim.currentFrame * draw.part.w;
                    }
                }
            }
        }

        static void draw(SDL_Renderer* ren, SDL_Texture* tex) {
            static const Mask mask = MaskBuilder().set<Drawable>().set<Position>().build();

            for (Entity e = Entity::first(); !e.eof(); e.next()) {
                if (e.test(mask)) {
                    auto& d = e.get<Drawable>();
                    auto& pos = e.get<Position>();

                    SDL_FRect dest = { pos.x, pos.y, d.dest.w, d.dest.h };
                    SDL_RenderTextureRotated(ren, tex, &d.part, &dest, d.a, nullptr, SDL_FLIP_NONE);
                }
            }
        }
    };

    class AISystem {
    public:
        static void update(Entity player) {
            auto& playerPos = player.get<Position>();

            static const Mask aiMask = MaskBuilder()
                .set<AI>()
                .set<Position>()
                .set<Movement>()
                .build();

            for (Entity e = Entity::first(); !e.eof(); e.next()) {
                if (e.test(aiMask)) {
                    auto& ai = e.get<AI>();
                    auto& pos = e.get<Position>();
                    auto& mov = e.get<Movement>();

                    float direction = (pos.x < playerPos.x) ? 1.0f : -1.0f;

                    if (ai.type == AIType::CHASER) {
                        mov.vx = direction * ai.speed;
                    } else if (ai.type == AIType::RUNNER) {
                        mov.vx = -direction * ai.speed;
                    }
                }
            }
        }
    };
}

template <> struct bagel::Storage<GoldenAxe::AI> final : bagel::NoInstance {
    using type = bagel::PackedStorage<GoldenAxe::AI>;
};

template <> struct bagel::Storage<GoldenAxe::Animation> final : bagel::NoInstance {
    using type = bagel::PackedStorage<GoldenAxe::Animation>;
};

template <> struct bagel::Storage<GoldenAxe::Collider> final : bagel::NoInstance {
    using type = bagel::PackedStorage<GoldenAxe::Collider>;
};
