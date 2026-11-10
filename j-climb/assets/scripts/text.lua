-- The texts of the game, written with the characters of font.png. A module: game.lua requires it.
local Text = {}

local FONT = "sprites/font.png"
-- The order of the characters in font.png, see tools/sprites.py
local FONT_ORDER = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:/!.-,+>"
local LAYER = 10

-- The size of a character, spacing included, in pixels
Text.WIDTH = 4
Text.HEIGHT = 6

-- Writes a text in capitals and returns its entities, as keys. It is centered on x, or starts at x with
-- options.align = "left", and is options.scale times larger. A character missing from the font is a space.
function Text.write(text, x, y, options)
    local scale = options and options.scale or 1
    local left = (options and options.align == "left") and x or x - #text * Text.WIDTH * scale // 2
    local entities = {}

    text = text:upper()
    for index = 1, #text do
        local glyph = FONT_ORDER:find(text:sub(index, index), 1, true) or 1
        local entity = ecs.create()

        ecs.add(entity, "Transform")
        ecs.set(entity, "Transform", {
            position = { x = left + (index - 1) * Text.WIDTH * scale, y = y },
            scale = { x = scale, y = scale },
        })
        ecs.add(entity, "Sprite")
        ecs.set(entity, "Sprite", {
            texture = FONT,
            textureRect = { left = (glyph - 1) * Text.WIDTH, top = 0, width = Text.WIDTH, height = Text.HEIGHT },
            layer = LAYER,
        })
        entities[entity] = true
    end
    return entities
end

return Text
