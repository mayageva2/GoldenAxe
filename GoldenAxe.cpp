#include "GoldenAxe.h"
#include <iostream>
using namespace std;

#include "bagel.h"
using namespace bagel;

namespace goldenaxe {

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


        Mask mdraw= MaskBuilder().set<Drawable>()
                             .build();
        for (auto e = Entity::first();!e.eof(); e.next()) {
            if (e.test(mdraw)) {
              const Drawable& d = e.get<Drawable>();
              const Position& p = e.get<Position>();
              SDL_FRect dest=  {p.x,p.y,d.size.x,d.size.y};
              SDL_RenderTexture(ren,d.texture,&d.part,&dest);
            }
        }
        SDL_RenderPresent(ren);
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

                SDL_FRect next = col.part;

                next.x += mov.vx;
                next.y += mov.vy;
                if (outofbounds(next))
                    canmove=false;

                for (Entity e1 = Entity::first(); !e1.eof() && canmove; e1.next()) {
                    if (e1==e) continue;
                    if (e1.test(collidemask)) {
                        const auto& c1= e1.get<Collider>();
                        if (overlap(next,c1.part))
                            canmove=false;
                    }
                }
                if (canmove) {

                    auto& intent = e.get<Intent>();
                    auto& pos = e.get<Position>();
                    auto& mov = e.get<Movement>();

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

                    pos.x = next.x;
                    pos.y = next.y;

                    col.part = next;
                }
            }
        }
    }

    void GoldenAxe::resetStage() const {

        //Delete all the entities with ability to move
        Mask m= MaskBuilder().set<Movement>()
                             .build();
        for (auto e = Entity::first();!e.eof(); e.next()) {
            if (e.test(m)) {
                e.destroy();
            }
        }

        //Add 1 player, 1 teammate, 2 enemies
        Entity::create().addAll(
            Position{leftStartingPosition,upperStartingPosition},
            Movement{0,0},
            Intent{false,false,false,false},
            Keys{SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_D,SDL_SCANCODE_A,SDL_SCANCODE_F},
            Collider{colliderRect({leftStartingPosition,upperStartingPosition},makeDrawable(HERO_IDLE,characterstex))},
            makeDrawable(HERO_IDLE,characterstex));

        Entity::create().addAll(
            Position{leftStartingPosition,bottomStartingPosition},
            Movement{0,0},
            Intent{false,false,false,false},
            AI{false,speed,AIType::RUNNER},
            Collider{colliderRect({leftStartingPosition,bottomStartingPosition},makeDrawable(HERO_IDLE,characterstex))},
            makeDrawable(HERO_IDLE,characterstex));

        Entity::create().addAll(
            Position{rightStartingPosition,upperStartingPosition},
            Movement{0,0},
            Intent{false,false,false,false},
            AI{false,speed,AIType::CHASER},
            Collider{colliderRect({rightStartingPosition,upperStartingPosition},makeDrawable(ENEMY_IDLE,enemiestex))},
            makeDrawable(ENEMY_IDLE,enemiestex));

        Entity::create().addAll(
            Position{rightStartingPosition,bottomStartingPosition},
            Movement{0,0},
            Intent{false,false,false,false},
            AI{false,speed,AIType::CHASER},
            Collider{colliderRect({rightStartingPosition,bottomStartingPosition},makeDrawable(ENEMY_IDLE,enemiestex))},
            makeDrawable(ENEMY_IDLE,enemiestex));
    }

    GoldenAxe::GoldenAxe() {

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            cout << SDL_GetError() << endl;
            return;
        }

        // create game window
        if (!SDL_CreateWindowAndRenderer(
            "Golden Axe", WIN_W, WIN_H, 0, &win, &ren)) {
            cout << SDL_GetError() << endl;
            return;
        }

        // load spritesheets images
        SDL_Surface *surf = IMG_Load(CHARACTERS_FILE);
        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return;
        }
        characterstex = SDL_CreateTextureFromSurface(ren, surf);

        surf = IMG_Load(ENEMIES_FILE);
        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return;
        }
        enemiestex = SDL_CreateTextureFromSurface(ren, surf);

        surf = IMG_Load(FLASK_FILE);
        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return;
        }
        flasktex = SDL_CreateTextureFromSurface(ren, surf);

        surf = IMG_Load(SANTA_FILE);
        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return;
        }
        santatex = SDL_CreateTextureFromSurface(ren, surf);

        surf = IMG_Load(STAGE_FILE);
        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return;
        }
        stagetex = SDL_CreateTextureFromSurface(ren, surf);
    }

    void GoldenAxe::run() {
        resetStage();
        bool quit =false;
        while (!quit) {
            input_system();
            move_system();
            draw_system();
        }
        SDL_Delay(5000);
        SDL_Quit();
    }
}
