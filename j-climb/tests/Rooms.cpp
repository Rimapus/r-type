/*
** EPITECH PROJECT, 2026
** j-climb/tests/Rooms
** File description:
** Rooms unit tests
*/

#include <criterion/criterion.h>

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <ranges>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "TestGame.hpp"

using jclimb::tests::TestGame;

namespace {

constexpr std::size_t SIZE = 16;
constexpr std::size_t ROOM_COUNT = 10;
// The steps a new room takes to put the robot at its start, as after START on the title screen
constexpr int SETTLE_STEPS = 3;

// A room of rooms.lua, with the keys that finish it and pick up its gear on the way
struct Route
{
    const char *name;
    const char *keys;
};

// Segments "steps:held:pressed" separated by ';': "14:right:jump" holds right for 14 steps and presses jump at the
// first one. An action held in two segments in a row stays down; a pressed one is released after its first step.
const Route ROUTES[] = {
    {"THE YARD",
     "30:right;14:right,jump;4:;20:left,jump;8:;18:right,jump;6:;14:right;20:right,jump;10:;"
     "20:left,jump;8:;6:right;24:right,jump;8:;10:right;10:left;30:;20:left,jump;8:;6:left;"
     "20:left,jump;8:;30:jump"                                                                       },
    {"SCAFFOLD",
     "30:right;3:left,jump;6:left;16:right:jump;16:left:jump;16:right:jump;20:right;24:right;26:left;"
     "22:left:jump;16:right:jump;16:left:jump;16:right:jump;16:left:jump;10:"                        },
    {"THE GAP",
     "18:right;8:right:jump;24:right:dash;10:;6:right;12:jump:jump;10:up,jump:dash;20:right;10:;"
     "10:left,jump:jump;10:left,up:dash;12:left;10:;30:left;10:;10:left;14:jump:jump;20:up,jump:dash"},
    {"SPRINGS",
     "8:right;50:right;30:;8:left,jump:jump;10:left,up:dash;20:left;10:;14:jump:jump;20:up,jump:dash"},
    {"RUST",
     "14:right;30:right:jump;30:right:jump;30:right:jump;6:right;30:left:jump;12:left;10:left:jump;"
     "20:left:dash;20:left;10:;14:jump:jump;20:up,jump:dash"                                         },
    {"THE LIFT",
     "150:;14:right;80:;10:right:jump;20:right;10:;16:right:jump;80:;8:left:jump;14:left:dash;20:left;"
     "10:;20:jump:jump"                                                                              },
    {"BATTERIES",
     "14:right;4:right:jump;14:right:dash;10:right,up:dash;14:right,up:dash;30:right;14:left;"
     "14:jump:jump;10:left,up:dash;40:right;10:;14:jump:jump;20:up,jump:dash"                        },
    {"THE WORKS",
     "14:right;28:right:jump;4:right;28:right:jump;30:right:jump;12:right;10:;10:left,up:dash;"
     "10:left,up:dash;30:left;10:;26:right;20:;8:left,jump:jump;10:left,up:dash;20:left;10:;"
     "14:jump:jump;20:up,jump:dash"                                                                  },
    {"THE STORM",
     "14:right;12:right,jump:jump;12:jump;6:right;24:right:jump;40:right;20:;3:left,jump;12:left;"
     "16:right:jump;16:left:jump;20:left;4:;14:jump:jump;20:up,jump:dash"                            },
    {"THE BEACON",
     "14:right;20:right:jump;22:;10:right,up:dash;10:right,up:dash;14:right;10:;12:left,jump:jump;"
     "14:left:dash;20:left;10:;14:right;10:right,jump:jump;14:right:dash;60:right"                   },
};

std::vector<std::string> split(std::string_view text, char separator)
{
    std::vector<std::string> parts;

    for (const auto part : std::views::split(text, separator)) {
        parts.emplace_back(part.begin(), part.end());
    }
    return parts;
}

std::vector<std::string> splitActions(const std::string &actions)
{
    std::vector<std::string> names = split(actions, ',');

    std::erase(names, "");
    return names;
}

// Plays keys in the room shown, until the name of the next room shows: the robot left this one
void play(TestGame &game, std::string_view keys, std::string_view next)
{
    std::set<std::string> held;

    for (const std::string &segment : split(keys, ';')) {
        const std::vector<std::string> fields = split(segment + "::", ':');
        const std::vector<std::string> holding = splitActions(fields[1]);
        const std::vector<std::string> pressing = splitActions(fields[2]);

        for (const std::string &action : held) {
            if (!std::ranges::contains(holding, action)) {
                game.release(action);
            }
        }
        for (const std::string &action : holding) {
            if (!held.contains(action)) {
                game.press(action);
            }
        }
        for (const std::string &action : pressing) {
            game.press(action);
        }
        held = {holding.begin(), holding.end()};
        for (int step = 0; step < std::stoi(fields[0]); ++step) {
            game.step(1);
            if (step == 0) {
                for (const std::string &action : pressing) {
                    if (!held.contains(action)) {
                        game.release(action);
                    }
                }
            }
            if (!next.empty() && game.shows(next)) {
                break;
            }
        }
    }
    for (const std::string &action : held) {
        game.release(action);
    }
}

// The maps of rooms.lua, in their order: the 16 lines between the braces of each "map = {"
std::vector<std::vector<std::string>> readMaps(void)
{
    std::ifstream file(JCLIMB_ASSETS_DIR "/scripts/rooms.lua");
    const std::regex line("^\\s*\"([^\"]*)\",\\s*$");
    std::vector<std::vector<std::string>> maps;
    std::string text;
    bool inMap = false;

    while (std::getline(file, text)) {
        std::smatch match;

        if (text.contains("map = {")) {
            maps.emplace_back();
            inMap = true;
        } else if (inMap && std::regex_match(text, match, line)) {
            maps.back().push_back(match[1]);
        } else if (inMap && text.contains("}")) {
            inMap = false;
        }
    }
    return maps;
}

} // namespace

