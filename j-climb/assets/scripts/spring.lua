-- A spring: the robot touching it is thrown up and gets its dash back, see player.lua. The spring shows it is pressed
-- for a moment.
local Spring = { pressed = 0 }

local TILE = 8
-- The columns of the spring at rest and pressed in the first row of sheet.png
local FRAME = { rest = 7, pressed = 8 }
local PRESSED_TIME = 0.2

local function showFrame(entity, column)
    ecs.set(entity, "Sprite", { textureRect = { left = column * TILE, top = 0, width = TILE, height = TILE } })
end

function Spring.update(entity, dt)
    if WORLD.used[entity] then
        WORLD.used[entity] = nil
        Spring.pressed = PRESSED_TIME
        showFrame(entity, FRAME.pressed)
    elseif Spring.pressed > 0 then
        Spring.pressed = Spring.pressed - dt
        if Spring.pressed <= 0 then
            showFrame(entity, FRAME.rest)
        end
    end
end

return Spring
