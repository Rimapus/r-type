-- Fails unless start ran once before the update
local Script = { starts = 0 }

function Script.start(entity)
    Script.starts = Script.starts + 1
end

function Script.update(entity, dt)
    if Script.starts ~= 1 then
        error("start ran " .. Script.starts .. " times")
    end
end

return Script
