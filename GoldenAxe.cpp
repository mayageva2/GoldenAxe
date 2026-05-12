#include "GoldenAxe.h"
#include <iostream>
#include "bagel.h"

using namespace std;
using namespace bagel;

namespace goldenaxe {

    GoldenAxe::GoldenAxe() {
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
            42, 20.0f});

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
            315,10,60, 75,
            75,-15.0f});
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
                float finalX = p.x;

                if (mov.vx < 0) {
                    flip = SDL_FLIP_HORIZONTAL;
                    if (e.test(MaskBuilder().set<Animation>().build())) {
                        finalX += e.get<Animation>().flipOffsetX;
                    }
                }
                dest.x = finalX;
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

    void GoldenAxe::resetStage() {
        Mask m = MaskBuilder().set<Movement>().build();
        for (auto e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(m)) e.destroy();
        }

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
            move_system();
            draw_system();
            animation_system(1.0f / 60.0f);
            SDL_Delay(16);
        }
        SDL_Quit();
    }
}