-- Fails unless a value that is not an id is not alive
local Script = {}

function Script.start(entity)
    if ecs.isAlive(nil) or ecs.isAlive(-1) or ecs.isAlive(1.5) or ecs.isAlive(2 ^ 40) or not ecs.isAlive(entity) then
        error("a wrong id is alive")
    end
end

return Script
