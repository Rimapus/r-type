-- Without input, binding does nothing and nothing is bound
return {
    start = function(entity)
        input.bindKey("jump", "Space")
        input.bindAxis("jump", "Y", -1)
        if #input.getBindings("jump") ~= 0 then
            error("something is bound")
        end
    end,
}
