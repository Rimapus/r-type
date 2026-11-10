-- Reads the default fields of its Sample, then writes a field of each type
local Script = {}

function Script.start(entity)
    local sample = ecs.get(entity, "Sample")

    if sample.area ~= nil or sample.color.a ~= 255 or sample.count ~= 0 or sample.on or sample.name ~= "sample" or
        #sample.frames ~= 0 then
        error("unexpected default fields")
    end
    ecs.set(entity, "Sample", {
        on = true,
        count = sample.count + 4,
        speed = 1.5,
        name = sample.name .. "!",
        color = { r = 1, g = 2, b = 3 },
        area = { left = 1, top = 2, width = 3, height = 4 },
        frames = {
            { left = 0, top = 0, width = 8, height = 8 },
            { left = 8, top = 0, width = 8, height = 8 },
        },
    })
end

return Script
