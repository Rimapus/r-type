-- Fails when onCollision gets its entity and the entity created after it: the failure proves the call
local Script = {}

function Script.onCollision(entity, other)
    if other == entity + 1 then
        error("collided with the next entity")
    end
end

return Script
