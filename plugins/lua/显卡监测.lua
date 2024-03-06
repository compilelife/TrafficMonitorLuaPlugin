name="nvidiaœ‘ø®º‡≤‚"
sample="œ‘ø®: 100%"
interval=3

function onUpdate()
	return "œ‘ø®: "..tf.runCmdLine("nvidia-smi.exe --query-gpu=utilization.gpu --format=csv,noheader")
end

function onClick()
end