-- Fails unless the error of an ecs function gives the file and the line of the call
local Script = {}

function Script.start(entity)
    local ok, message = pcall(function() return ecs.get(entity, "Unknown") end)

    if ok or not string.find(message, "locates_its_errors.lua:5: ", 1, true) then
        error("the error has no position: " .. tostring(message))
    end
end

return Script
