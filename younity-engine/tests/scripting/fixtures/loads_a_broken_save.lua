return {
    start = function(entity)
        if storage.load("broken") ~= nil then
            error("loaded a broken save")
        end
    end,
}
