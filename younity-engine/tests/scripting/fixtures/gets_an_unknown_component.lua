-- No component is named Unknown
local Script = {}

function Script.start(entity)
    ecs.get(entity, "Unknown")
end

return Script
