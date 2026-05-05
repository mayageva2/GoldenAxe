#include <iostream>
#include "bagel.h"
#include "GoldenAxe.h"

using namespace bagel;
using namespace GoldenAxe;

int main() {
    std::cout << "--- Golden Axe Logic Test ---" << std::endl;

    auto player = CreateHero(100.0f, 100.0f);
    auto enemy = CreateEnemy(150.0f, 100.0f);
    auto santa = CreateSanta(80.0f, 100.0f);

    std::cout << "Hero and Enemy created." << std::endl;

    for (int frame = 0; frame < 5; ++frame) {
        std::cout << "\n--- Frame " << frame << " ---" << std::endl;
        GameplaySystems::UpdateAI(player);

        auto& e_pos = World::getComponent<Position>(enemy);
        auto& e_mov = World::getComponent<Movement>(enemy);
        e_pos.x += e_mov.vx;
        std::cout << "Enemy (Chaser) Pos X: " << e_pos.x << " (vx: " << e_mov.vx << ")" << std::endl;

        auto& s_pos = World::getComponent<Position>(santa);
        auto& s_mov = World::getComponent<Movement>(santa);
        s_pos.x += s_mov.vx;
        std::cout << "Santa (Runner) Pos X: " << s_pos.x << " (vx: " << s_mov.vx << ")" << std::endl;

        if (frame == 3) {
            std::cout << ">> Player Pressing Attack! <<" << std::endl;
            auto& hit = World::getComponent<Hit>(player);
            hit.is_attacking = true;
        }

        CombatSystem::checkAttack(player, enemy);
    }

    std::cout << "\n--- Testing Magic System ---" << std::endl;

    auto& playerFlasks = World::getComponent<FlaskUsage>(player);
    playerFlasks.current_flasks = 5;

    GameplaySystems::useMagic(player);
    auto& enemyLives = World::getComponent<ChangeLives>(enemy);
    
    std::cout << "Enemy lives after magic: " << enemyLives.lives << std::endl;
    std::cout << "\nTest Finished." << std::endl;
    return 0;
}