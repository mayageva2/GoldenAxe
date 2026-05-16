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
            return;        }

        if (!SDL_CreateWindowAndRenderer("Golden Axe", SCREEN_W, SCREEN_H, 0, &win, &ren)) {
            cout << SDL_GetError() << endl;
            return;//
        }

        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = { 0.0f, 0.0f };
        world = b2CreateWorld(&worldDef);

        stageFrame=STAGE_FRAMES[static_cast<int>(currStage)];

        if (b2World_IsValid(world) == false) {
            cout << "Failed to create Box2D world" << endl;
            return;
        }

        characterstex = IMG_LoadTexture(ren, CHARACTERS_FILE);
        enemiestex = IMG_LoadTexture(ren, ENEMIES_FILE);
        flasktex = IMG_LoadTexture(ren, FLASK_FILE);
        santatex = IMG_LoadTexture(ren, SANTA_FILE);
        stagetex = IMG_LoadTexture(ren, STAGE_FILE);
        fontstex = IMG_LoadTexture(ren, FONTS_FILE);

        if (!stagetex) cout << "Warning: Could not load textures. Check external folder!" << endl;
    }

    STAGE_INDEX GoldenAxe::currStage =
        STAGE_INDEX::STAGE1;

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

    void GoldenAxe::startStageTransition() {

        if (currStage ==
            STAGE_INDEX::STAGE4)
            return;
        if (transitioning)
            return;

        transitioning = true;

        transitionTargetX =
            STAGE_FRAMES[
                static_cast<int>(currStage) + 1
            ].x;

        // find hero
        Mask heroMask =
            MaskBuilder()
            .set<Keys>()
            .build();

        for (Entity e = Entity::first();
             !e.eof();
             e.next()) {

            if (e.test(heroMask)) {

                transitionHero = e;
                break;
            }
             }
    }

    bool GoldenAxe::battleOver() {
        Mask enemiesalive = MaskBuilder().set<AI>().build();

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(enemiesalive))
                return false;
        }
        return true;
    }

    void GoldenAxe::transition_system() {

        if (!transitioning)
            return;

        // move camera
        stageFrame.x += cameraSpeed;

        // auto-walk hero
        if (!transitionHero.eof()) {

            auto& pos =
                transitionHero.get<Position>();

            auto& mov =
                transitionHero.get<Movement>();

            mov.vx = cameraSpeed;

            pos.x += cameraSpeed;
        }

        // reached next stage
        if (stageFrame.x >=
            transitionTargetX) {

            stageFrame.x =
                transitionTargetX;

            // advance stage
            currStage =
                static_cast<STAGE_INDEX>(
                    static_cast<int>(
                        currStage
                    ) + 1
                );

            // snap hero to spawn
            const auto& s =
                SPAWNS[
                    static_cast<int>(
                        currStage
                    )
                ];

            if (!transitionHero.eof()) {

                auto& pos =
                    transitionHero.get<Position>();

                auto& mov =
                    transitionHero.get<Movement>();

                float left =leftBound(currStage,s.hero1.y);

                float right =rightBound(currStage,s.hero1.y);

                pos.x =
    std::clamp(
        s.hero1.x,
        left + 10.f,
        right - 40.f
    );

                pos.y = s.hero1.y;

                auto& col =
                    transitionHero.get<Collider>();

                b2Body_SetTransform(
                    col.body,
                    {
                        pos.x / BOX_SCALE,
                        pos.y / BOX_SCALE
                    },
                    b2Body_GetRotation(col.body)
                );

                mov.vx = 0;
                mov.vy = 0;
            }

            transitioning = false;
            resetStage(false);
            }
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
        shapeDef.enableSensorEvents = true;
        b2CreatePolygonShape(body, &shapeDef, &box);

        World::addComponent(hero, Position{x, y});
        World::addComponent(hero, Movement{0, 0});
        World::addComponent(hero, Collider{body});
        World::addComponent(hero, Drawable{HERO_IDLE, {HERO_IDLE.w, HERO_IDLE.h}, texture});
        World::addComponent(hero, Animation{4, 0, 0.12f, 0.0f,
            0, 82, 32, 70,
            -5, 145,32, 62,
            0, 345, 43, 65, 6,
            0, 485, 40, 55, 5,
            42, 20.0f, false, 0.0f});
        World::addComponent(hero, ChangeLives{3, 1});
        World::addComponent(hero, Score{0});

        return hero;
    }

    ent_type CreateEnemy(b2WorldId world, float x, float y, float w, float h, int frames, SDL_Texture* texture) {
        ent_type enemy = World::createEntity();

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = { x / 10.0f, y / 10.0f };
        bodyDef.userData = (void*)(uintptr_t)enemy.id;
        b2BodyId body = b2CreateBody(world, &bodyDef);

        // Collider
        b2Polygon box = b2MakeBox(w / 20.0f, h / 20.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shapeDef, &box);

        // Sensor
        b2Polygon sensorBox = b2MakeBox(6.0f, 2.0f);
        b2ShapeDef sensorDef = b2DefaultShapeDef();
        sensorDef.isSensor = true;
        sensorDef.enableSensorEvents = true;
        b2CreatePolygonShape(body, &sensorDef, &sensorBox);

        // Components
        World::addComponent(enemy, Position{x, y});
        World::addComponent(enemy, Movement{0, 0});
        World::addComponent(enemy, Collider{body});
        World::addComponent(enemy, Drawable{ENEMY_IDLE, {w, h}, texture});
        World::addComponent(enemy, Animation{3,0,0.15f,0.0f,
            0,10, 60, 75,
            365,20,40, 50,
            0, 95, 60, 60, 9,
            255, 322, 55, 55, 5,
            50,-15.0f, true, 0.0f});
        World::addComponent(enemy, Intent{false, false, false, false, false});
        World::addComponent(enemy, AI{true, 0.8f, AIType::CHASER});
        World::addComponent(enemy, ChangeLives{1, 0});

        return enemy;
    }

    // Assumptions:
    // - You already loaded the font texture into SDL_Texture* fontTexture
    // - Each character in the sprite sheet is 8x8 pixels
    // - Renderer width is GameConfig::WIDTH * cellSize (or similar)


    const int CHAR_W = 17;
    const int CHAR_H = 17;
    const int SCALE = 2;

    void drawChar(SDL_Renderer* renderer,
                  SDL_Texture* fontTexture,
                  char c,
                  int x,
                  int y)
    {
        int index = c - 32;

        if (index < 0)
            return;

        const int COLS = 16;

        SDL_FRect src;
        src.x = (index % COLS) * CHAR_W;
        src.y = (index / COLS) * CHAR_H;
        src.w = CHAR_W;
        src.h = CHAR_H;

        SDL_FRect dst;
        dst.x = (float)x;
        dst.y = (float)y;
        dst.w = CHAR_W * SCALE;
        dst.h = CHAR_H * SCALE;

        SDL_RenderTexture(renderer, fontTexture, &src, &dst);
    }

    void drawText(SDL_Renderer* renderer,
              SDL_Texture* fontTexture,
              const std::string& text,
              int x,
              int y)
    {
        for (size_t i = 0; i < text.size(); i++)
        {
            drawChar(renderer,
                     fontTexture,
                     text[i],
                     x + (int)i * CHAR_W * SCALE,
                     y);
        }
    }

    // Draw HUD centered at top
    void drawHUD(SDL_Renderer* renderer,
                 SDL_Texture* fontTexture,
                 int score,
                 int lives,
                 int screenWidth)
    {
        std::string text =
            "SCORE: " + std::to_string(score) +
            "   LIVES: " + std::to_string(lives);

        int textWidth = (int)text.size() * CHAR_W;

        int x = (screenWidth - textWidth) / 2;
        int y = 5;

        drawText(renderer, fontTexture, text, x, y);
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
        SDL_FRect dst = {
            0,
            0,
            SCREEN_W,
            SCREEN_H
        };

        SDL_RenderTexture(
            ren,
            stagetex,
            &stageFrame,
            &dst
        );

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
                if (e.has<Score>()) //Player
                {
                    auto& score = e.get<Score>();
                    auto& lives = e.get<ChangeLives>();
                    drawHUD(ren,fontstex,score.points,lives.lives,WIN_W);
                }

                SDL_RenderTextureRotated(ren, d.texture, &d.part, &dest, 0, nullptr, flip);
            }
        }
        SDL_RenderPresent(ren);
    }

    void GoldenAxe::animation_system(float deltaTime) const {
        static const Mask animMask = MaskBuilder()
            .set<Animation>().set<Drawable>().set<Movement>().set<Intent>().build();

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(animMask)) {
                auto& anim = e.get<Animation>();
                auto& draw = e.get<Drawable>();
                const auto& mov = e.get<Movement>();
                const auto& intent = e.get<Intent>();

                anim.elapsed += deltaTime;
                if (anim.hitTimer > 0) {
                    float totalHitDuration = 0.6f;
                    float timeElapsed = totalHitDuration - anim.hitTimer;
                    if (timeElapsed < 0) timeElapsed = 0;
                    int frameToDisplay = (int)(timeElapsed / (totalHitDuration / anim.hitFrames));

                    if (anim.hitTimer <= 0 || frameToDisplay >= anim.hitFrames) {
                        frameToDisplay = anim.hitFrames - 1;
                    }

                    float frameXOffset = 0;
                    float currentW = 45;
                    switch (frameToDisplay) {
                        case 0: frameXOffset = 0;   currentW = 45; break;
                        case 1: frameXOffset = 45;  currentW = 45; break;
                        case 2: frameXOffset = 90;  currentW = 45; break;
                        case 3: frameXOffset = 135; currentW = 65; break;
                        case 4: frameXOffset = 200; currentW = 55; break;
                        default: frameXOffset = 200; currentW = 55; break;
                    }

                    draw.part.x = anim.hitX + frameXOffset;
                    draw.part.y = anim.hitY;
                    draw.part.w = currentW;
                    draw.part.h = anim.hitH;
                    draw.size = { currentW, anim.hitH };

                    continue;
                }
                else if (intent.hit) {
                    if (anim.elapsed >= anim.frameTime) {
                        anim.elapsed = 0.0f;
                        anim.currentFrame = (anim.currentFrame + 1) % anim.attackFrames;
                    }
                    draw.part.x = anim.attackX + (anim.currentFrame * anim.frameWidth);
                    draw.part.y = anim.attackY;
                    draw.part.w = anim.attackW;
                    draw.part.h = anim.attackH;
                    draw.size = { anim.attackW, anim.attackH };
                }

                else if (mov.vx != 0 || mov.vy != 0) {
                    if (anim.elapsed >= anim.frameTime) {
                        anim.elapsed = 0.0f;
                        anim.currentFrame = (anim.currentFrame + 1) % anim.numFrames;
                    }
                    draw.part.x = anim.runX + (anim.currentFrame * anim.frameWidth);
                    draw.part.y = anim.runY;
                    draw.part.w = anim.runW;
                    draw.part.h = anim.runH;
                    draw.size = { anim.runW, anim.runH };
                }

                else {
                    draw.part = { anim.idleX, anim.idleY, anim.idleW, anim.idleH };
                    draw.size = { anim.idleW, anim.idleH };
                    anim.currentFrame = 0;
                }
            }
        }
    }

    void GoldenAxe::input_system() const {
        if (transitioning)
            return;
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

   void GoldenAxe::move_system() const {

    static const Mask movemask =
        MaskBuilder()
        .set<Intent>()
        .set<Position>()
        .set<Movement>()
        .set<Collider>()
        .build();

    static const Mask collidemask =
        MaskBuilder()
        .set<Collider>()
        .build();

    for (Entity e = Entity::first();
         !e.eof();
         e.next()) {

        if (!e.test(movemask))
            continue;

        // stun / hit freeze
        if (e.has<Animation>() &&
            e.get<Animation>().hitTimer > 0) {

            auto& mov = e.get<Movement>();

            mov.vx = 0;
            mov.vy = 0;

            continue;
        }

        auto& pos = e.get<Position>();
        auto& mov = e.get<Movement>();
        auto& intent = e.get<Intent>();
        auto& col = e.get<Collider>();

        // movement speed
        float currentMaxSpeed = speed;

        if (e.test(
            MaskBuilder()
            .set<AI>()
            .build()))
        {
            currentMaxSpeed =
                e.get<AI>().speed;
        }

        // reset velocity
        mov.vx = 0;
        mov.vy = 0;

        // input
        if (!intent.hit) {

            if (intent.left)
                mov.vx = -currentMaxSpeed;

            if (intent.right)
                mov.vx = currentMaxSpeed;

            if (intent.up)
                mov.vy = -currentMaxSpeed;

            if (intent.down)
                mov.vy = currentMaxSpeed;
        }

        // physics position
        b2Vec2 currentPos =
            b2Body_GetPosition(col.body);

        const auto& d =
            e.get<Drawable>();

        // next position
        SDL_FRect next = {

            currentPos.x * BOX_SCALE,

            currentPos.y * BOX_SCALE,

            d.size.x,

            d.size.y
        };

        next.x += mov.vx;
        next.y += mov.vy;

        // prevent float drift
        if (mov.vx == 0)
            next.x = pos.x;

        if (mov.vy == 0)
            next.y = pos.y;

        // stage bounds
        float left =
            leftBound(
                currStage,
                next.y
            );

        float right =
            rightBound(
                currStage,
                next.y
            );

        const auto& b =
            STAGE_BOUNDS[
                static_cast<int>(
                    currStage
                )
            ];

        float top =
            b.topY * SCALE_Y;

        float bottom =
            b.bottomY * SCALE_Y;

        // clamp to stage instead of blocking
        next.x = std::clamp(
            next.x,
            left,
            right - next.w
        );

        next.y = std::clamp(
            next.y,
            top,
            bottom - next.h
        );

        // collision check
        bool collided = false;

        for (Entity e1 = Entity::first();
             !e1.eof();
             e1.next()) {

            if (e1.entity().id ==
                e.entity().id)
                continue;

            if (!e1.test(collidemask))
                continue;

            const auto& c1 =
                e1.get<Collider>();

            const auto& d1 =
                e1.get<Drawable>();

            b2Vec2 p1 =
                b2Body_GetPosition(
                    c1.body
                );

            SDL_FRect rect1 = {

                p1.x * BOX_SCALE,

                p1.y * BOX_SCALE,

                d1.size.x,

                d1.size.y
            };

            if (goldenaxe::overlap(
                next,
                rect1))
            {
                collided = true;
                break;
            }
        }

        // apply movement only if no collision
        if (!collided) {

            b2Vec2 newPos = {

                next.x / BOX_SCALE,

                next.y / BOX_SCALE
            };

            b2Rot currentRotation =
                b2Body_GetRotation(
                    col.body
                );

            b2Body_SetTransform(
                col.body,
                newPos,
                currentRotation
            );

            pos.x = next.x;
            pos.y = next.y;
        }
    }
}

    bool AI::is_anyone_attacking = false;
    float AI::global_attack_cooldown = 0.0f;
    bool is_player_active = false;

    void GoldenAxe::ai_system() const {
        if (transitioning)
            return;
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

        b2SensorEvents sensorEvents = b2World_GetSensorEvents(world);
        for (int i = 0; i < sensorEvents.beginCount; ++i) {
            b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];
            id_type enemyId = (id_type)(uintptr_t)b2Body_GetUserData(b2Shape_GetBody(event.sensorShapeId));
            id_type visitorId = (id_type)(uintptr_t)b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));

            Entity enemyEnt = Entity{{enemyId}};
            Entity visitorEnt = Entity{{visitorId}};
            if (visitorEnt.test(MaskBuilder().set<Keys>().build())) {
                if (enemyEnt.test(MaskBuilder().set<AI>().build())) {
                    enemyEnt.get<AI>().is_player_in_range = true;
                }
            }
        }

        for (int i = 0; i < sensorEvents.endCount; ++i) {
            b2SensorEndTouchEvent event = sensorEvents.endEvents[i];

            id_type enemyId = (id_type)(uintptr_t)b2Body_GetUserData(b2Shape_GetBody(event.sensorShapeId));
            id_type visitorId = (id_type)(uintptr_t)b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));

            Entity enemyEnt = Entity{{enemyId}};
            Entity visitorEnt = Entity{{visitorId}};

            if (visitorEnt.test(MaskBuilder().set<Keys>().build())) {
                if (enemyEnt.test(MaskBuilder().set<AI>().build())) {
                    enemyEnt.get<AI>().is_player_in_range = false;
                }
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

                switch (ai.state) {
                    case AIState::APPROACH:
                        if (ai.is_player_in_range) {
                            if (!AI::is_anyone_attacking && AI::global_attack_cooldown <= 0) {
                                ai.state = AIState::ATTACK;
                                ai.timer = 0.6f;
                                AI::global_attack_cooldown = 1.5f;
                            } else {
                                ai.state = AIState::WAIT;
                                ai.timer = 0.5f + (rand() % 100 / 100.0f);
                            }
                        } else {
                            if (abs(dy) > 10) {
                                if (dy > 0) intent.down = true; else intent.up = true;
                            }
                            if (abs(dy) < 30) {
                                if (dx > 20) intent.right = true; else if (dx < -20) intent.left = true;
                            }
                        }
                        break;

                    case AIState::WAIT:
                        ai.timer -= deltaTime;
                        if (abs(dy) < 40) {
                            if (pos.y > heroPos.y) intent.down = true; else intent.up = true;
                        }
                        if (ai.timer <= 0) ai.state = AIState::APPROACH;
                        break;

                    case AIState::ATTACK:
                        intent.hit = true;
                        ai.timer -= deltaTime;
                        if (ai.timer <= 0) {
                            ai.state = AIState::COOLDOWN;
                            ai.timer = 0.8f;
                        }
                        break;

                    case AIState::COOLDOWN:
                        ai.timer -= deltaTime;
                        if (abs(dx) < 80) {
                            if (dx > 0) intent.left = true; else intent.right = true;
                        }
                        if (ai.timer <= 0) ai.state = AIState::APPROACH;
                        break;
                }
            }
        }
    }

    void GoldenAxe::combat_system(float deltaTime) {
        if (transitioning)
            return;
        static const Mask attackerMask = MaskBuilder().set<Intent>().set<Position>().set<Animation>().build();
        static const Mask victimMask = MaskBuilder().set<ChangeLives>().set<Position>().build();
        static const Mask scoreMask = MaskBuilder().set<Score>().build();

        for (Entity attacker = Entity::first(); !attacker.eof(); attacker.next()) {
            if (!attacker.test(attackerMask)) continue;

            auto& intent = attacker.get<Intent>();
            auto& anim = attacker.get<Animation>();

            if (intent.hit && anim.currentFrame >= 1) {
                for (Entity victim = Entity::first(); !victim.eof(); victim.next()) {
                    if (!victim.test(victimMask) || attacker.entity().id == victim.entity().id) continue;

                    const auto& aPos = attacker.get<Position>();
                    const auto& vPos = victim.get<Position>();
                    if (abs(aPos.x - vPos.x) < 70.0f && abs(aPos.y - vPos.y) < 20.0f) {
                        auto& vLife = victim.get<ChangeLives>();


                        if (vLife.invulnTimer <= 0) { //if confirms attack is fully performed

                            //Perfom the hit on the victim
                            vLife.lives -= 1;
                            vLife.invulnTimer = 0.8f;
                            if (victim.has<Animation>())
                                victim.get<Animation>().hitTimer = 0.5f;

                            //Add Score to player (if he is the attacker)
                            if (attacker.has<Score>()) {
                                auto& score = attacker.get<Score>();
                                score.points++;
                            }
                        }


                    }
                }
            }
        }

        for (Entity e = Entity::first(); !e.eof(); ) {
            bool entityWasDestroyed = false;
            if (e.has<Animation>()) {
                auto& anim = e.get<Animation>();
                if (anim.hitTimer > 0) anim.hitTimer -= deltaTime;
                if (anim.hitTimer <= 0 && anim.lieDeadTimer > 0) {
                    anim.lieDeadTimer -= deltaTime;
                }
            }

            if (e.has<ChangeLives>()) {
                auto& cl = e.get<ChangeLives>();
                if (cl.invulnTimer > 0) cl.invulnTimer -= deltaTime;
                if (cl.lives <= 0) {
                    auto& anim = e.get<Animation>();
                    if (anim.hitTimer <= 0 && anim.lieDeadTimer <= 0) {
                        if (e.has<Keys>()) {
                            resetStage(true);
                            break;
                        } else {
                            e.destroy();
                            entityWasDestroyed = true;
                        }
                    }
                }
            }

            if (!entityWasDestroyed) {
                e.next();
            }
        }
    }

    void GoldenAxe::resetStage(bool spawnHero) {

        battleFinished = false;

        const auto& s =
            SPAWNS[
                static_cast<int>(currStage)
            ];

       /* Mask m =
            MaskBuilder()
            .set<Movement>()
            .build();

        for (auto e = Entity::first();
             !e.eof();
             e.next()) {

            if (e.test(m))
                e.destroy();
             }

        is_player_active = false;*/

        // Spawn hero only if requested
        if (spawnHero) {

            ent_type hero =
                CreateHero(
                    world,
                    s.hero1.x,
                    s.hero1.y,
                    characterstex
                );

            World::addComponent(
                hero,
                Intent{
                    false,false,
                    false,false,
                    false
                }
            );

            World::addComponent(
                hero,
                Keys{
                    SDL_SCANCODE_W,
                    SDL_SCANCODE_S,
                    SDL_SCANCODE_D,
                    SDL_SCANCODE_A,
                    SDL_SCANCODE_F
                }
            );

            /*
            // Optional teammate
            CreateHero(
                world,
                s.hero2.x,
                s.hero2.y,
                characterstex
            );
            */
        }

        // Enemy 1
        CreateEnemy(
            world,
            s.enemy1.x,
            s.enemy1.y,
            55,
            70,
            4,
            enemiestex
        );

        // Enemy 2
        CreateEnemy(
            world,
            s.enemy2.x,
            s.enemy2.y,
            55,
            70,
            4,
            enemiestex
        );
    }

    void GoldenAxe::run() {
        resetStage(true);
        bool quit = false;
        const float dt = 1.0f / 60.0f;
        SDL_Event event;

        while (!quit) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) quit = true;
            }

            b2World_Step(world, dt, 4);
            input_system();
            ai_system();
            combat_system(dt);

            if (battleOver() &&!battleFinished) {
                battleFinished = true;
                startStageTransition();
            }

            transition_system();
            move_system();
            draw_system();
            animation_system(dt);

            SDL_Delay(16);
        }
        SDL_Quit();
    }
}