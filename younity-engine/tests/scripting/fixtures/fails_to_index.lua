-- Raises an error if its functions are looked up through the metatable
return setmetatable({}, {
    __index = function()
        error("no such function")
    end,
})
