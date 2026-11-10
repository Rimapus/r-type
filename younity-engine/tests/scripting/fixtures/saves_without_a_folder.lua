return {
    start = function(entity)
        if storage.save("best", { time = 1 }) or storage.load("best") ~= nil then
            error("saved without a folder")
        end
    end,
}
