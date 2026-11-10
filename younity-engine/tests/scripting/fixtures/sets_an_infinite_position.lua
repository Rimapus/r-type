-- An infinite position would reach the physics and the drawing
local Script = {}

function Script.start(entity)
    ecs.set(entity, "Transform", { position = { x = math.huge, y = 0 } })
end

return Script
