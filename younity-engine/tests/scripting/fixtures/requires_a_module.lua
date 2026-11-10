-- The same module twice is the same table, and a module returning nothing gives true
return {
    start = function(entity)
        local counter = require("modules.counter")
        counter.add()
        if require("modules.counter").count ~= 1 then
            error("loaded twice")
        end
        if require("modules.nothing") ~= true then
            error("a module returning nothing must give true")
        end
    end,
}
