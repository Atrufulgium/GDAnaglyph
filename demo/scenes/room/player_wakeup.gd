extends "res://scenes/room/player.gd"

# (ik, the animation system exists, but i'm lazy)

@export
var do_silly_animation : bool = true
@export
var wake_up_material : ShaderMaterial

var anim_progress = 0.0

func _ready() -> void:
	if do_silly_animation:
		AudioServer.set_bus_volume_db(0, -100)
		wake_up_material.set_shader_parameter("progress", 1.0)
	else:
		AudioServer.set_bus_volume_db(0, 0);
		wake_up_material.set_shader_parameter("progress", 0.0)
	super()

func _process(delta: float) -> void:
	if !do_silly_animation:
		super(delta)
		return
	
	anim_progress += delta
	
	# Between 1s and 6s: increase volume from unhearable to regular
	AudioServer.set_bus_volume_db(0, lerp(-80, 0, clamp((anim_progress - 1) / 5, 0, 1)**0.5))
	# Between 5s and 7s: open eyes
	wake_up_material.set_shader_parameter("progress", clamp((anim_progress - 5) / 2, 0, 1))
	# Between 7s and 8s: move camera forward (lerp's so ugly lol)
	$Camera3D.rotation_degrees.x = lerp(70, 0, clamp(anim_progress - 7, 0, 1))
	# Between 7.5s and 8.5s: move up
	crouch_transition = 1 - clamp((anim_progress - 7.5) / 1, 0, 1)
	$Camera3D.position.y = smoothstep(camera_height, camera_height_crouching, crouch_transition)
	
	# After 8.5s, relinquish control to base.
	if anim_progress > 8.5:
		do_silly_animation = false

func _input(event: InputEvent) -> void:
	if !do_silly_animation:
		super(event)

func _physics_process(delta: float) -> void:
	if !do_silly_animation:
		super(delta)
