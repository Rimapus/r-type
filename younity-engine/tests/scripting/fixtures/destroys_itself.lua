-- Destroys its entity at its first update
local Script = {}

function Script.update(entity, dt)
    ecs.destroy(entity)
    if not ecs.isAlive(entity) then
        error("destroyed before the end of the step")
    end
end

return Script
