#include <iostream>
#include "bagel.h"
#include "GoldenAxe.h"


using namespace bagel;
using namespace GoldenAxe;

int main() {

    if (!SDL_Init(SDL_INIT_VIDEO))


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
        ScoreSystem::update(player);
        auto& score = World::getComponent<Score>(player);
        std::cout << "Current Score: " << score.points << std::endl;
    }

    std::cout << "\n--- Testing Magic System & Collection ---" << std::endl;

    std::cout << "Collecting a flask..." << std::endl;
    GameplaySystems::collectFlask(player);
    ScoreSystem::update(player);

    auto& playerFlasks = World::getComponent<FlaskUsage>(player);
    playerFlasks.current_flasks = 5;

    std::cout << "Using Magic!" << std::endl;
    GameplaySystems::useMagic(player);
    ScoreSystem::update(player);

    auto& finalScore = World::getComponent<Score>(player);
    std::cout << "\nFinal Score: " << finalScore.points << std::endl;
    std::cout << "Test Finished." << std::endl;

    return 0;
}