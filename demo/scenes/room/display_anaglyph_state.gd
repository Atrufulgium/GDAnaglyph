extends RichTextLabel

@export_multiline
var anaglyph_on_bb = ""
@export_multiline
var anaglyph_off_bb = ""

var previous_state : bool = true

func _ready() -> void:
	text = anaglyph_on_bb

func _process(_delta: float) -> void:
	var current_state = AudioStreamPlayerAnaglyph.get_anaglyph_enabled()
	if current_state != previous_state:
		if current_state:
			text = anaglyph_on_bb
		else:
			text = anaglyph_off_bb
		previous_state = current_state
