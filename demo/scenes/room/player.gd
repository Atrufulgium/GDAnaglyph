extends CharacterBody3D

@export var speed = 3.0
@export var mouse_sensitivity = 0.002
@export var controller_sensitivity = 0.05
@export var vertical_look_limit_deg = 80

@export var camera_height = 0.52
@export var camera_height_crouching = -0.3
@export var crouch_movement_factor = 0.3
@export var crouch_transition_speed_in = 5
@export var crouch_transition_speed_out = 10

@export var footstep_sounds : Array[AudioStream]
@export var footstep_delay = 0.8
@export var footstep_delay_crouch = 0.4

# Every second, add/remove `crouch_transition_speed` to this variable to determine what height we
# are at. A value of 0 is "entirely not crouching", while a value of 1 is "entirely crouching.
var crouch_transition = 0.0

# Until 0 is reached, don't play a footstep sound (unless the player stops walking).
# There's also two children, one for each foot, so keep track.
var footstep_time = 0.0
var footstep_is_left = true
var footstep_rng = RandomNumberGenerator.new()

var walking = false
var walking_previous_frame = false

func _ready() -> void:
	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

func _input(event: InputEvent) -> void:
	var rot = Vector2(0,0)
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		rot = event.relative
		rot *= mouse_sensitivity
	if event is InputEventJoypadMotion:
		if event.axis == JOY_AXIS_RIGHT_X:
			rot.x = event.axis_value
		elif event.axis == JOY_AXIS_RIGHT_Y:
			rot.y = event.axis_value
		rot *= controller_sensitivity
	
	if rot.length_squared() > 0:
		rotate_y(-rot.x)
		$Camera3D.rotate_x(-rot.y)
		$Camera3D.rotation.x = clamp(
			$Camera3D.rotation.x,
			-deg_to_rad(vertical_look_limit_deg),
			deg_to_rad(vertical_look_limit_deg)
		)
	
	if event.is_action_pressed("pause"):
		if Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
			Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
		else:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	if event.is_action_pressed("click") and Input.mouse_mode == Input.MOUSE_MODE_VISIBLE:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	if event.is_action_pressed("toggle_anaglyph"):
		AudioStreamPlayerAnaglyph.set_anaglyph_enabled(!AudioStreamPlayerAnaglyph.get_anaglyph_enabled())

func _process(delta: float) -> void:
	var crouching = Input.is_action_pressed("crouch")
	if crouching:
		crouch_transition += crouch_transition_speed_in * delta
	else:
		crouch_transition -= crouch_transition_speed_out * delta
	crouch_transition = clamp(crouch_transition, 0, 1)
	$Camera3D.position.y = smoothstep(camera_height, camera_height_crouching, crouch_transition)
	
	var play_walk_sound = false
	if walking:
		footstep_time -= delta
		if footstep_time < 0:
			play_walk_sound = true
	elif walking_previous_frame:
		play_walk_sound = true
	walking_previous_frame = walking
	
	if play_walk_sound:
		if crouching:
			footstep_time = footstep_delay_crouch
		else:
			footstep_time = footstep_delay
		
		var play_node : AudioStreamPlayerAnaglyph
		if footstep_is_left:
			play_node = $LeftFoot
		else:
			play_node = $RightFoot
		footstep_is_left = !footstep_is_left
		
		play_node.stream = footstep_sounds.pick_random()
		play_node.play()

func _physics_process(delta: float) -> void:
	if not is_on_floor():
		velocity += get_gravity() * delta

	var input_dir := Input.get_vector("move_left", "move_right", "move_forward", "move_backward")
	var direction := (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()
	walking = direction.length_squared() > 0
	var move_speed = speed
	if Input.is_action_pressed("crouch"):
		move_speed *= crouch_movement_factor
	if direction:
		velocity.x = direction.x * move_speed
		velocity.z = direction.z * move_speed
	else:
		velocity.x = move_toward(velocity.x, 0, move_speed)
		velocity.z = move_toward(velocity.z, 0, move_speed)

	move_and_slide()
