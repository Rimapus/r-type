-- No entity has the id 1000
local Script = {}

function Script.start(entity)
    ecs.get(1000, "Transform")
end

return Script
