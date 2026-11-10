-- The rotation is right, the scale is not a Vector2f: neither is written
local Script = {}

function Script.start(entity)
    ecs.set(entity, "Transform", { rotation = 90, scale = "big" })
end

return Script
