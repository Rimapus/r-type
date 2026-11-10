local Counter = { count = 0 }

function Counter.add()
    Counter.count = Counter.count + 1
end

return Counter
