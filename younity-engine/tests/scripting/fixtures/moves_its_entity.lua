-- Moves its entity one unit to the right at each step
local Script = {}

function Script.update(entity, dt)
    local transform = ecs.get(entity, "Transform")

    ecs.set(entity, "Transform", { position = { x = transform.position.x + 1, y = transform.position.y } })
end

return Script
