-- Creates an entity moving at (2, 3)
local Script = {}

function Script.start(entity)
    local created = ecs.create()

    ecs.add(created, "Velocity")
    ecs.set(created, "Velocity", { value = { x = 2, y = 3 } })
end

return Script
