-- A Transform has no speed
local Script = {}

function Script.start(entity)
    ecs.set(entity, "Transform", { speed = 1 })
end

return Script
