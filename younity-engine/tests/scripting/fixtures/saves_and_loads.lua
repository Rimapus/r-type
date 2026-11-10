-- Saves a table, loads it back and checks every value came back with its type
return {
    start = function(entity)
        local saved = { time = 12.5, deaths = 3, name = "J", rooms = { 1, 2, 3 }, done = true, nested = { a = { b = "c" } } }
        if not storage.save("best", saved) then
            error("not saved")
        end
        local best = storage.load("best")
        if best.time ~= 12.5 or best.deaths ~= 3 or math.type(best.deaths) ~= "integer" or best.name ~= "J" then
            error("wrong values")
        end
        if #best.rooms ~= 3 or best.rooms[3] ~= 3 or best.done ~= true or best.nested.a.b ~= "c" then
            error("wrong tables")
        end
    end,
}
