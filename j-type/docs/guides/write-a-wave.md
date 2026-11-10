# Write a wave

The Bydos of a game come from `assets/data/waves.txt`. The server reads it when it starts, so a change is played at the next start of `./r-type_server`, without building the game again. A wrong file stops the server with the line and the reason, e.g. `line 12: unknown formation "circle"`.

## The file

A `wave <pause>` line starts a wave. Each line after it brings a group of Bydos:

```text
<type> <pattern> <count> <delay> [<formation> <height>] [drop]
```

| Field       | Values                                     | Meaning                                                    |
|-------------|--------------------------------------------|------------------------------------------------------------|
| `pause`     | Seconds, 0 or more                         | The calm before the wave, once the wave before is over     |
| `type`      | `blobling`, `jelly`, `sentry_drone`        | The kind of Bydo                                           |
| `pattern`   | `straight`, `sine_wave`, `chase`           | How it moves                                               |
| `count`     | 1 or more                                  | How many Bydos the group brings for one player             |
| `delay`     | Seconds, 0 or more                         | Between two Bydos of the group: 0 brings them all at once  |
| `formation` | `random`, `line`, `column`, `v`            | Where they appear, `random` when it is left out            |
| `height`    | From 0, the top of the screen, to 1, the bottom | The center of the formation, left out for `random`    |
| `drop`      | The word `drop`, or nothing                | The last Bydo of the group carries a power-up            |

A `wall <top> <bottom>` or `zapper <top> <bottom>` line brings an obstacle with the wave, between two heights from 0, against the ceiling, to 1, against the floor, the top one first. A `boss incubator` line brings the boss with the wave: a wave has one boss at most. Empty lines and the lines starting with `#` are skipped.

## The Bydos

| Type           | Health | Points | Shoots                                   |
|----------------|--------|--------|------------------------------------------|
| `blobling`     | 1      | 100    | Never                                    |
| `jelly`        | 2      | 200    | Straight to the left                     |
| `sentry_drone` | 3      | 300    | At the nearest player                    |

A Bydo that shoots fires 1 second after it enters the screen (`ENEMY_FIRST_SHOT_DELAY`), then every 3 seconds (`ENEMY_SHOOT_DELAY`), in `common/include/jtype/common/Gameplay.hpp`.

| Pattern     | Movement                                                                        |
|-------------|---------------------------------------------------------------------------------|
| `straight`  | To the left                                                                     |
| `sine_wave` | To the left, waving up and down                                                 |
| `chase`     | Towards the nearest player, then to the left after `CHASE_DURATION` (5.5 s)      |

## The formations

Each Bydo appears just past the right edge of the screen, then flies in.

| Formation | Where the Bydos appear                                                                                   |
|-----------|----------------------------------------------------------------------------------------------------------|
| `random`  | Each one at its own random height                                                                        |
| `line`    | All at the height: with a delay, one behind the other                                                     |
| `column`  | Stacked from top to bottom, `FORMATION_GAP` (32 pixels) apart, centered on the height: with no delay, a wall |
| `v`       | The first one at the height, then by pairs above and below it, each pair one `FORMATION_GAP` further back  |

A formation that does not fit is pushed back inside the screen.

## The stage

The ceiling and the floor are `TERRAIN_HEIGHT` (32) pixels high, along the whole game: the Bydos appear between them, the boss stays between them, and a ship that touches them loses a life. The heights of the groups and of the obstacles go from 0 to 1 between them.

| Obstacle | Sprites                  | What it does                                                                          |
|----------|--------------------------|---------------------------------------------------------------------------------------|
| `wall`   | `block`, `block_cracked` | A column of 16-pixel blocks; each one breaks after `BLOCK_HEALTH` (2) missiles of the players |
| `zapper` | `zapper`, `zapper_beam_v` | A node at each end and a beam between them; nothing breaks it                         |

An obstacle comes in from the right with its wave and scrolls to the left at `STAGE_SPEED` (40 pixels a second), as the ceiling and the floor are drawn. It stops every missile, and a ship that touches it loses a life. It does not hold the next wave: the pause of the next wave starts once the Bydos are gone, while the obstacles scroll on.

```text
# A zapper down from the ceiling, a wall up from the floor: fly through the middle, or shoot a way through the wall
wave 2
zapper 0 0.4
wall 0.6 1
```

## The power-ups

The last Bydo of a group marked `drop` carries a power-up, chosen at random. A player who shoots it gets its points and the power-up appears where it was, drifting to the left at `POWER_UP_SPEED` (40 pixels a second); a carrier that crashes into a ship leaves nothing. The first ship that touches the power-up takes it, and the game writes what it took over the ship.

| Power-up     | Sprite          | Gives                                                                   |
|--------------|-----------------|-------------------------------------------------------------------------|
| Extra life   | `extra_life`    | One more life, up to `MAX_LIVES` (5)                                    |
| Shield       | `shield`        | Takes the next hit instead of the ship, drawn as a bubble around it      |
| Spread shot  | `weapon_spread` | Three pellets in a fan at each shot, until the ship is destroyed         |

A ship destroyed comes back with the plasma, its weapon at the start. The HUD writes the weapon of each player after its lives.

## The boss

The Incubator comes in from the right, stops with its right side on the right edge, then rises and falls around the middle of the screen. Its armored body stops the missiles and hurts the ships that touch it; only its eye takes damage: `BOSS_HEALTH` (60) points, and `BOSS_HEALTH_PER_PLAYER` (20) more for each player after the first. It fires an orb at the nearest player every 1.5 seconds and a fan of 5 spits every 4 seconds, every 1 and 3 seconds once its eye has lost half of its health. The player who destroys the eye earns `BOSS_POINTS` (5000), and the game is won: the players see VICTORY, and the next game starts after `GAME_OVER_DELAY`.

## How the waves are played

- The waves wait for the first player: the pause of the first wave starts when a player joins an empty server.
- The groups of a wave start together.
- The pause of the next wave starts only once every Bydo of the wave is destroyed or gone off the left edge, so the waves never pile up.
- Each player after the first adds a quarter of the Bydos of each group, rounded down: a group of 8 brings 10 Bydos with two players and 14 with four.
- After the last wave, the waves start again from the first one, their delays shorter by `WAVE_SPEEDUP` each time, down to `MIN_DELAY_FACTOR`. A wave with a boss ends the game instead, once the boss is destroyed: put it last.

## Example

```text
# A wall of four Bloblings at the bottom, while four Jellies wave at the top
wave 2
jelly sine_wave 4 1.0 line 0.3
blobling straight 4 0 column 0.75
```

Bring one new type or idea per wave first, so a player learns what it does before the waves mix them. The test `common_waves::game_file_is_valid` reads the file of the game: run the tests after a change.
