return {
    start = function(entity)
        if storage.load("nothing") ~= nil then
            error("loaded a missing save")
        end
    end,
}
