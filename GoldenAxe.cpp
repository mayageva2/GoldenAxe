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
        Mask::bit_type index = Component<Movement>::Bit;


    }

    GoldenAxe::GoldenAxe() {

    }
}