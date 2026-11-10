-- A gear: floats up and down, and is picked up when the robot touches it
local Gear = { time = 0 }

-- Pixels up and down, and turns per second
local FLOAT_HEIGHT = 1.5
local FLOAT_SPEED = 0.6

function Gear.start(entity)
    Gear.y = ecs.get(entity, "Transform").position.y
end

function Gear.update(entity, dt)
    local position = ecs.get(entity, "Transform").position

    Gear.time = Gear.time + dt
    position.y = Gear.y + math.floor(math.sin(Gear.time * FLOAT_SPEED * 2 * math.pi) * FLOAT_HEIGHT + 0.5)
    ecs.set(entity, "Transform", { position = position })
end

function Gear.onCollision(entity, other)
    if other ~= WORLD.player then
        return
    end
    audio.play("sounds/gear.wav")
    WORLD.gears = WORLD.gears + 1
    WORLD.collected[WORLD.room] = true
    WORLD.roomEntities[entity] = nil
    ecs.destroy(entity)
end

return Gear
