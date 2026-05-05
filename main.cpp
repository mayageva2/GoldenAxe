#include <iostream>
#include "bagel.h"
#include "GoldenAxe.h"

using namespace bagel;
using namespace GoldenAxe;

int main() {
    std::cout << "--- Golden Axe Logic Test ---" << std::endl;

    auto player = CreateHero(100.0f, 100.0f);
    auto enemy = CreateEnemy(150.0f, 100.0f);

    std::cout << "Hero and Enemy created." << std::endl;

    for (int frame = 0; frame < 5; ++frame) {
        std::cout << "\n--- Frame " << frame << " ---" << std::endl;
        GameplaySystems::UpdateAI(player);

        auto& p_pos = World::getComponent<Position>(player);
        auto& e_pos = World::getComponent<Position>(enemy);
        auto& e_mov = World::getComponent<Movement>(enemy);

        e_pos.x += e_mov.vx;
        std::cout << "Enemy Position X: " << e_pos.x << std::endl;

        if (frame == 3) {
            std::cout << ">> Player Pressing Attack! <<" << std::endl;
            auto& hit = World::getComponent<Hit>(player);
            hit.is_attacking = true;
        }

        CombatSystem::checkAttack(player, enemy);
    }

    std::cout << "\nTest Finished." << std::endl;
    return 0;
}