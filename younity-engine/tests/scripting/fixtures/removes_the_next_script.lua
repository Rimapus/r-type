-- Removes the Script of the entity created after its own, before that script starts
local Script = {}

function Script.start(entity)
    ecs.remove(entity + 1, "Script")
end

return Script
