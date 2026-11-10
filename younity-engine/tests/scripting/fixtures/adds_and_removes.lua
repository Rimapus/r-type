-- Swaps the Transform of its entity for a Velocity
local Script = {}

function Script.start(entity)
    ecs.add(entity, "Velocity")
    ecs.remove(entity, "Transform")
    if not ecs.has(entity, "Velocity") or ecs.has(entity, "Transform") then
        error("the components did not change")
    end
end

return Script
