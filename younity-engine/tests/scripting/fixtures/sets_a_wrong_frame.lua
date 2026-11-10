-- A frame must be a table of four integers
local Script = {}

function Script.start(entity)
    ecs.set(entity, "Sample", { frames = { { left = 0, top = 0, width = "wide", height = 8 } } })
end

return Script
