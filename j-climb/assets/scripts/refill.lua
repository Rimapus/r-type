-- A battery: gives the dash back to the robot that used it, see player.lua, then is away for a few seconds
local Refill = { time = 0, away = 0 }

-- Pixels up and down, and turns per second
local FLOAT_HEIGHT = 1
local FLOAT_SPEED = 0.8
local AWAY_TIME = 2.5

function Refill.start(entity)
    Refill.y = ecs.get(entity, "Transform").position.y
end

function Refill.update(entity, dt)
    if WORLD.used[entity] then
        -- Without its kind, the robot touching it gets nothing
        WORLD.used[entity] = nil
        WORLD.kinds[entity] = nil
        Refill.away = AWAY_TIME
        ecs.set(entity, "Sprite", { visible = false })
    end
    if Refill.away > 0 then
        Refill.away = Refill.away - dt
        if Refill.away <= 0 then
            WORLD.kinds[entity] = "refill"
            ecs.set(entity, "Sprite", { visible = true })
        end
        return
    end
    local position = ecs.get(entity, "Transform").position

    Refill.time = Refill.time + dt
    position.y = Refill.y + math.floor(math.sin(Refill.time * FLOAT_SPEED * 2 * math.pi) * FLOAT_HEIGHT + 0.5)
    ecs.set(entity, "Transform", { position = position })
end

return Refill
