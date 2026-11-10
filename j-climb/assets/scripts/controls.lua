-- The controls: binds the actions the scripts read to the keyboard and the gamepad. A module: game.lua requires it,
-- so the keys are bound once, and saving this file while the game runs binds the new ones in place of the old ones.
-- It returns the actions, in the order of the controls screen.

-- The buttons of an Xbox gamepad, as SFML numbers them
local BUTTON = { a = 0, x = 2, start = 7 }

local ACTIONS = {
    { name = "left", keys = { "Left" }, axis = "X", side = -1 },
    { name = "right", keys = { "Right" }, axis = "X", side = 1 },
    { name = "up", keys = { "Up" }, axis = "Y", side = -1 },
    { name = "down", keys = { "Down" }, axis = "Y", side = 1 },
    { name = "jump", keys = { "C", "Space" }, button = BUTTON.a },
    { name = "dash", keys = { "X", "LShift" }, button = BUTTON.x },
    { name = "pause", keys = { "Escape" }, button = BUTTON.start },
}

for _, action in ipairs(ACTIONS) do
    input.unbind(action.name)
    for _, key in ipairs(action.keys) do
        input.bindKey(action.name, key)
    end
    if action.axis then
        input.bindAxis(action.name, action.axis, action.side)
    end
    if action.button then
        input.bindButton(action.name, action.button)
    end
end

return ACTIONS
