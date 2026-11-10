/*
** EPITECH PROJECT, 2026
** j-type/tests/client/SoundCues
** File description:
** Unit tests of the sounds found in the messages of the server
*/

#include <criterion/criterion.h>

#include <vector>

#include "audio/SoundCues.hpp"
#include "jtype/common/EntityType.hpp"
#include "jtype/common/Messages.hpp"

using jtype::DeathEventMessage;
using jtype::EntityState;
using jtype::EntityType;
using jtype::getEventSound;
using jtype::ScoreboardMessage;
using jtype::SlotState;
using jtype::SnapshotMessage;
using jtype::Sound;
using jtype::SpawnMessage;

namespace {

// The world drawn before the events: the ship of the player of slot 1, with the id 3, and a Bydo with the id 4
SnapshotMessage worldOf(void)
{
    return SnapshotMessage{
        .tick = 0,
        .entities = std::vector<EntityState>{EntityState{3, EntityType::Player2, 0.0F, 0.0F, 1},
                                             EntityState{4, EntityType::Blobling, 0.0F, 0.0F, 1}}
    };
}

Sound soundOf(const jtype::Message &event, const ScoreboardMessage &scoreboard = {})
{
    const auto sound = getEventSound(event, worldOf(), scoreboard);

    cr_assert(sound.has_value());
    return *sound;
}

} // namespace

Test(client_sound_cues, shots_and_explosions_that_appear_play_their_sound)
{
    cr_assert(soundOf(SpawnMessage{
                  1, EntityState{9, EntityType::Missile3, 0.0F, 0.0F, 0}
    }) == Sound::PlayerShot);
    cr_assert(soundOf(SpawnMessage{
                  1, EntityState{9, EntityType::EnemyMissile, 0.0F, 0.0F, 0}
    }) == Sound::EnemyShot);
    cr_assert(soundOf(SpawnMessage{
                  1, EntityState{9, EntityType::Explosion, 0.0F, 0.0F, 0}
    }) == Sound::Explosion);
    cr_assert_not(getEventSound(SpawnMessage{
                                    1, EntityState{9, EntityType::Jelly, 0.0F, 0.0F, 2}
    },
                                worldOf(), {}));
}

Test(client_sound_cues, a_ship_destroyed_plays_the_hit_or_the_last_life)
{
    ScoreboardMessage scoreboard;

    scoreboard.players[1].state = SlotState::Playing;
    cr_assert(soundOf(DeathEventMessage{1, 3, jtype::NO_PLAYER}, scoreboard) == Sound::PlayerHit);
    scoreboard.players[1].state = SlotState::Out;
    cr_assert(soundOf(DeathEventMessage{1, 3, jtype::NO_PLAYER}, scoreboard) == Sound::PlayerOut);
}

Test(client_sound_cues, a_bydo_destroyed_plays_only_its_explosion)
{
    cr_assert_not(getEventSound(DeathEventMessage{1, 4, 0}, worldOf(), {}).has_value());
    cr_assert_not(getEventSound(DeathEventMessage{1, 99, 0}, worldOf(), {}).has_value());
}

Test(client_sound_cues, the_game_over_plays_its_sound)
{
    cr_assert(soundOf(jtype::GameOverEventMessage{1}) == Sound::GameOver);
    cr_assert_not(getEventSound(jtype::DestroyMessage{1, 4}, worldOf(), {}).has_value());
}

Test(client_sound_cues, a_victory_plays_no_game_over)
{
    ScoreboardMessage scoreboard;

    scoreboard.gameOver = true;
    scoreboard.victory = true;
    cr_assert_not(getEventSound(jtype::GameOverEventMessage{1}, worldOf(), scoreboard).has_value());
}

Test(client_sound_cues, shots_and_wreck_of_the_boss_play_their_sound)
{
    cr_assert(soundOf(SpawnMessage{
                  1, EntityState{9, EntityType::IncubatorOrb, 0.0F, 0.0F, 0}
    }) == Sound::EnemyShot);
    cr_assert(soundOf(SpawnMessage{
                  1, EntityState{9, EntityType::IncubatorSpit, 0.0F, 0.0F, 0}
    }) == Sound::EnemyShot);
    cr_assert(soundOf(SpawnMessage{
                  1, EntityState{9, EntityType::IncubatorWreck, 0.0F, 0.0F, 0}
    }) == Sound::Explosion);
    cr_assert_not(getEventSound(SpawnMessage{
                                    1, EntityState{9, EntityType::IncubatorBody, 0.0F, 0.0F, 0}
    },
                                worldOf(), ScoreboardMessage{})
                      .has_value());
}

Test(client_sound_cues, pellets_play_the_shot_and_power_ups_appear_silently)
{
    cr_assert(soundOf(SpawnMessage{
                  1, EntityState{9, EntityType::SpreadPellet2, 0.0F, 0.0F, 0}
    }) == Sound::PlayerShot);
    cr_assert_not(getEventSound(SpawnMessage{
                                    1, EntityState{9, EntityType::ShieldPowerUp, 0.0F, 0.0F, 0}
    },
                                worldOf(), ScoreboardMessage{})
                      .has_value());
    cr_assert_not(getEventSound(jtype::PickUpEventMessage{1, 0, jtype::PowerUpType::Shield}, worldOf(), ScoreboardMessage{}).has_value());
}
