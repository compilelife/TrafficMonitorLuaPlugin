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
    "Talk is cheap. Show me the code.",
    "程序员最讨厌的 2 件事: 1. 写注释/文档；2. 别人不写注释/文档", 
    "Hello World ！", 
    "代码和人，有一个能跑就行", 
    "如果它看起来像鸭子、游泳像鸭子、叫声像鸭子，那么它可能就是只鸭子",
    "手持两把锟斤拷，口中疾呼烫烫烫。 脚踏千朵屯屯屯，笑看万物锘锘锘。",
    "可能发生的事情就一定会发生",
    "Unix 哲学：一个程序只做一件事，并且做好",
    "勿在浮沙筑高台",
    "对任何人来说，640KB 内存都足够了。 ——比尔·盖茨.",
    "Across the Great Wall, we can reach every corner in the world",
    "过早的优化是万恶之源",
}