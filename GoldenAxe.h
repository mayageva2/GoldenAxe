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

    struct Score {
        int points = 0;
    };

    enum class AIType { CHASER, RUNNER };
    struct AI {
        bool active = true;
        float speed = 1.2f;
        AIType type = AIType::CHASER; // default chaser
    };

    struct Hit {
        bool is_attacking = false;
        int damage = 1;
    };

    struct FlaskUsage {
        int current_flasks = 0;
        int goal = 5;
    };

    // For Score System
    struct EnemyKilledEvent {};
    struct FlaskCollectedEvent {};

    // Entities Creation
    static ent_type CreateHero(float x, float y) {
        ent_type hero = World::createEntity();
        World::addComponent(hero, Position{x, y});
        World::addComponent(hero, Movement{0, 0});
        World::addComponent(hero, ChangeLives{3, 1});
        World::addComponent(hero, Hit{false, 1});
        World::addComponent(hero, FlaskUsage{0, 5});
        World::addComponent(hero, Score{0});
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

    static ent_type CreateSanta(float x, float y) {
        ent_type santa = World::createEntity();
        World::addComponent(santa, Position{x, y});
        World::addComponent(santa, Movement{0, 0});
        World::addComponent(santa, ChangeLives{1, 0});
        World::addComponent(santa, AI{true, 2.0f, AIType::RUNNER}); //santa is faster and running from player
        return santa;
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

                    if (enemy_pos.x < player_pos.x) {
                        enemy_mov.vx = (enemy_ai.type == AIType::CHASER) ? enemy_ai.speed : -enemy_ai.speed;
                    } else {
                        enemy_mov.vx = (enemy_ai.type == AIType::CHASER) ? -enemy_ai.speed : enemy_ai.speed;
                    }
                }
            }
        }

        static void useMagic(ent_type player) {
            auto& flasks = World::getComponent<FlaskUsage>(player);

            if (flasks.current_flasks >= flasks.goal) {
                std::cout << "!!! CASTING ULTIMATE MAGIC !!!" << std::endl;

                for (int i = 0; i <= World::maxId(); ++i) {
                    ent_type e{i};
                    if (World::mask(e).test(Component<AI>::Bit)) {
                        auto& lives = World::getComponent<ChangeLives>(e);
                        if (lives.lives > 0) {
                            lives.lives = 0;
                            World::addComponent(e, EnemyKilledEvent{});
                        }
                    }
                }

                flasks.current_flasks = 0;
            } else {
                std::cout << "Not enough flasks for magic! Current: "
                          << flasks.current_flasks << "/" << flasks.goal << std::endl;
            }
        }

        static void collectFlask(ent_type player) {
            auto& flasks = World::getComponent<FlaskUsage>(player);

            if (flasks.current_flasks < flasks.goal) {
                flasks.current_flasks++;
                World::addComponent(player, FlaskCollectedEvent{});

                std::cout << "Flask Collected! Current Magic: "
                          << flasks.current_flasks << "/" << flasks.goal << std::endl;
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

                    if (victim_stats.lives <= 0 && !World::mask(victim).test(Component<EnemyKilledEvent>::Bit)) {                        World::addComponent(victim, EnemyKilledEvent{});
                        World::addComponent(victim, EnemyKilledEvent{});
                    }

                    hit.is_attacking = false;
                }
            }
        }
    };

    class ScoreSystem {
    public:
        static void update(ent_type player) {
            auto& score = World::getComponent<Score>(player);

            for (int i = 0; i <= World::maxId(); ++i) {
                ent_type e{i};
                auto mask = World::mask(e);

                if (mask.test(Component<EnemyKilledEvent>::Bit)) {
                    score.points += 100;
                    //World::removeComponent<EnemyKilledEvent>(e);
                }

                if (mask.test(Component<FlaskCollectedEvent>::Bit)) {
                    score.points += 50;
                    //World::removeComponent<FlaskCollectedEvent>(e);
                }
            }
        }
    };
}