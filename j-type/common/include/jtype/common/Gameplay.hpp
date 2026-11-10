/*
** EPITECH PROJECT, 2026
** j-type/common/include/jtype/common/Gameplay
** File description:
** Gameplay header file
*/

#pragma once

#include <cstdint>

#include "younity/core/Vector2.hpp"

namespace jtype {

// In pixels per second
inline constexpr float PLAYER_SPEED = 150.0F;
inline constexpr float MISSILE_SPEED = 360.0F;
inline constexpr float BYDO_SPEED = 80.0F;
inline constexpr float ENEMY_MISSILE_SPEED = 160.0F;

// In seconds
inline constexpr float SHOOT_COOLDOWN = 0.25F;
// Between two shots of a Bydo
inline constexpr float ENEMY_SHOOT_DELAY = 3.0F;
// Between the moment a Bydo enters the screen and its first shot, so it never shoots from off-screen
inline constexpr float ENEMY_FIRST_SHOT_DELAY = 1.0F;
// How long a Bydo chases the players before it flies away to the left, in seconds since it appeared: about 4 s
// on screen, so a chaser left alive leaves and the next wave can come
inline constexpr float CHASE_DURATION = 5.5F;

// Each round of the waves multiplies the delays between the Bydos by it, down to MIN_DELAY_FACTOR
inline constexpr float WAVE_SPEEDUP = 0.85F;
inline constexpr float MIN_DELAY_FACTOR = 0.3F;

// The lives of a player when the game starts, and the most an extra life can bring
inline constexpr std::uint8_t PLAYER_LIVES = 3;
inline constexpr std::uint8_t MAX_LIVES = 5;
// The time between the game over and the next game, in seconds
inline constexpr float GAME_OVER_DELAY = 5.0F;

// The health points of a ship: one hit destroys it, unless a shield adds one
inline constexpr int PLAYER_HEALTH = 1;
inline constexpr int SHIELDED_HEALTH = PLAYER_HEALTH + 1;
// The time a ship cannot be hurt after it came back, in seconds
inline constexpr float RESPAWN_INVULNERABILITY = 2.0F;
// How long an explosion is shown, in seconds
inline constexpr float EXPLOSION_DURATION = 0.4F;
// Where the ships start, from the left of the screen
inline constexpr float PLAYER_START_X = 60.0F;

// The wave of a Bydo flying in a sine wave: how far it goes up and down, in pixels, and how many waves per second
inline constexpr float SINE_AMPLITUDE = 30.0F;
inline constexpr float SINE_FREQUENCY = 0.75F;

// The position of an entity is the origin of its sprite: the box of the sprite around it, in pixels
inline constexpr younity::Vector2f PLAYER_ORIGIN{24.0F, 19.0F};
inline constexpr younity::Vector2f PLAYER_SIZE{40.0F, 36.0F};
inline constexpr younity::Vector2f BYDO_ORIGIN{12.0F, 12.0F};
inline constexpr younity::Vector2f BYDO_SIZE{24.0F, 24.0F};
inline constexpr younity::Vector2f MISSILE_SIZE{16.0F, 8.0F};

// The hitboxes, smaller than the sprites: the size, and the offset of the top left corner from the position
// The body of the technician of a ship: the gun and the thruster stay out of it
inline constexpr younity::Vector2f PLAYER_HITBOX_SIZE{14.0F, 20.0F};
inline constexpr younity::Vector2f PLAYER_HITBOX_OFFSET{-4.0F, -10.0F};
inline constexpr younity::Vector2f BYDO_HITBOX_SIZE{17.0F, 17.0F};
inline constexpr younity::Vector2f BYDO_HITBOX_OFFSET{-8.0F, -8.0F};
inline constexpr younity::Vector2f MISSILE_HITBOX_SIZE{12.0F, 4.0F};
inline constexpr younity::Vector2f MISSILE_HITBOX_OFFSET{-6.0F, -2.0F};
inline constexpr younity::Vector2f ENEMY_MISSILE_HITBOX_SIZE{6.0F, 6.0F};
inline constexpr younity::Vector2f ENEMY_MISSILE_HITBOX_OFFSET{-3.0F, -3.0F};

// The ceiling and the floor of the stage, along the top and the bottom of the screen: a ship that touches them loses a
// life, and the Bydos, the formations and the boss stay between them
inline constexpr float TERRAIN_HEIGHT = 32.0F;
// The walls and the zappers scroll to the left with the stage, as fast as the floor and the ceiling are drawn, in pixels
// per second
inline constexpr float STAGE_SPEED = 40.0F;
// A block of a wall and a piece of a zapper, with the hitboxes of a block, of a node and of the thinner beam
inline constexpr younity::Vector2f OBSTACLE_SIZE{16.0F, 16.0F};
inline constexpr int BLOCK_HEALTH = 2;
inline constexpr younity::Vector2f BLOCK_HITBOX_SIZE{16.0F, 16.0F};
inline constexpr younity::Vector2f BLOCK_HITBOX_OFFSET{-8.0F, -8.0F};
inline constexpr younity::Vector2f ZAPPER_NODE_HITBOX_SIZE{12.0F, 12.0F};
inline constexpr younity::Vector2f ZAPPER_NODE_HITBOX_OFFSET{-6.0F, -6.0F};
inline constexpr younity::Vector2f ZAPPER_BEAM_HITBOX_SIZE{6.0F, 16.0F};
inline constexpr younity::Vector2f ZAPPER_BEAM_HITBOX_OFFSET{-3.0F, -8.0F};

// How far past the right edge the Bydos appear, so they always fly in from
// off-screen and never pop on top of a player hugging the right side
inline constexpr float BYDO_SPAWN_MARGIN = 120.0F;
// Between two Bydos of a column or of a V, up and down and, for a V, back, in pixels: a Bydo is 24 pixels high
inline constexpr float FORMATION_GAP = 32.0F;

// Where the missiles leave the ship, from its position
inline constexpr younity::Vector2f MUZZLE_OFFSET{15.0F, 0.0F};
// The angle between two pellets of the spread shot, in radians
inline constexpr float SPREAD_ANGLE = 0.2F;
inline constexpr younity::Vector2f PELLET_HITBOX_SIZE{6.0F, 6.0F};
inline constexpr younity::Vector2f PELLET_HITBOX_OFFSET{-3.0F, -3.0F};

// A power-up drifts to the left at this speed, in pixels per second, slower than the Bydos so the ships can reach it
inline constexpr float POWER_UP_SPEED = 40.0F;
inline constexpr younity::Vector2f POWER_UP_SIZE{16.0F, 16.0F};
inline constexpr younity::Vector2f POWER_UP_HITBOX_SIZE{14.0F, 14.0F};
inline constexpr younity::Vector2f POWER_UP_HITBOX_OFFSET{-7.0F, -7.0F};

// The boss: the health of its eye, and what each player after the first adds, at most 255 for the snapshots
inline constexpr int BOSS_HEALTH = 60;
inline constexpr int BOSS_HEALTH_PER_PLAYER = 20;
// The points of the player whose missile destroys the eye
inline constexpr std::uint32_t BOSS_POINTS = 5000;
// The frame of the body: its position is the center, and it stops with its right side on the right edge
inline constexpr younity::Vector2f BOSS_SIZE{128.0F, 144.0F};
// It comes in from the right at this speed, in pixels per second, then rises and falls around the middle of the
// screen: how far, in pixels, and how many times per second
inline constexpr float BOSS_ENTER_SPEED = 40.0F;
// The body stays between the ceiling and the floor: (270 - 2 * 32 - 144) / 2 = 31 pixels each way at most
inline constexpr float BOSS_RISE_AMPLITUDE = 30.0F;
inline constexpr float BOSS_RISE_FREQUENCY = 0.1F;
// Its attacks, in seconds between two, and faster once the eye has lost half of its health
inline constexpr float BOSS_ORB_DELAY = 1.5F;
inline constexpr float BOSS_ANGRY_ORB_DELAY = 1.0F;
inline constexpr float BOSS_SPIT_DELAY = 4.0F;
inline constexpr float BOSS_ANGRY_SPIT_DELAY = 3.0F;
// A fan of spits: how many, and the angle between two, in radians, so a ship fits between them away from the mouth
inline constexpr int BOSS_SPIT_COUNT = 5;
inline constexpr float BOSS_SPIT_SPREAD = 0.35F;
// How long the wreck is shown, in seconds
inline constexpr float BOSS_WRECK_DURATION = 2.0F;
// From the position of the body: the eye, where the orbs and the spits leave the mouth, and the box of the armored body.
// The box leaves out the head, so the missiles reach the eye, and starts more than a missile away from the eye, so a
// missile never touches both
inline constexpr younity::Vector2f BOSS_EYE_OFFSET{-27.0F, -15.0F};
inline constexpr younity::Vector2f BOSS_MOUTH_OFFSET{-56.0F, 2.0F};
inline constexpr younity::Vector2f BOSS_BODY_HITBOX_SIZE{68.0F, 140.0F};
inline constexpr younity::Vector2f BOSS_BODY_HITBOX_OFFSET{-6.0F, -70.0F};
inline constexpr younity::Vector2f BOSS_EYE_HITBOX_SIZE{16.0F, 16.0F};
inline constexpr younity::Vector2f BOSS_EYE_HITBOX_OFFSET{-8.0F, -8.0F};

} // namespace jtype