Test(Rooms, every_map_has_sixteen_lines_one_start_one_gear_and_a_way_out)
{
    const std::vector<std::vector<std::string>> maps = readMaps();

    cr_assert_eq(maps.size(), ROOM_COUNT);
    for (std::size_t room = 0; room < maps.size(); ++room) {
        const std::vector<std::string> &map = maps[room];
        std::string tiles;

        cr_assert_eq(map.size(), SIZE, "room %zu has %zu lines", room + 1, map.size());
        for (const std::string &row : map) {
            cr_assert_eq(row.size(), SIZE, "room %zu: '%s' is not 16 tiles", room + 1, row.c_str());
            cr_assert(row.find_first_not_of("#^vGEPS%=|+.") == std::string::npos, "room %zu: '%s'", room + 1,
                      row.c_str());
            tiles += row;
        }
        cr_assert_eq(std::ranges::count(tiles, 'P'), 1, "room %zu needs one start", room + 1);
        cr_assert_eq(std::ranges::count(tiles, 'G'), 1, "room %zu needs one gear", room + 1);
        // The last room ends at the beacon, the others through the top of the screen
        if (room + 1 == maps.size()) {
            cr_assert_eq(std::ranges::count(tiles, 'E'), 1, "the last room needs the beacon");
        } else {
            cr_assert(map.front().contains('.'), "room %zu has no way out at the top", room + 1);
        }
    }
}

Test(Rooms, a_run_finishes_the_ten_rooms_with_every_gear)
{
    TestGame game;

    game.start();
    for (std::size_t room = 0; room < std::size(ROUTES); ++room) {
        const char *next = room + 1 < std::size(ROUTES) ? ROUTES[room + 1].name : "";

        cr_assert(game.shows(ROUTES[room].name) || room == 0, "room %zu is not %s", room + 1, ROUTES[room].name);
        play(game, ROUTES[room].keys, next);
        cr_assert(next[0] == '\0' || game.shows(next), "the robot did not finish %s", ROUTES[room].name);
        game.step(SETTLE_STEPS);
    }

    cr_assert(game.shows("BEACON ON!"));
    cr_assert(game.shows("GEARS 10/10"));
}
