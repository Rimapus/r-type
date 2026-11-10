-- The game: the title screen and its menus, then the run. A run builds the rooms of rooms.lua, lets the rain fall,
-- counts the time, the deaths and the gears, and shows them at the beacon. The robot moves by itself, see player.lua.
--
-- The scripts share the global WORLD table:
--   screen                    "title", "controls", "run", "pause" or "end": the robot only moves during "run"
--   room, spawn, spawnCount   the room shown, where the robot starts in it, and how many times it was set
--   solid, kinds              the solid tiles by row * 16 + column, true or the entity of a crumbling block, and
--                             "spike", "gear", "beacon", "spring" or "refill" by entity
--   platforms                 the box and the direction of each moving platform, by entity
--   stoodOn, used             the crumbling blocks the robot stands on, and the springs and batteries it used
--   player, particles         the entity of the robot, and its sparks with the seconds they have left
--   robot                     what the other scripts reach of the robot: getBox() and carry(dx, dy, pushed)
--   exit, finished            set by the robot when it leaves the room by the top, or touches the beacon
--   time, deaths, gears       the score of the run, and the rooms whose gear was picked up in collected
--   shownGears                the gears counted on the screen, so a new one is shown once
local Text = require("scripts.text")
local controls = require("scripts.controls")

local Game = {}

local TILE = 8
local SIZE = 16
local SHEET = "sprites/sheet.png"

-- From the back to the front
local LAYER = { background = -10, tiles = 0, items = 1, player = 2, rain = 5, shade = 9 }

-- The column of each picture in the first row of sheet.png
local FRAME = {
    groundTop = 0,
    ground = 1,
    spikeUp = 2,
    spikeDown = 3,
    gear = 4,
    beacon = 5,
    block = 6,
    spring = 7,
    crumble = 9,
    platform = 11,
    refill = 13,
}
-- The height of the top of a platform the robot stands on
local PLATFORM_HEIGHT = 4

-- The night and the sky colors of tools/sprites.py
local BACKGROUND = { r = 30, g = 34, b = 60 }
local SHADE = { r = 30, g = 34, b = 60, a = 200 }
local SKY = { r = 90, g = 180, b = 238 }
local RAINDROPS = 32
-- The music of the run, and the sounds of the menus and of the beacon
local MUSIC = "music/climb.wav"
local SOUND = { select = "sounds/select.wav", confirm = "sounds/confirm.wav", beacon = "sounds/beacon.wav" }
-- Seconds the name of a room stays on screen
local BANNER_TIME = 1.5
-- Seconds before the results can be left, so a jump held at the beacon does not skip them
local RESULTS_TIME = 1

-- The entries of the menus, chosen with up, down and jump
local MENUS = {
    title = { "START", "CONTROLS", "QUIT" },
    pause = { "RESUME", "RESTART THE ROOM", "BACK TO THE TITLE" },
}
local MENU_SPACING = 10

-- The things the robot touches: their kind for player.lua, their picture, their hitbox in their tile, and their script
local ITEMS = {
    ["^"] = { kind = "spike", frame = FRAME.spikeUp, size = { x = 6, y = 3 }, offset = { x = 1, y = 5 } },
    ["v"] = { kind = "spike", frame = FRAME.spikeDown, size = { x = 6, y = 3 }, offset = { x = 1, y = 0 } },
    ["G"] = { kind = "gear", frame = FRAME.gear, size = { x = 6, y = 6 }, offset = { x = 1, y = 1 }, script = "gear" },
    ["E"] = { kind = "beacon", frame = FRAME.beacon, size = { x = 4, y = 8 }, offset = { x = 2, y = 0 } },
    ["S"] = {
        kind = "spring", frame = FRAME.spring, size = { x = 6, y = 4 }, offset = { x = 1, y = 4 }, script = "spring",
    },
    ["+"] = {
        kind = "refill", frame = FRAME.refill, size = { x = 6, y = 6 }, offset = { x = 1, y = 1 }, script = "refill",
    },
}
-- The direction a platform starts in: "=" goes right first, "|" goes up first
local PLATFORMS = { ["="] = { dx = 1, dy = 0 }, ["|"] = { dx = 0, dy = -1 } }

