# https://github.com/godotengine/godot/issues/72034
# You can't access _ready() when extending AudioStreamPlayerAnaglyph as super().
# I feel like this explains the crash that follows when you try and
# `func _ready()` anyways: nothing is initialized while I assume it is.
# So instead _ready() the parent...
# extends AudioStreamPlayerAnaglyph

extends Node3D

@export var ready_seek = 23.0

func _ready() -> void:
	$AudioStreamPlayerAnaglyph.play(ready_seek)
