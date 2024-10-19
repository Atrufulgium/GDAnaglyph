extends Node3D

@export
var sounds : Array[AudioStream]
@export
var sounds_per_minute : float
@export
var anaglyph_data : AnaglyphEffectData
@export
var audio_bus : StringName
var round_robin : int
var process : PoissonProcess

func _ready() -> void:
	process = PoissonProcess.new(sounds_per_minute)
	AudioStreamPlayerAnaglyph.set_max_anaglyph_buses(8)
	AudioStreamPlayerAnaglyph.prepare_anaglyph_buses(8)

func _process(delta: float) -> void:
	var events = process.advance_time(delta)
	if len(sounds) == 0:
		return
	for i in events:
		round_robin += 1
		round_robin %= len(sounds)
		AudioStreamPlayerAnaglyph.play_oneshot(
			sounds[round_robin],
			global_position,
			-6,
			anaglyph_data,
			audio_bus
		)
