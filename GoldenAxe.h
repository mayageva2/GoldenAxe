#include "bagel.h"

using namespace bagel;

namespace GoldenAxe {

    // Structs Definitions
    struct Position { float x, y; };
    struct Movement { float vx, vy; };

    struct ChangeLives {
        int lives = 3;
        int credits = 1;
    };

    struct AI {
        bool active = true;
        float speed = 1.2f;
    };

    struct Hit {
        bool is_attacking = false;
        int damage = 1;
    };

    struct FlaskUsage {
        int current_flasks = 0;
        int goal = 5;
    };

    // Entities Creation
    static ent_type CreateHero(float x, float y) {
        ent_type hero = World::createEntity();
        World::addComponent(hero, Position{x, y});
        World::addComponent(hero, Movement{0, 0});
        World::addComponent(hero, ChangeLives{3, 1});
        World::addComponent(hero, Hit{false, 1});
        World::addComponent(hero, FlaskUsage{0, 5});
        return hero;
    }

    static ent_type CreateEnemy(float x, float y) {
        ent_type enemy = World::createEntity();
        World::addComponent(enemy, Position{x, y});
        World::addComponent(enemy, Movement{0, 0});
        World::addComponent(enemy, ChangeLives{1, 0});
        World::addComponent(enemy, AI{true, 1.0f});
        return enemy;
    }

    // Systems Implementations
    class GameplaySystems {
    public:
        static void UpdateAI(ent_type player_entity) {
            auto& player_pos = World::getComponent<Position>(player_entity);

            for (int i = 0; i <= World::maxId(); ++i) {
                ent_type e{i};
                if (World::mask(e).test(Component<AI>::Bit) &&
                    World::mask(e).test(Component<Movement>::Bit)) {

                    auto& enemy_pos = World::getComponent<Position>(e);
                    auto& enemy_mov = World::getComponent<Movement>(e);
                    auto& enemy_ai = World::getComponent<AI>(e);

                    if (enemy_pos.x < player_pos.x) enemy_mov.vx = enemy_ai.speed;
                    else enemy_mov.vx = -enemy_ai.speed;
                    }
            }
        }
    };

    class CombatSystem {
    public:
        static void checkAttack(ent_type attacker, ent_type victim) {
            auto& hit = World::getComponent<Hit>(attacker);
            auto& victim_stats = World::getComponent<ChangeLives>(victim);
            auto& attacker_pos = World::getComponent<Position>(attacker);
            auto& victim_pos = World::getComponent<Position>(victim);

            if (hit.is_attacking) {
                float dist = std::abs(attacker_pos.x - victim_pos.x);
                if (dist < 50.0f) {
                    victim_stats.lives -= hit.damage;
                    std::cout << "Hit! Victim lives left: " << victim_stats.lives << std::endl;

                    hit.is_attacking = false;
                }
            }
        }
    };
}