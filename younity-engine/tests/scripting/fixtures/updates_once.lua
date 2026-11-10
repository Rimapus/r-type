-- Fails on its second update: a second step, or a state shared with another entity
local Script = { updates = 0 }

function Script.update(entity, dt)
    Script.updates = Script.updates + 1
    if Script.updates > 1 then
        error("updated twice")
    end
end

return Script
