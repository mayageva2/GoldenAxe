#include "GoldenAxe.h"
#include <iostream>
using namespace std;

#include "bagel.h"
using namespace bagel;

namespace goldenaxe {

    constexpr Drawable GoldenAxe::makeDrawable(SDL_FRect part, SDL_Texture* texture) {
        return Drawable{{part}, {part.w*TEX_SCALE, part.h*TEX_SCALE},texture};
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
            Movement{speed,speed},
            Intent{false,false,false,false},
            makeDrawable(HERO_IDLE,characterstex),
            Collider{});

        Entity::create().addAll(
            Position{leftStartingPosition,bottomStartingPosition},
            Movement{speed,speed},
            Intent{false,false,false,false},
            AI{false,speed,AIType::RUNNER},
            makeDrawable(HERO_IDLE,characterstex),
            Collider{});

        Entity::create().addAll(
            Position{rightStartingPosition,upperStartingPosition},
            Movement{speed,speed},
            Intent{false,false,false,false},
            AI{false,speed,AIType::CHASER},
            makeDrawable(ENEMY_IDLE,enemiestex),
            Collider{});

        Entity::create().addAll(
            Position{rightStartingPosition,bottomStartingPosition},
            Movement{speed,speed},
            Intent{false,false,false,false},
            AI{false,speed,AIType::CHASER},
            makeDrawable(ENEMY_IDLE,enemiestex),
            Collider{});
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

        resetStage();
        draw_system();

        SDL_Delay(5000);
        SDL_Quit();
    }
}