local function destroyAll(entities)
    for entity in pairs(entities) do
        if ecs.isAlive(entity) then
            ecs.destroy(entity)
        end
    end
end

-- An entity drawn with the picture at the given column and row of the sheet
local function createSprite(x, y, column, row, layer)
    local entity = ecs.create()

    ecs.add(entity, "Transform")
    ecs.set(entity, "Transform", { position = { x = x, y = y } })
    ecs.add(entity, "Sprite")
    ecs.set(entity, "Sprite", {
        texture = SHEET,
        textureRect = { left = column * TILE, top = row * TILE, width = TILE, height = TILE },
        layer = layer,
    })
    return entity
end

-- A plain rectangle over the whole screen
local function createScreen(color, layer)
    local entity = createSprite(0, 0, FRAME.block, 0, layer)

    ecs.set(entity, "Transform", { scale = { x = SIZE, y = SIZE } })
    ecs.set(entity, "Sprite", { color = color })
    return entity
end

-- Tells what is wrong with a room, or nil if it can be built
local function check(room)
    local spawns = 0

    if type(room.name) ~= "string" or type(room.map) ~= "table" or #room.map ~= SIZE then
        return "a room needs a name and 16 lines"
    end
    for row, line in ipairs(room.map) do
        if #line ~= SIZE or line:find("[^#%^vGEPS%%=|+%.]") then
            return "line " .. row .. " needs 16 tiles among # ^ v G E P S % = | + ."
        end
        local _, count = line:gsub("P", "")

        spawns = spawns + count
    end
    if spawns ~= 1 then
        return "a room needs one P"
    end
    return nil
end

