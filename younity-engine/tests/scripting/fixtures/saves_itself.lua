return {
    start = function(entity)
        local table = {}
        table.again = table
        storage.save("best", table)
    end,
}
