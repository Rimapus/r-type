-- Binds the jump to a key, a button and an axis, then checks what the game says is bound to it
return {
    start = function(entity)
        input.bindKey("jump", "Space")
        input.bindButton("jump", 0)
        input.bindAxis("jump", "Y", -1)
        local names = input.getBindings("jump")
        if #names ~= 1 or names[1] ~= "Space" then
            error("wrong bindings")
        end
        input.unbind("dash")
    end,
}