local function createRain()
    local drops = {}

    for _ = 1, RAINDROPS do
        local entity = ecs.create()

        ecs.add(entity, "Transform")
        ecs.set(entity, "Transform", { position = { x = math.random(0, 127), y = math.random(0, 127) } })
        ecs.add(entity, "Sprite")
        ecs.set(entity, "Sprite", {
            texture = SHEET,
            textureRect = { left = FRAME.block * TILE, top = 0, width = 1, height = math.random(2, 4) },
            layer = LAYER.rain,
            color = { r = SKY.r, g = SKY.g, b = SKY.b, a = 120 },
        })
        ecs.add(entity, "Velocity")
        ecs.set(entity, "Velocity", { value = { x = -math.random(4, 10), y = math.random(110, 150) } })
        drops[#drops + 1] = entity
    end
    return drops
end

-- The drops leaving the screen come back on the other side
local function moveRain()
    for _, entity in ipairs(WORLD.rain) do
        local position = ecs.get(entity, "Transform").position

        if position.x < -2 or position.y > 129 then
            local x = position.x < -2 and 129 or position.x

            ecs.set(entity, "Transform", { position = { x = x, y = position.y > 129 and -4 or position.y } })
        end
    end
end

local function createPlayer()
    local entity = createSprite(0, 0, 0, 1, LAYER.player)

    ecs.add(entity, "Hitbox")
    ecs.set(entity, "Hitbox", { size = { x = 6, y = 6 }, offset = { x = 1, y = 2 } })
    ecs.add(entity, "Script")
    ecs.set(entity, "Script", { path = "scripts/player.lua" })
    return entity
end

-- Shows a text in the middle of the room for a moment, like its name, but never over the menus: a room built again
-- during the pause, when rooms.lua is saved, stays silent
local function announce(text)
    destroyAll(WORLD.banner)
    WORLD.banner = {}
    WORLD.bannerTime = 0
    if WORLD.screen == "run" then
        WORLD.banner = Text.write(text, 64, 60)
        WORLD.bannerTime = BANNER_TIME
    end
end

local function addScript(entity, name)
    ecs.add(entity, "Script")
    ecs.set(entity, "Script", { path = "scripts/" .. name .. ".lua" })
end

-- A platform is two tiles wide, from its tile to the right
local function createPlatform(x, y, direction)
    local entity = createSprite(x, y, FRAME.platform, 0, LAYER.tiles)

    ecs.set(entity, "Sprite", {
        textureRect = { left = FRAME.platform * TILE, top = 0, width = 2 * TILE, height = TILE },
    })
    WORLD.platforms[entity] = {
        x = x,
        y = y,
        width = 2 * TILE,
        height = PLATFORM_HEIGHT,
        dx = direction.dx,
        dy = direction.dy,
    }
    addScript(entity, "platform")
    return entity
end

-- The tile under a steel tile is a girder, the others are lit on top
local function createTile(map, row, column, char)
    local x = (column - 1) * TILE
    local y = (row - 1) * TILE
    local tile = (row - 1) * SIZE + column - 1

    if char == "#" then
        local above = row > 1 and map[row - 1]:sub(column, column) or "."
        local frame = above == "#" and FRAME.ground or FRAME.groundTop

        WORLD.solid[tile] = true
        return createSprite(x, y, frame, 0, LAYER.tiles)
    end
    if char == "%" then
        local entity = createSprite(x, y, FRAME.crumble, 0, LAYER.tiles)

        WORLD.solid[tile] = entity
        addScript(entity, "crumble")
        return entity
    end
    if PLATFORMS[char] then
        return createPlatform(x, y, PLATFORMS[char])
    end
    if char == "P" then
        WORLD.spawn = { x = x, y = y }
        return nil
    end
    if char == "G" and WORLD.collected[WORLD.room] then
        return nil
    end
    local item = ITEMS[char]

    if item == nil then
        return nil
    end
    local entity = createSprite(x, y, item.frame, 0, LAYER.items)

    ecs.add(entity, "Hitbox")
    ecs.set(entity, "Hitbox", { size = item.size, offset = item.offset })
    WORLD.kinds[entity] = item.kind
    if item.script then
        addScript(entity, item.script)
    end
    return entity
end

local function buildRoom(index)
    local room = ROOMS[index]
    local problem = check(room)

    if problem then
        log.error("room " .. index .. ": " .. problem)
        return
    end
    destroyAll(WORLD.roomEntities)
    WORLD.room = index
    WORLD.roomEntities = {}
    WORLD.solid = {}
    WORLD.kinds = {}
    WORLD.platforms = {}
    WORLD.stoodOn = {}
    WORLD.used = {}
    for row, line in ipairs(room.map) do
        for column = 1, SIZE do
            local entity = createTile(room.map, row, column, line:sub(column, column))

            if entity then
                WORLD.roomEntities[entity] = true
            end
        end
    end
    if not ecs.isAlive(WORLD.player) then
        WORLD.player = createPlayer()
    end
    WORLD.spawnCount = WORLD.spawnCount + 1
    WORLD.exit = false
    announce(room.name)
end

-- Removes the room, the robot and its sparks
local function clearRoom()
    destroyAll(WORLD.roomEntities)
    destroyAll(WORLD.banner)
    destroyAll(WORLD.particles)
    if ecs.isAlive(WORLD.player) then
        ecs.destroy(WORLD.player)
    end
    WORLD.roomEntities = {}
    WORLD.banner = {}
    WORLD.bannerTime = 0
    WORLD.particles = {}
    WORLD.player = nil
    WORLD.robot = nil
    WORLD.solid = {}
    WORLD.kinds = {}
    WORLD.platforms = {}
end

local function countGears()
    local count = 0

    for _, room in ipairs(ROOMS) do
        for _, line in ipairs(room.map) do
            count = count + select(2, line:gsub("G", ""))
        end
    end
    return count
end

local function formatTime(seconds)
    local hundredths = math.floor(seconds * 100)

    return string.format("%02d:%02d.%02d", hundredths // 6000, hundredths // 100 % 60, hundredths % 100)
end

-- Clears the texts of the screen shown, and shows another one. The name of the room goes too: the pause stops its
-- time, so it would stay over the menu
local function openScreen(screen)
    destroyAll(WORLD.texts)
    destroyAll(WORLD.banner)
    WORLD.banner = {}
    WORLD.bannerTime = 0
    WORLD.screen = screen
    WORLD.screenTime = 0
    WORLD.texts = {}
    WORLD.choice = 1
    WORLD.cursor = nil
end

-- Writes a text that stays until the screen changes
local function write(text, x, y, options)
    for entity in pairs(Text.write(text, x, y, options)) do
        WORLD.texts[entity] = true
    end
end

local function placeCursor()
    local entry = MENUS[WORLD.screen][WORLD.choice]
    local x = 64 - #entry * Text.WIDTH // 2 - 2 * Text.WIDTH

    ecs.set(WORLD.cursor, "Transform", {
        position = { x = x, y = WORLD.menuTop + (WORLD.choice - 1) * MENU_SPACING },
    })
end

-- Writes the entries of the menu of the screen from the line top, with a cursor before the chosen one
local function showMenu(top)
    for index, entry in ipairs(MENUS[WORLD.screen]) do
        write(entry, 64, top + (index - 1) * MENU_SPACING)
    end
    WORLD.menuTop = top
    WORLD.cursor = next(Text.write(">", 0, top, { align = "left" }))
    WORLD.texts[WORLD.cursor] = true
    placeCursor()
end

-- Moves the cursor with up and down, and returns the entry chosen with jump, or nil
local function readMenu()
    local entries = MENUS[WORLD.screen]
    local move = (input.isJustPressed("down") and 1 or 0) - (input.isJustPressed("up") and 1 or 0)

    if move ~= 0 then
        WORLD.choice = (WORLD.choice - 1 + move) % #entries + 1
        placeCursor()
        audio.play(SOUND.select, 0.6)
    end
    if input.isJustPressed("jump") then
        audio.play(SOUND.confirm, 0.6)
        return entries[WORLD.choice]
    end
    return nil
end

local function showTitle()
    local best = storage.load("best")

    openScreen("title")
    game.setTimeScale(1)
    audio.stopMusic()
    write("J-CLIMB", 64, 18, { scale = 3 })
    write("SWITCH THE BEACON ON", 64, 44)
    showMenu(64)
    if best and type(best.time) == "number" then
        write("BEST " .. formatTime(best.time), 64, 108)
    end
end

-- Each action with what is bound to it, e.g. "JUMP  C, SPACE, BUTTON 0"
local function showControls()
    openScreen("controls")
    write("CONTROLS", 64, 10, { scale = 2 })
    for index, action in ipairs(controls) do
        local y = 30 + (index - 1) * 10

        write(action.name, 8, y, { align = "left" })
        write(table.concat(input.getBindings(action.name), ", "), 36, y, { align = "left" })
    end
    write("JUMP TO GO BACK", 64, 112)
end

-- The time scale stops the rain and the sparks, and the robot waits for the run screen
local function showPause()
    openScreen("pause")
    game.setTimeScale(0)
    audio.pauseMusic()
    WORLD.texts[createScreen(SHADE, LAYER.shade)] = true
    write("PAUSED", 64, 36, { scale = 2 })
    showMenu(64)
end

-- The music goes on from where the pause stopped it
local function resume()
    openScreen("run")
    game.setTimeScale(1)
    audio.playMusic(MUSIC)
end

local function showEnd()
    local best = storage.load("best")
    local record = not (best and type(best.time) == "number" and best.time <= WORLD.time)

    if record then
        storage.save("best", { time = WORLD.time })
    end
    audio.stopMusic()
    audio.play(SOUND.beacon)
    openScreen("end")
    WORLD.texts[createScreen(SHADE, LAYER.shade)] = true
    write("BEACON ON!", 64, 20, { scale = 2 })
    write("TIME " .. formatTime(WORLD.time), 64, 46)
    write("DEATHS " .. WORLD.deaths, 64, 56)
    write("GEARS " .. WORLD.gears .. "/" .. countGears(), 64, 66)
    if record then
        write("NEW BEST TIME!", 64, 82)
    end
    write("JUMP FOR THE TITLE", 64, 106)
end

local function startRun()
    openScreen("run")
    audio.playMusic(MUSIC)
    WORLD.time = 0
    WORLD.deaths = 0
    WORLD.gears = 0
    WORLD.shownGears = 0
    WORLD.collected = {}
    WORLD.finished = false
    buildRoom(1)
end

local function updateTitle()
    local entry = readMenu()

    if entry == "START" and ROOMS then
        startRun()
    elseif entry == "CONTROLS" then
        showControls()
    elseif entry == "QUIT" then
        game.quit()
    end
end

local function updatePause()
    local entry = input.isJustPressed("pause") and "RESUME" or readMenu()

    if entry == "RESUME" then
        resume()
    elseif entry == "RESTART THE ROOM" then
        -- The robot starts the room again when the count changes, see player.lua
        WORLD.spawnCount = WORLD.spawnCount + 1
        resume()
    elseif entry == "BACK TO THE TITLE" then
        clearRoom()
        showTitle()
    end
end

local function updateRun(dt)
    if WORLD.exit and WORLD.room < #ROOMS then
        buildRoom(WORLD.room + 1)
    end
    if WORLD.gears > WORLD.shownGears then
        WORLD.shownGears = WORLD.gears
        announce("GEARS " .. WORLD.gears .. "/" .. countGears())
    end
    if WORLD.finished then
        showEnd()
    elseif input.isJustPressed("pause") then
        showPause()
    else
        WORLD.time = WORLD.time + dt
    end
end

function Game.start(entity)
    -- Kept when this file is saved again, so the run goes on in the same room
    WORLD = WORLD or {
        room = 1,
        spawn = { x = 0, y = 0 },
        spawnCount = 0,
        solid = {},
        kinds = {},
        platforms = {},
        stoodOn = {},
        used = {},
        roomEntities = {},
        particles = {},
        banner = {},
        bannerTime = 0,
        texts = {},
        time = 0,
        deaths = 0,
        gears = 0,
        shownGears = 0,
        collected = {},
        exit = false,
        finished = false,
    }
    if not ecs.isAlive(WORLD.background) then
        WORLD.background = createScreen(BACKGROUND, LAYER.background)
    end
    if WORLD.rain == nil then
        WORLD.rain = createRain()
    end
    if WORLD.screen == nil then
        showTitle()
    end
end

function Game.update(entity, dt)
    local playing = WORLD.screen == "run" or WORLD.screen == "pause" or WORLD.screen == "end"

    -- ROOMS is a new table each time rooms.lua runs: at the start, and each time it is saved again
    if ROOMS ~= Game.rooms then
        Game.rooms = ROOMS
        if ROOMS and playing then
            buildRoom(math.min(WORLD.room, #ROOMS))
        end
    end
    WORLD.screenTime = WORLD.screenTime + dt
    if WORLD.bannerTime > 0 then
        WORLD.bannerTime = WORLD.bannerTime - dt
        if WORLD.bannerTime <= 0 then
            destroyAll(WORLD.banner)
            WORLD.banner = {}
        end
    end
    moveRain()
    if WORLD.screen == "title" then
        updateTitle()
    elseif WORLD.screen == "controls" then
        if input.isJustPressed("jump") or input.isJustPressed("pause") then
            showTitle()
        end
    elseif WORLD.screen == "pause" then
        updatePause()
    elseif WORLD.screen == "run" and Game.rooms then
        updateRun(dt)
    elseif WORLD.screen == "end" and WORLD.screenTime > RESULTS_TIME and input.isJustPressed("jump") then
        clearRoom()
        showTitle()
    end
end

return Game
