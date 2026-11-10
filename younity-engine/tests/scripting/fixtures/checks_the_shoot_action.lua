-- Fails when the shoot action has just been pressed: the failure proves the script read it
local Script = {}

function Script.update(entity, dt)
    if input.isPressed("shoot") and input.isJustPressed("shoot") and not input.isJustReleased("shoot") then
        error("shoot pressed")
    end
end

return Script
