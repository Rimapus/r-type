-- The robot: runs, jumps, slides down the walls and jumps off them, and dashes once before touching the ground
-- again. It moves pixel by pixel against the solid tiles of WORLD.solid and the platforms of WORLD.platforms, so it
-- never goes into a wall; the engine tells it when it touches spikes, a spring, a battery or the beacon. Saving this
-- file while the game runs keeps the robot where it is.
local Player = {
    x = 0,
    y = 0,
    -- The part of a pixel moved but not drawn yet
    remainderX = 0,
    remainderY = 0,
    speedX = 0,
    speedY = 0,
    facing = 1,
    grounded = false,
    canDash = true,
    dashTime = 0,
    coyoteTime = 0,
    jumpBuffer = 0,
    runTime = 0,
    dead = false,
    deadTime = 0,
    leaving = false,
    -- Whether the screen was the run at the last step: the jump that chose a menu entry does not jump
    running = false,
}

-- In pixels and seconds
local RUN_SPEED = 60
local GROUND_ACCELERATION = 900
local AIR_ACCELERATION = 600
local GRAVITY = 600
-- Below this vertical speed, a held jump halves the gravity: the top of the jump floats a little
local APEX_SPEED = 20
local MAX_FALL = 140
local WALL_SLIDE_FALL = 45
local JUMP_SPEED = 150
local WALL_JUMP_SPEED = 96
local DASH_SPEED = 180
local DASH_TIME = 0.14
local SPRING_SPEED = 210
-- A jump still works for this long after leaving the ground, and a jump pressed this long before landing happens
local COYOTE_TIME = 0.1
local JUMP_BUFFER = 0.1
local RESPAWN_TIME = 0.5
local PARTICLE_TIME = 0.4
local PARTICLE_SPEED = 60

local TILE = 8
local SIZE = 16
-- The solid part of the 8x8 picture
local BOX = { x = 1, y = 2, width = 6, height = 6 }
local SHEET = "sprites/sheet.png"
-- The rows of sheet.png: the lamp of the antenna is lit while the dash is ready, off once it is used
local ROW = { ready = 1, used = 3 }
local FRAME = { idle = 0, run = 1, jump = 3 }
-- The sounds of the robot, in the assets
local SOUND = {
    jump = "sounds/jump.wav",
    wallJump = "sounds/walljump.wav",
    dash = "sounds/dash.wav",
    broken = "sounds/break.wav",
    spring = "sounds/spring.wav",
    battery = "sounds/battery.wav",
}
-- The sparks of the robot when it breaks: yellow like its lamp, or steel once the dash is used
local SPARKS = { ready = { r = 252, g = 222, b = 104 }, used = { r = 122, g = 137, b = 158 } }

-- Out of the room on the sides is a wall, above and below is air
local function isSolid(column, row)
    if column < 0 or column >= SIZE then
        return true
    end
    if row < 0 or row >= SIZE then
        return false
    end
    return WORLD.solid[row * SIZE + column] ~= nil
end

local function overlaps(left, top, width, height, box)
    return left < box.x + box.width and box.x < left + width and top < box.y + box.height and box.y < top + height
end

-- Tells if the box of the robot would overlap a solid tile or a platform at this position
local function collides(x, y)
    for row = (y + BOX.y) // TILE, (y + BOX.y + BOX.height - 1) // TILE do
        for column = (x + BOX.x) // TILE, (x + BOX.x + BOX.width - 1) // TILE do
            if isSolid(column, row) then
                return true
            end
        end
    end
    for _, platform in pairs(WORLD.platforms) do
        if overlaps(x + BOX.x, y + BOX.y, BOX.width, BOX.height, platform) then
            return true
        end
    end
    return false
end

-- Tells the crumbling blocks under the robot that it stands on them, see crumble.lua
local function standOnBlocks()
    local row = (Player.y + BOX.y + BOX.height) // TILE

    for column = (Player.x + BOX.x) // TILE, (Player.x + BOX.x + BOX.width - 1) // TILE do
        local tile = WORLD.solid[row * SIZE + column]

        if math.type(tile) == "integer" then
            WORLD.stoodOn[tile] = true
        end
    end
end

-- 1 for a wall on the right, -1 on the left, 0 for none, within 2 pixels
local function getWall()
    if collides(Player.x + 2, Player.y) then
        return 1
    end
    if collides(Player.x - 2, Player.y) then
        return -1
    end
    return 0
end

local function moveX(distance)
    Player.remainderX = Player.remainderX + distance
    local pixels = math.floor(Player.remainderX + 0.5)
    local step = pixels > 0 and 1 or -1

    Player.remainderX = Player.remainderX - pixels
    for _ = 1, math.abs(pixels) do
        if collides(Player.x + step, Player.y) then
            Player.speedX = 0
            Player.remainderX = 0
            return
        end
        Player.x = Player.x + step
    end
