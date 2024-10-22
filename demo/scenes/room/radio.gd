extends AudioStreamPlayerAnaglyph

@export var ready_seek = 23.0

func _ready() -> void:
	play(ready_seek)
