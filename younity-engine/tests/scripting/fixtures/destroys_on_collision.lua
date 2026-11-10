-- Destroys its entity at its first collision: it gets no other collision and no update afterwards
local Script = { collisions = 0 }

function Script.onCollision(entity, other)
    Script.collisions = Script.collisions + 1
    if Script.collisions > 1 then
        error("collided after its destruction")
    end
    ecs.destroy(entity)
end

function Script.update(entity, dt)
    if Script.collisions > 0 then
        error("updated after its destruction")
    end
end

return Script
