name="程序员格言"
sample="程序员最讨厌的 2 件事: 1. 写注释/文档；2. 别人不写注释/文档"
interval=300

math.randomseed(os.time())

function onUpdate()
    local length = #res
    local index = math.random(length)
	return res[index]
end

function onClick()
    return onUpdate()
end

res = {
    "Apple", "Banana", "Cherry", "Date", "Elderberry"
}