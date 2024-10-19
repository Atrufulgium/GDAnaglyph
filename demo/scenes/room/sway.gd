extends Node3D

@export
var max_angle : float = 20
@export
var period_seconds : float = 2.19

# Just hardcode the offset, I don't care
var time : float = 1.5

func _process(delta: float) -> void:
	time += delta
	var angle = sin(TAU * time / period_seconds)
	angle *= deg_to_rad(max_angle)
	rotation.z = angle
