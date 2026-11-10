-- A moving platform: goes along a line until a wall or the side of the room, then back. It carries the robot standing
-- on it and pushes the one in its way, through WORLD.robot; a push into a wall breaks the robot. Its box is in
-- WORLD.platforms, where the robot finds it to stand on it, see player.lua.
local Platform = { remainder = 0 }

local TILE = 8
local SIZE = 16
-- Pixels per second
local SPEED = 30

-- Out of the room is a wall, so a platform turns back at its sides
local function isSolid(column, row)
    if column < 0 or column >= SIZE or row < 0 or row >= SIZE then
        return true
    end
    return WORLD.solid[row * SIZE + column] ~= nil
end

local function isBlocked(box, x, y)
    for row = y // TILE, (y + box.height - 1) // TILE do
        for column = x // TILE, (x + box.width - 1) // TILE do
            if isSolid(column, row) then
                return true
            end
        end
    end
    return false
end

-- Tells if the robot stands on the box, and if it overlaps it
local function findRobot(box)
    local left, top, width, height = WORLD.robot.getBox()

    if left == nil or left >= box.x + box.width or box.x >= left + width then
        return false, false
    end
    return top + height == box.y, top < box.y + box.height and box.y < top + height
end

function Platform.update(entity, dt)
    local box = WORLD.platforms[entity]

    if box == nil or WORLD.robot == nil then
        return
    end
    Platform.remainder = Platform.remainder + SPEED * dt
    while Platform.remainder >= 1 do
        Platform.remainder = Platform.remainder - 1
        if isBlocked(box, box.x + box.dx, box.y + box.dy) then
            box.dx = -box.dx
            box.dy = -box.dy
        else
            local riding = findRobot(box)

            box.x = box.x + box.dx
            box.y = box.y + box.dy
            local _, inside = findRobot(box)

            if riding or inside then
                WORLD.robot.carry(box.dx, box.dy, inside)
            end
        end
    end
    ecs.set(entity, "Transform", { position = { x = box.x, y = box.y } })
end

return Platform
