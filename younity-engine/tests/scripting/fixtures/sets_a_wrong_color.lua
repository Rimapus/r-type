-- A channel goes from 0 to 255
local Script = {}

function Script.start(entity)
    ecs.set(entity, "Sample", { color = { r = 256 } })
end

return Script