end

local function moveY(distance)
    Player.remainderY = Player.remainderY + distance
    local pixels = math.floor(Player.remainderY + 0.5)
    local step = pixels > 0 and 1 or -1

    Player.remainderY = Player.remainderY - pixels
    for _ = 1, math.abs(pixels) do
        if collides(Player.x, Player.y + step) then
            Player.speedY = 0
            Player.remainderY = 0
            return
        end
        Player.y = Player.y + step
    end
end

local function approach(value, target, amount)
    if value < target then
        return math.min(value + amount, target)
    end
    return math.max(value - amount, target)
end

local function getAxis(negative, positive)
    return (input.isPressed(positive) and 1 or 0) - (input.isPressed(negative) and 1 or 0)
end

local function burst(color)
    for index = 0, 7 do
        local angle = index * math.pi / 4
        local entity = ecs.create()

        ecs.add(entity, "Transform")
        ecs.set(entity, "Transform", { position = { x = Player.x + 3, y = Player.y + 4 } })
        ecs.add(entity, "Sprite")
        ecs.set(entity, "Sprite", {
            texture = SHEET,
            textureRect = { left = 6 * TILE, top = 0, width = 2, height = 2 },
            layer = 3,
            color = color,
        })
        ecs.add(entity, "Velocity")
        ecs.set(entity, "Velocity", {
            value = { x = math.cos(angle) * PARTICLE_SPEED, y = math.sin(angle) * PARTICLE_SPEED },
        })
        WORLD.particles[entity] = PARTICLE_TIME
    end
end

local function updateParticles(dt)
    for entity, time in pairs(WORLD.particles) do
        if time - dt <= 0 then
            if ecs.isAlive(entity) then
                ecs.destroy(entity)
            end
            WORLD.particles[entity] = nil
        else
            WORLD.particles[entity] = time - dt
        end
    end
end

local function die()
    Player.dead = true
    Player.deadTime = RESPAWN_TIME
    WORLD.deaths = WORLD.deaths + 1
    ecs.set(Player.entity, "Sprite", { visible = false })
    burst(Player.canDash and SPARKS.ready or SPARKS.used)
    audio.play(SOUND.broken)
end

local function respawn()
    Player.x = WORLD.spawn.x
    Player.y = WORLD.spawn.y
    Player.remainderX = 0
    Player.remainderY = 0
    Player.speedX = 0
    Player.speedY = 0
    Player.canDash = true
    Player.dashTime = 0
    -- A jump pressed before the robot broke or left the room does not jump at the start
    Player.jumpBuffer = 0
    Player.coyoteTime = 0
    Player.dead = false
    Player.leaving = false
    Player.spawnCount = WORLD.spawnCount
    -- Moved now, or the engine would find it on the spikes again before the next draw
    ecs.set(Player.entity, "Transform", { position = { x = Player.x, y = Player.y } })
    ecs.set(Player.entity, "Sprite", { visible = true })
end

-- Thrown up by a spring, with the dash back
local function bounce()
    Player.speedY = -SPRING_SPEED
    Player.dashTime = 0
    Player.canDash = true
    Player.jumpBuffer = 0
    Player.coyoteTime = 0
    audio.play(SOUND.spring)
end

local function getBox()
    if Player.dead then
        return nil
    end
    return Player.x + BOX.x, Player.y + BOX.y, BOX.width, BOX.height
end

-- Moved one pixel by a platform it stands on, or pushed by one; a wall in the way of a push breaks it
local function carry(dx, dy, pushed)
    if Player.dead then
        return
    end
    if collides(Player.x + dx, Player.y + dy) then
        if pushed then
            die()
        end
        return
    end
    Player.x = Player.x + dx
    Player.y = Player.y + dy
end

local function startDash(horizontal, vertical)
    if horizontal == 0 and vertical == 0 then
        horizontal = Player.facing
    end
    local length = math.sqrt(horizontal * horizontal + vertical * vertical)

    Player.speedX = DASH_SPEED * horizontal / length
    Player.speedY = DASH_SPEED * vertical / length
    Player.dashTime = DASH_TIME
    Player.canDash = false
    audio.play(SOUND.dash)
end

