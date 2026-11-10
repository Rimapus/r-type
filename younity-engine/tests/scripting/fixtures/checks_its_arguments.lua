-- Fails when it gets an integer entity and a dt of 0.5: the failure proves update was called with them
local Script = {}

function Script.update(entity, dt)
    if math.type(entity) == "integer" and dt == 0.5 then
        error("called with the entity and dt")
    end
end

return Script
