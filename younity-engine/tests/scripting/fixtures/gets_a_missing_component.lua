-- Its entity has no Velocity
local Script = {}

function Script.start(entity)
    ecs.get(entity, "Velocity")
end

return Script
