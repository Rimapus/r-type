-- Fails unless ecs.get gives back the color, the rectangle and the frames written by ecs.set
local Script = {}

function Script.start(entity)
    ecs.set(entity, "Sample", {
        color = { r = 1, g = 2, b = 3, a = 4 },
        area = { left = 1, top = 2, width = 3, height = 4 },
        frames = {
            { left = 0, top = 0, width = 8, height = 8 },
            { left = 8, top = 0, width = 8, height = 8 },
        },
    })

    local sample = ecs.get(entity, "Sample")

    if sample.color.a ~= 4 or sample.area.width ~= 3 or math.type(sample.area.left) ~= "integer" or
        #sample.frames ~= 2 or sample.frames[2].left ~= 8 or math.type(sample.frames[2].height) ~= "integer" then
        error("the fields changed on the way back")
    end
end

return Script