-- Runs, falls and jumps, when the player is not dashing
local function steer(horizontal, dt)
    local wall = getWall()
    local maxFall = MAX_FALL
    local gravity = GRAVITY

    Player.speedX = approach(Player.speedX, horizontal * RUN_SPEED,
        (Player.grounded and GROUND_ACCELERATION or AIR_ACCELERATION) * dt)
    if math.abs(Player.speedY) < APEX_SPEED and input.isPressed("jump") then
        gravity = gravity / 2
    end
    if not Player.grounded and wall ~= 0 and horizontal == wall then
        maxFall = WALL_SLIDE_FALL
    end
    Player.speedY = approach(Player.speedY, maxFall, gravity * dt)
    if Player.jumpBuffer > 0 then
        if Player.coyoteTime > 0 then
            Player.speedY = -JUMP_SPEED
            Player.jumpBuffer = 0
            Player.coyoteTime = 0
            audio.play(SOUND.jump, 0.8)
        elseif wall ~= 0 then
            Player.speedY = -JUMP_SPEED
            Player.speedX = -wall * WALL_JUMP_SPEED
            Player.jumpBuffer = 0
            audio.play(SOUND.wallJump, 0.8)
        end
    end
    if input.isJustReleased("jump") and Player.speedY < 0 then
        Player.speedY = Player.speedY / 2
    end
end

local function move(dt)
    local horizontal = getAxis("left", "right")
    local vertical = getAxis("up", "down")

    if horizontal ~= 0 then
        Player.facing = horizontal
    end
    Player.grounded = collides(Player.x, Player.y + 1)
    if Player.grounded then
        standOnBlocks()
    end
    Player.coyoteTime = Player.grounded and COYOTE_TIME or Player.coyoteTime - dt
    Player.jumpBuffer = input.isJustPressed("jump") and JUMP_BUFFER or Player.jumpBuffer - dt
    if Player.grounded and Player.dashTime <= 0 then
        Player.canDash = true
    end
    if input.isJustPressed("dash") and Player.canDash then
        startDash(horizontal, vertical)
    end
    if Player.dashTime > 0 then
        -- No gravity during the dash; out of it, the player keeps a run speed and half of the rise
        Player.dashTime = Player.dashTime - dt
        if Player.dashTime <= 0 then
            Player.speedX = math.max(-RUN_SPEED, math.min(Player.speedX, RUN_SPEED))
            Player.speedY = Player.speedY < 0 and Player.speedY / 2 or Player.speedY
        end
    else
        steer(horizontal, dt)
    end
    moveX(Player.speedX * dt)
    moveY(Player.speedY * dt)
    Player.runTime = horizontal ~= 0 and Player.runTime + dt or 0
end

local function draw()
    local row = (Player.canDash and ROW.ready or ROW.used) + (Player.facing < 0 and 1 or 0)
    local frame = FRAME.idle

    if not Player.grounded then
        frame = FRAME.jump
    elseif Player.runTime > 0 then
        frame = FRAME.run + math.floor(Player.runTime * 10) % 2
    end
    ecs.set(Player.entity, "Transform", { position = { x = Player.x, y = Player.y } })
    ecs.set(Player.entity, "Sprite", {
        textureRect = { left = frame * TILE, top = row * TILE, width = TILE, height = TILE },
    })
end

function Player.start(entity)
    local position = ecs.get(entity, "Transform").position

    Player.entity = entity
    Player.x = math.floor(position.x)
    Player.y = math.floor(position.y)
    -- Started again by a save of this file: stays where it is, but starts in its room the first time
    Player.spawnCount = Player.x == 0 and Player.y == 0 and -1 or WORLD.spawnCount
    WORLD.robot = { getBox = getBox, carry = carry }
end

function Player.onCollision(entity, other)
    if Player.dead or WORLD.screen ~= "run" then
        return
    end
    local kind = WORLD.kinds[other]

    if kind == "spike" then
        die()
    elseif kind == "spring" then
        bounce()
        WORLD.used[other] = true
    elseif kind == "refill" and not Player.canDash then
        Player.canDash = true
        WORLD.used[other] = true
        audio.play(SOUND.battery)
    elseif kind == "beacon" then
        WORLD.finished = true
    end
end

function Player.update(entity, dt)
    local wasRunning = Player.running

    Player.running = WORLD.screen == "run"
    updateParticles(dt)
    if Player.spawnCount ~= WORLD.spawnCount then
        respawn()
    end
    -- Paused or at the results: the robot waits where it is
    if not Player.running then
        return
    end
    if Player.dead then
        Player.deadTime = Player.deadTime - dt
        if Player.deadTime <= 0 then
            respawn()
        end
        return
    end
    if wasRunning and not WORLD.finished and not Player.leaving then
        move(dt)
    end
    if Player.y > SIZE * TILE then
        die()
        return
    end
    if Player.y < 0 and WORLD.room < #ROOMS then
        Player.leaving = true
        WORLD.exit = true
    end
    draw()
end

return Player
