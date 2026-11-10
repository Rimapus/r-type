-- Fails if a collision is given twice
local Script = { collisions = 0 }

function Script.onCollision(entity, other)
    Script.collisions = Script.collisions + 1
    if Script.collisions > 1 then
        error("collided twice")
    end
end

return Script
