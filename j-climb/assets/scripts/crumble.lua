-- A crumbling block: solid until the robot stands on it, then shakes, breaks, and comes back a few seconds later once
-- the robot is out of its way. Its tile in WORLD.solid holds its entity, so the robot tells it when it stands on it in
-- WORLD.stoodOn, see player.lua.
local Crumble = { state = "whole", time = 0 }

local TILE = 8
local SIZE = 16
-- The columns of the block whole and cracked in the first row of sheet.png
local FRAME = { whole = 9, cracked = 10 }
local SHAKE_TIME = 0.5
local AWAY_TIME = 2.5

local function showFrame(entity, column)
    ecs.set(entity, "Sprite", { textureRect = { left = column * TILE, top = 0, width = TILE, height = TILE } })
end

-- Tells if the box of the robot overlaps the tile of the block
local function isRobotInside()
    if WORLD.robot == nil then
        return false
    end
    local left, top, width, height = WORLD.robot.getBox()

    return left ~= nil and left < Crumble.x + TILE and Crumble.x < left + width and top < Crumble.y + TILE
        and Crumble.y < top + height
end

function Crumble.start(entity)
    local position = ecs.get(entity, "Transform").position

    Crumble.x = position.x
    Crumble.y = position.y
    Crumble.tile = position.y // TILE * SIZE + position.x // TILE
end

function Crumble.update(entity, dt)
    if Crumble.state == "whole" and WORLD.stoodOn[entity] then
        Crumble.state = "shaking"
        Crumble.time = SHAKE_TIME
        showFrame(entity, FRAME.cracked)
        audio.play("sounds/crumble.wav", 0.6)
    elseif Crumble.state == "shaking" then
        Crumble.time = Crumble.time - dt
        local shake = math.floor(Crumble.time * 30) % 2

        ecs.set(entity, "Transform", { position = { x = Crumble.x + shake, y = Crumble.y } })
        if Crumble.time <= 0 then
            Crumble.state = "broken"
            Crumble.time = AWAY_TIME
            WORLD.solid[Crumble.tile] = nil
            ecs.set(entity, "Sprite", { visible = false })
        end
    elseif Crumble.state == "broken" then
        Crumble.time = Crumble.time - dt
        if Crumble.time <= 0 and not isRobotInside() then
            Crumble.state = "whole"
            WORLD.solid[Crumble.tile] = entity
            showFrame(entity, FRAME.whole)
            ecs.set(entity, "Transform", { position = { x = Crumble.x, y = Crumble.y } })
            ecs.set(entity, "Sprite", { visible = true })
        end
    end
    WORLD.stoodOn[entity] = nil
end

return Crumble
