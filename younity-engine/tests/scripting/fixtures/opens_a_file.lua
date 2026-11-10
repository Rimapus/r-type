-- Uses the io library, which the scripts do not have
local Script = {}

function Script.start(entity)
    io.open("file.txt")
end

return Script
