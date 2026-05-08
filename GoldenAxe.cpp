#include "GoldenAxe.h"
#include <iostream>
using namespace std;

#include "bagel.h"
using namespace bagel;

namespace GoldenAxe {

    constexpr Drawable GoldenAxe::makeDrawable(SDL_FRect part) {
        return Drawable{{part}, {part.w*TEX_SCALE, part.h*TEX_SCALE}};
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
            Collider{});

        Entity::create().addAll(
            Position{leftStartingPosition,bottomStartingPosition},
            Movement{speed,speed},
            Intent{false,false,false,false},
            AI{false,speed,AIType::RUNNER},
            Collider{});

        Entity::create().addAll(
            Position{rightStartingPosition,upperStartingPosition},
            Movement{speed,speed},
            Intent{false,false,false,false},
            AI{false,speed,AIType::CHASER},
            Collider{});

        Entity::create().addAll(
            Position{rightStartingPosition,bottomStartingPosition},
            Movement{speed,speed},
            Intent{false,false,false,false},
            AI{false,speed,AIType::CHASER},
            Collider{});


    }

    GoldenAxe::GoldenAxe() {

    }
}