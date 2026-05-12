#include "GoldenAxe.h"
#include <iostream>
#include <ctime>
#include "bagel.h"

using namespace std;
using namespace bagel;

namespace goldenaxe {

    GoldenAxe::GoldenAxe() {
        srand((unsigned int)time(NULL));

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            cout << SDL_GetError() << endl;
            return;
        }

        if (!SDL_CreateWindowAndRenderer("Golden Axe", WIN_W, WIN_H, 0, &win, &ren)) {
            cout << SDL_GetError() << endl;
            return;
        }

        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = { 0.0f, 0.0f };
        world = b2CreateWorld(&worldDef);

        if (b2World_IsValid(world) == false) {
            cout << "Failed to create Box2D world" << endl;
            return;
        }

        characterstex = IMG_LoadTexture(ren, CHARACTERS_FILE);
        enemiestex = IMG_LoadTexture(ren, ENEMIES_FILE);
        flasktex = IMG_LoadTexture(ren, FLASK_FILE);
        santatex = IMG_LoadTexture(ren, SANTA_FILE);
        stagetex = IMG_LoadTexture(ren, STAGE_FILE);

        if (!stagetex) cout << "Warning: Could not load textures. Check external folder!" << endl;
    }

    GoldenAxe::~GoldenAxe() {
        if (b2World_IsValid(world)) b2DestroyWorld(world);
        if (characterstex) SDL_DestroyTexture(characterstex);
        if (enemiestex) SDL_DestroyTexture(enemiestex);
        if (flasktex) SDL_DestroyTexture(flasktex);
        if (santatex) SDL_DestroyTexture(santatex);
        if (stagetex) SDL_DestroyTexture(stagetex);
        if (ren) SDL_DestroyRenderer(ren);
        if (win) SDL_DestroyWindow(win);
        SDL_Quit();
    }

    ent_type CreateHero(b2WorldId world, float x, float y, SDL_Texture* texture) {
        ent_type hero = World::createEntity();

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = { x / 10.0f, y / 10.0f };
        bodyDef.userData = (void*)(uintptr_t)hero.id;
        b2BodyId body = b2CreateBody(world, &bodyDef);

        b2Polygon box = b2MakeBox(2.0f, 3.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shapeDef, &box);

        World::addComponent(hero, Position{x, y});
        World::addComponent(hero, Movement{0, 0});
        World::addComponent(hero, Collider{body});
        World::addComponent(hero, Drawable{HERO_IDLE, {HERO_IDLE.w, HERO_IDLE.h}, texture});
        World::addComponent(hero, Animation{4, 0, 0.12f, 0.0f,
            0, 82, 32, 70,
            -5, 145,32, 62,
            42, 20.0f, false});

        return hero;
    }

    ent_type CreateEnemy(b2WorldId world, float x, float y, float w, float h, int frames, SDL_Texture* texture) {
        ent_type enemy = World::createEntity();

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = { x / 10.0f, y / 10.0f };
        bodyDef.userData = (void*)(uintptr_t)enemy.id;
        b2BodyId body = b2CreateBody(world, &bodyDef);

        b2Polygon box = b2MakeBox(w / 20.0f, h / 20.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shapeDef, &box);

        World::addComponent(enemy, Position{x, y});
        World::addComponent(enemy, Movement{0, 0});
        World::addComponent(enemy, Collider{body});
        World::addComponent(enemy, Drawable{ENEMY_IDLE, {w, h}, texture});
        World::addComponent(enemy, Animation{4,0,0.15f,0.0f,
            0,10, 60, 75,
            370,20,40, 50,
            50,-15.0f, true});
        World::addComponent(enemy, Intent{false, false, false, false, false});
        World::addComponent(enemy, AI{true, 0.8f, AIType::CHASER});

        return enemy;
    }

    constexpr Drawable GoldenAxe::makeDrawable(SDL_FRect part, SDL_Texture* texture) {
        return Drawable{{part}, {part.w*TEX_SCALE, part.h*TEX_SCALE},texture};
    }

    constexpr SDL_FRect GoldenAxe::colliderRect(const Position& p,const Drawable& d) {
        return {
            p.x,
            p.y,
            d.size.x,
            d.size.y
        };
    }

    void GoldenAxe::draw_system() const {

        SDL_SetRenderDrawColor(ren,0,0,0,255);
        SDL_RenderClear(ren);
        SDL_RenderTexture(ren,stagetex,nullptr,nullptr);

        Mask mdraw = MaskBuilder().set<Drawable>().set<Position>().set<Movement>().build();
        for (auto e = Entity::first();!e.eof(); e.next()) {
            if (e.test(mdraw)) {
                const Drawable& d = e.get<Drawable>();
                const Position& p = e.get<Position>();
                const auto& mov = e.get<Movement>();

                float scaledW = d.size.x * TEX_SCALE;
                float scaledH = d.size.y * TEX_SCALE;

                float offsetY = scaledH - d.size.y;
                SDL_FRect dest = { p.x, p.y - offsetY, scaledW, scaledH };

                SDL_FlipMode flip = SDL_FLIP_NONE;
                bool lookLeftInSheet = false;
                float fOffset = 0;

                if (e.test(MaskBuilder().set<Animation>().build())) {
                    const auto& anim = e.get<Animation>();
                    lookLeftInSheet = anim.defaultLookLeft;
                    fOffset = anim.flipOffsetX;
                }
                if ((!lookLeftInSheet && mov.vx < -0.1f) || (lookLeftInSheet && mov.vx > 0.1f)) {
                    flip = SDL_FLIP_HORIZONTAL;
                    dest.x += fOffset * TEX_SCALE;
                }

                SDL_RenderTextureRotated(ren, d.texture, &d.part, &dest, 0, nullptr, flip);
            }
        }
        SDL_RenderPresent(ren);
    }

    void GoldenAxe::animation_system(float deltaTime) const {
        static const Mask animMask = MaskBuilder()
            .set<Animation>().set<Drawable>().set<Movement>().build();

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(animMask)) {
                auto& anim = e.get<Animation>();
                auto& draw = e.get<Drawable>();
                const auto& mov = e.get<Movement>();

                if (mov.vx != 0 || mov.vy != 0) {
                    anim.elapsed += deltaTime;
                    if (anim.elapsed >= anim.frameTime) {
                        anim.elapsed = 0.0f;
                        anim.currentFrame = (anim.currentFrame + 1) % anim.numFrames;

                        draw.part.x = anim.runX + (anim.currentFrame * anim.frameWidth);
                        draw.part.y = anim.runY;
                        draw.part.w = anim.runW;
                        draw.part.h = anim.runH;

                        draw.size.x = anim.runW;
                        draw.size.y = anim.runH;
                    }
                } else {
                    draw.part.x = anim.idleX;
                    draw.part.y = anim.idleY;
                    draw.part.w = anim.idleW;
                    draw.part.h = anim.idleH;

                    draw.size.x = anim.idleW;
                    draw.size.y = anim.idleH;
                    anim.currentFrame = 0;
                }
            }
        }
    }

    void GoldenAxe::input_system() const {
        {
            static const Mask mask = MaskBuilder()
                .set<Keys>()
                .set<Intent>()
                .build();

            SDL_PumpEvents();
            const bool* keys = SDL_GetKeyboardState(nullptr);

            for (Entity e = Entity::first(); !e.eof(); e.next()) {
                if (e.test(mask)) {
                    const auto& k = e.get<Keys>();
                    auto& i = e.get<Intent>();

                    i.up = keys[k.up];
                    i.down = keys[k.down];
                    i.left = keys[k.left];
                    i.right = keys[k.right];
                    i.hit = keys[k.hit];
                }
            }
        }

    }

    void GoldenAxe::move_system() const {
        static const Mask movemask = MaskBuilder()
        .set<Intent>()
        .set<Position>()
        .set<Movement>()
        .set<Collider>()
        .build();

        static const Mask collidemask = MaskBuilder()
        .set<Collider>()
        .build();

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(movemask))
            {
                bool canmove = true;

                auto& pos = e.get<Position>();
                auto& mov = e.get<Movement>();
                auto& intent = e.get<Intent>();
                auto& col = e.get<Collider>();

                mov.vx = 0;
                mov.vy = 0;

                if (intent.left)
                    mov.vx = -speed;

                if (intent.right)
                    mov.vx = speed;

                if (intent.up)
                    mov.vy = -speed;

                if (intent.down)
                    mov.vy = speed;

                b2Vec2 currentPos = b2Body_GetPosition(col.body);
                const auto& d = e.get<Drawable>();

                SDL_FRect next = {
                    currentPos.x * BOX_SCALE,
                    currentPos.y * BOX_SCALE,
                    d.size.x,
                    d.size.y
                };

                next.x += mov.vx;
                next.y += mov.vy;
                if (goldenaxe::outofbounds(next))
                    canmove = false;

                for (Entity e1 = Entity::first(); !e1.eof() && canmove; e1.next()) {
                    if (e1.entity().id == e.entity().id) continue;

                    if (e1.test(collidemask)) {
                        const auto& c1 = e1.get<Collider>();
                        const auto& d1 = e1.get<Drawable>();

                        b2Vec2 p1 = b2Body_GetPosition(c1.body);
                        SDL_FRect rect1 = {
                            p1.x * BOX_SCALE,
                            p1.y * BOX_SCALE,
                            d1.size.x,
                            d1.size.y
                        };

                        if (goldenaxe::overlap(next, rect1))
                            canmove = false;
                    }
                }
                if (canmove) {

                    b2Vec2 newPos = { next.x / BOX_SCALE, next.y / BOX_SCALE };
                    b2Rot currentRotation = b2Body_GetRotation(col.body);
                    b2Body_SetTransform(col.body, newPos, currentRotation);

                    pos.x = next.x;
                    pos.y = next.y;
                }
            }
        }
    }

    bool AI::is_anyone_attacking = false;
    float AI::global_attack_cooldown = 0.0f;
    bool is_player_active = false;

    void GoldenAxe::ai_system() const {
        Position heroPos = {0, 0};
        bool heroFound = false;
        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(MaskBuilder().set<Keys>().set<Position>().set<Movement>().build())) {
                heroPos = e.get<Position>();
                heroFound = true;

                const auto& mov = e.get<Movement>();
                if (abs(mov.vx) > 0.1f || abs(mov.vy) > 0.1f) {
                    is_player_active = true;
                }
                break;
            }
        }
        if (!heroFound || !is_player_active) return;

        float deltaTime = 1.0f / 60.0f;
        if (AI::global_attack_cooldown > 0) {
            AI::global_attack_cooldown -= deltaTime;
        }

        AI::is_anyone_attacking = false;
        Mask aiMask = MaskBuilder().set<AI>().set<Intent>().set<Position>().build();
        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(aiMask) && e.get<AI>().state == AIState::ATTACK) {
                AI::is_anyone_attacking = true;
                break;
            }
        }

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(aiMask)) {
                auto& ai = e.get<AI>();
                auto& intent = e.get<Intent>();
                auto& pos = e.get<Position>();

                intent.left = intent.right = intent.up = intent.down = intent.hit = false;
                float dx = heroPos.x - pos.x;
                float dy = heroPos.y - pos.y;
                float dist = sqrt(dx*dx + dy*dy);

                switch (ai.state) {
                    case AIState::APPROACH:
                        if (abs(dx) < 60 && abs(dy) < 20) {
                            if (!AI::is_anyone_attacking && AI::global_attack_cooldown <= 0) {
                                ai.state = AIState::ATTACK;
                                ai.timer = 0.8f;
                                AI::global_attack_cooldown = 120.0f;
                            } else {
                                ai.state = AIState::WAIT;
                                ai.timer = 2.0f;
                            }
                        } else {
                            if (dx > 5) intent.right = true; else if (dx < -5) intent.left = true;
                            if (dy > 5) intent.down = true; else if (dy < -5) intent.up = true;
                        }
                        break;

                    case AIState::WAIT:
                        ai.timer -= deltaTime;
                        if (abs(dx) < 100) {
                            if (dx > 0) intent.left = true; else intent.right = true;
                        }
                        if (ai.timer <= 0) ai.state = AIState::APPROACH;
                        break;

                    case AIState::ATTACK:
                        intent.hit = true;
                        ai.timer -= deltaTime;
                        if (ai.timer <= 0) {
                            ai.state = AIState::COOLDOWN;
                            ai.timer = 3.0f;
                        }
                        break;

                    case AIState::COOLDOWN:
                        ai.timer -= deltaTime;
                        if (dx > 0) intent.left = true; else intent.right = true;
                        if (ai.timer <= 0) ai.state = AIState::APPROACH;
                        break;
                }
            }
        }
    }

    void GoldenAxe::resetStage() {
        Mask m = MaskBuilder().set<Movement>().build();
        for (auto e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(m)) e.destroy();
        }

        is_player_active = false;
        ent_type hero = CreateHero(world, leftStartingPosition, upperStartingPosition, characterstex);
        World::addComponent(hero, Intent{false, false, false, false, false});
        World::addComponent(hero, Keys{SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_A, SDL_SCANCODE_F});

        CreateEnemy(world, rightStartingPosition, upperStartingPosition, 55, 70, 4, enemiestex);
        CreateEnemy(world, rightStartingPosition, bottomStartingPosition, 55, 70, 4, enemiestex);
    }

    void GoldenAxe::run() {
        resetStage();
        bool quit =false;
        SDL_Event event;
        while (!quit) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) quit = true;
            }

            b2World_Step(world, 1.0f / 60.0f, 4);
            input_system();
            ai_system();
            move_system();
            draw_system();
            animation_system(1.0f / 60.0f);
            SDL_Delay(16);
        }
        SDL_Quit();
    }
}