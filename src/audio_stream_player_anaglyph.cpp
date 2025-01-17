#include "audio_stream_player_anaglyph.h"
#include "anaglyph_bus_manager.h"
#include "anaglyph_dll_bridge.h"
#include "helpers.h"

#include <godot_cpp/classes/audio_listener3d.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>

using namespace godot;

bool AudioStreamPlayerAnaglyph::anaglyph_enabled = true;

AudioStreamPlayerAnaglyph::AudioStreamPlayerAnaglyph() {
	runtime_players = Players{};

	// DON'T instantiate Ref<Resource> and let the engine do the work.
	// (See https://github.com/godotengine/godot/blob/1917bc3454e58fc56750b00e04aa25cb94d8d266/core/object/class_db.cpp#L2176 )
	//anaglyph_state.instantiate();
	volume = 0;
	gain_reduction_fallback = 6;
	unit_size = 1;
	pitch_scale = 1;
	autoplay = false;
	max_polyphony = 1;
	bus = StringName("Master");

	max_anaglyph_range = 10;
	forcing = FORCE_NONE;

	dupe_protection = true;
	delete_on_finish = false;

	set_process_internal(true);
}

AudioStreamPlayerAnaglyph::~AudioStreamPlayerAnaglyph() {
	// This shouldn't be needed, but in case someone frees a node that's still
	// playing at the time.
	// Like, who does that?
	// But hey, just in case.
	// (its 2300pm im probably fucking up destructor order hey future self TODO)
	if (!Engine::get_singleton()->is_editor_hint()) {
		return_anaglyph();
	}
}

bool AudioStreamPlayerAnaglyph::get_players(AudioStreamPlayerAnaglyph::Players& players) const {
	if (get_child_count() != 2) {
		return false;
	}
	else {
		players.anaglyph = nullptr;
		players.fallback = nullptr;

		for (int i = 0; i < 2; i++) {
			Node* child = get_child(i);
			AudioStreamPlayer* a = Object::cast_to<AudioStreamPlayer>(child);
			AudioStreamPlayer3D* b = Object::cast_to<AudioStreamPlayer3D>(child);
			if (a != nullptr) {
				players.anaglyph = a;
			}
			else if (b != nullptr) {
				players.fallback = b;
			}
		}
		return players.anaglyph != nullptr
			&& players.fallback != nullptr;
	}
}

bool AudioStreamPlayerAnaglyph::get_players_runtime(AudioStreamPlayerAnaglyph::Players& players) const {
	if (Engine::get_singleton()->is_editor_hint())
		return false;
	return get_players(players);
}

void AudioStreamPlayerAnaglyph::_notification(int what) {
	if (what == NOTIFICATION_READY) {
		ready();
	}
	else if (what == NOTIFICATION_ENTER_TREE) {
		enter_tree();
	}
	else if (what == NOTIFICATION_INTERNAL_PROCESS) {
		process();
	}
}

PackedStringArray AudioStreamPlayerAnaglyph::_get_configuration_warnings() const {
	PackedStringArray warnings = Node3D::_get_configuration_warnings();

	Players players = Players{};

	if (!get_players(players)) {
		warnings.push_back("An AudioStreamPlayerAnaglyph requires exactly two children:\n- An AudioStreamPlayer for when Anaglyph is enabled; and\n- A fallback AudioStreamPlayer3D when Anaglyph is disabled.\nThese are created automatically when you add this node.\nSo usually you're just fine not adding any other children.");
	}
	return warnings;
}

Node3D* AudioStreamPlayerAnaglyph::get_listener_node() const {
	// I would've liked to copy the following code as verbatim as possible.
	// Unfortunately, Viewport::get_audio_listener_3d() is not exposed.
	// https://github.com/godotengine/godot/blob/e7c39efdb15eaaeb133ed8d0ff0ba0891f8ca676/scene/3d/audio_stream_player_3d.cpp#L355
	// So instead just give up and return the camera's position.
	// (I could add a method and ask the user to explicitly register the listener
	//  node, but that feels very awkward.)
	// TOOD: Not much I can do about it here, but what happens when you have
	// multiple viewports/cameras?
	Viewport* vp = get_viewport();
	Camera3D* active_camera = vp->get_camera_3d();
	return active_camera;
}

bool AudioStreamPlayerAnaglyph::try_add_children() {
	Players players = Players{};

	if (!get_players(players) && get_child_count() == 0) {
		AudioStreamPlayer* a = memnew(AudioStreamPlayer);
		a->set_name("Anaglyph AudioStream");
		add_child(a, true);
		
		AudioStreamPlayer3D* b = memnew(AudioStreamPlayer3D);
		b->set_name("Fallback AudioStream");
		add_child(b, true);

		copy_shared_properties();
		return true;
	}
	return false;
}

void AudioStreamPlayerAnaglyph::enter_tree() {
	Players players = Players{};

	if (!get_players(players) && get_child_count() == 0) {
		try_add_children();
	}
	if (get_players(players)) {
		Node* scene_root = get_tree()->get_edited_scene_root();
		players.anaglyph->set_owner(scene_root);
		players.fallback->set_owner(scene_root);
	}
}

void AudioStreamPlayerAnaglyph::ready() {
	// Only run when playing.
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	if (!get_players(runtime_players)) {
		runtime_players = Players{};
		AnaglyphHelpers::print_warning("Malformed tree structure in AudioStreamPlayerAnaglyph.\nIt has been removed from the tree, please fix it in the editor.\n(It requires two children, an AudioStreamPlayer for Anaglyph, and a fallback AudioStreamPlayer3D.)");
		return_anaglyph();
		Node* parent = get_parent();
		if (parent != nullptr || true) {
			parent->remove_child(this);
		}
			return;
	}

	if (anaglyph_data == nullptr) {
		AnaglyphHelpers::print_warning("Your AudioStreamPlayerAnaglyph \"", get_name(), " \"has `anaglyph_data` null. Using default data, but please set it yourself.");
		anaglyph_data = Ref<AnaglyphEffectData>(memnew(AnaglyphEffectData));
	}

	// Duplicate resources create the problem of "only one position gets set, and
	// other users' positions get ignored". To protect users, have a toggle that
	// defaults to true that prevents this by copying the params when actually
	// starting.
	// The downside is that doing this prevents playing around with sound
	// settings while playing. So allow a toggle to disable this protection.
	if (dupe_protection) {
		anaglyph_data = anaglyph_data->duplicate();
	}

	// (The two child players should at all times be synced)
	runtime_players.anaglyph->connect("finished", Callable(this, "_finish_signal_handler_internal_do_not_call"));

	user_bus = bus;
	if (autoplay) {
		borrow_anaglyph();
	}
}

void AudioStreamPlayerAnaglyph::process() {
	// Only run when playing.
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	// If the tree is not setup properly, don't do anything.
	Players players = Players{};
	if (!get_players(players) || audio_stream == nullptr || anaglyph_data == nullptr) {
		return;
	}

	// Only run when playing. (The other definition.)
	if (!get_playing()) {
		return;
	}

	// Get the anaglyph positional parameters
	Node3D* camera = get_listener_node();
	if (camera == nullptr) {
		AnaglyphHelpers::print_warning("This scene has no 3D camera.\nThis AudioStreamPlayerAnaglyph has been removed from the scene.");
		get_parent()->remove_child(this);
		this->queue_free();
		return;
	}
	Vector3 polar = AnaglyphHelpers::calculate_polar_position(this, camera);
	polar.z /= unit_size;

	// Decide whether to process Anaglyph or the fallback.
	// Switching between Anaglyph and the fallback should be as smooth as
	// possible as it can happen at any time for a variety of reasons.
	// Priority: global override > local override > default behaviour.
	bool use_anaglyph = polar.z < max_anaglyph_range;
	if (forcing == FORCE_ANAGLYPH_ON) {
		use_anaglyph = true;
	}
	else if (forcing == FORCE_ANAGLYPH_OFF) {
		use_anaglyph = false;
	}
	if (!get_anaglyph_enabled()) {
		use_anaglyph = false;
	}

	// In *very* rare cases where I *really* hate users, this *might* happen.
	// You'd have to ignore pretty much every warning in the documentation thuohg.
	use_anaglyph &= borrowed_effect != nullptr;

	// To ensure both are synced in playback, we don't remove the node from the
	// tree or anything, we just send the inactive node's audio to a muted bus.
	// TODO: Only update buses if use_anaglyph changes. Dunno how expensive the
	// set_bus call is.
	StringName anaglyph_bus = borrowed_bus;
	StringName silent_bus = AnaglyphBusManager::get_singleton()->get_silent_bus();
	if (use_anaglyph) {
		borrowed_effect->set_azimuth(polar.x);
		borrowed_effect->set_elevation(polar.y);
		borrowed_effect->set_distance(polar.z);
		runtime_players.anaglyph->set_bus(anaglyph_bus);
		runtime_players.fallback->set_bus(silent_bus);
	}
	else {
		runtime_players.anaglyph->set_bus(silent_bus);
		runtime_players.fallback->set_bus(user_bus);
	}
}

void AudioStreamPlayerAnaglyph::copy_shared_properties() {
	Players players = Players{};
	// Don't do anything if the setup is incorrect.
	if (!get_players(players)) {
		return;
	}

	// Setting the stream resets it, so don't do that if we're not *actually*
	// setting the stream.
	if (players.anaglyph->get_stream() != audio_stream
		|| !players.anaglyph->is_playing()) {
		players.anaglyph->set_stream(audio_stream);
	}
	if (players.fallback->get_stream() != audio_stream
		|| !players.fallback->is_playing()) {
		players.fallback->set_stream(audio_stream);
	}

	// I overwrite buses in play mode, so only set the child buses like this if
	// we're not playing.
	if (Engine::get_singleton()->is_editor_hint()) {
		players.anaglyph->set_bus(bus);
		players.fallback->set_bus(bus);
	}
	else {
		user_bus = bus;
	}

	// The easy ones.
	players.anaglyph->set_volume_db(volume);
	players.anaglyph->set_pitch_scale(pitch_scale);
	players.anaglyph->set_autoplay(autoplay);
	// (No anaglyph "set_unit_size"; we directly divide in this _process to
	//  achieve that effect.)

	players.fallback->set_volume_db(volume - gain_reduction_fallback);
	players.fallback->set_pitch_scale(pitch_scale);
	players.fallback->set_autoplay(autoplay);
	players.fallback->set_unit_size(unit_size);
}

void AudioStreamPlayerAnaglyph::set_stream(Ref<AudioStream> p_audio_stream) {
	// "Setting this property stops all currently playing sounds." - gdocs
	// So, this is a return point too.
	// (I checked, even if it's set to the same, it stops.)
	if (!Engine::get_singleton()->is_editor_hint()) {
		return_anaglyph();
	}
	audio_stream = p_audio_stream;
	copy_shared_properties();
}

Ref<AudioStream> AudioStreamPlayerAnaglyph::get_stream() const {
	return audio_stream;
}

void AudioStreamPlayerAnaglyph::set_volume_db(float p_volume) {
	volume = p_volume;
	copy_shared_properties();
}

float AudioStreamPlayerAnaglyph::get_volume_db() const {
	return volume;
}

void AudioStreamPlayerAnaglyph::set_gain_reduction_fallback_db(float reduction) {
	gain_reduction_fallback = reduction;
	copy_shared_properties();
}

float AudioStreamPlayerAnaglyph::get_gain_reduction_fallback_db() const {
	return gain_reduction_fallback;
}

void AudioStreamPlayerAnaglyph::set_unit_size(float meters) {
	unit_size = CLAMP(meters, 0.01, 100);
	copy_shared_properties();
}
float AudioStreamPlayerAnaglyph::get_unit_size() const {
	return unit_size;
}

void AudioStreamPlayerAnaglyph::set_pitch_scale(float p_pitch_scale) {
	pitch_scale = p_pitch_scale;
	copy_shared_properties();
}

float AudioStreamPlayerAnaglyph::get_pitch_scale() const {
	return pitch_scale;
}

void AudioStreamPlayerAnaglyph::set_bus(const StringName& p_bus) {
	// Some interesting shit's happening at
	// https://github.com/godotengine/godot/blob/2e144928793f17ebd70e1475bb7a7f4fd1095484/scene/audio/audio_stream_player.cpp#L128
	// Also, the AudioStreamPlayer's bus gets redirected at runtime to a new
	// bus with the anaglyph effect that in turn sends it to this bus.
	// TOOD: Ensure that this redirection persists even if this property is
	// changed when editting in play mode.
	// NOTE: The above TODO is *not yet* relevant. Godot's audio interface
	// does not interact with playmode. This may update some day.
	bus = p_bus;
	copy_shared_properties();
}

StringName AudioStreamPlayerAnaglyph::get_bus() const {
	return bus;
}

void AudioStreamPlayerAnaglyph::set_max_anaglyph_range(float meters) {
	max_anaglyph_range = meters;
}

float AudioStreamPlayerAnaglyph::get_max_anaglyph_range() const {
	return max_anaglyph_range;
}

void AudioStreamPlayerAnaglyph::play(float from_position) {
	Players players = Players{};
	if (!get_players_runtime(players)) {
		AnaglyphHelpers::print_warning("You cannot play/stop AudioStreamPlayerAnaglyphs in-editor.\nTo test, try the children, or play your game.");
		return;
	}

	borrow_anaglyph();
	players.anaglyph->play(from_position);
	players.fallback->play(from_position);
}

void AudioStreamPlayerAnaglyph::seek(float to) {
	Players players = Players{};
	if (!get_players_runtime(players)) {
		AnaglyphHelpers::print_warning("You cannot seek AudioStreamPlayerAnaglyphs in-editor.\nTo test, try the children, or play your game.");
		return;
	}

	players.anaglyph->seek(to);
	players.fallback->seek(to);
}

void AudioStreamPlayerAnaglyph::stop() {
	Players players = Players{};
	if (!get_players_runtime(players)) {
		AnaglyphHelpers::print_warning("You cannot play/stop AudioStreamPlayerAnaglyphs in-editor.\nTo test, try the children, or play your game.");
		return;
	}

	return_anaglyph();
	players.anaglyph->stop();
	players.fallback->stop();
}

void AudioStreamPlayerAnaglyph::set_playing(bool playing) {
	// See https://github.com/godotengine/godot/blob/db66bd35af704fe0d83ba9348b8c50a48e51b2ba/scene/audio/audio_stream_player_internal.cpp#L289
	// `set_playing(true)` to a playing instance will reset it to the beginning,
	// also in "vanilla" AudioStreamPlayers.
	if (playing) {
		play();
	}
	else {
		stop();
	}
}

bool AudioStreamPlayerAnaglyph::get_playing() const {
	Players players = Players{};
	if (!get_players_runtime(players)) {
		return false;
	}

	// These should be synced unless (1) user error (2) *weirdness*.
	// Not assuming any weird path.
	// Same below for the other "direct" getters.
	return players.anaglyph->is_playing();
}

float AudioStreamPlayerAnaglyph::get_playback_position() const {
	Players players = Players{};
	if (!get_players_runtime(players)) {
		return 0;
	}

	return players.anaglyph->get_playback_position();
}

void AudioStreamPlayerAnaglyph::set_stream_paused(bool paused) {
	Players players = Players{};
	if (!get_players(players)) {
		return;
	}

	players.anaglyph->set_stream_paused(paused);
	players.fallback->set_stream_paused(paused);

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (get_stream_paused()) {
		return_anaglyph();
	}
	else {
		borrow_anaglyph();
	}
}

bool AudioStreamPlayerAnaglyph::get_stream_paused() const {
	Players players = Players{};
	if (!get_players(players)) {
		return true;
	}

	return players.anaglyph->get_stream_paused();
}

void AudioStreamPlayerAnaglyph::set_autoplay(bool p_autoplay) {
	autoplay = p_autoplay;
	copy_shared_properties();
}

bool AudioStreamPlayerAnaglyph::get_autoplay() const {
	return autoplay;
}

void AudioStreamPlayerAnaglyph::set_forcing(ForceStream p_forcing) {
	forcing = p_forcing;
}

AudioStreamPlayerAnaglyph::ForceStream AudioStreamPlayerAnaglyph::get_forcing() const {
	return forcing;
}

void AudioStreamPlayerAnaglyph::set_anaglyph_data(Ref<AnaglyphEffectData> p_anaglyph_data) {
	anaglyph_data = p_anaglyph_data;
}

Ref<AnaglyphEffectData> AudioStreamPlayerAnaglyph::get_anaglyph_data() const {
	return anaglyph_data;
}

void AudioStreamPlayerAnaglyph::set_dupe_protection(const bool protect) {
	dupe_protection = protect;
}

bool AudioStreamPlayerAnaglyph::get_dupe_protection() const {
	return dupe_protection;
}

void AudioStreamPlayerAnaglyph::set_delete_on_finish(const bool del) {
	delete_on_finish = del;
}

bool AudioStreamPlayerAnaglyph::get_delete_on_finish() const {
	return delete_on_finish;
}

void AudioStreamPlayerAnaglyph::set_anaglyph_enabled(bool enabled) {
	anaglyph_enabled = enabled;
}

bool AudioStreamPlayerAnaglyph::get_anaglyph_enabled() {
	if (AnaglyphBridge::GetEffectData() == nullptr)
		return false;
	return anaglyph_enabled;
}

void AudioStreamPlayerAnaglyph::set_max_anaglyph_buses(int count) {
	AnaglyphBusManager::get_singleton()->set_max_anaglyph_buses(count);
}

int AudioStreamPlayerAnaglyph::get_max_anaglyph_buses() {
	return AnaglyphBusManager::get_singleton()->get_max_anaglyph_buses();
}

void AudioStreamPlayerAnaglyph::prepare_anaglyph_buses(int count) {
	AnaglyphBusManager::get_singleton()->prepare_anaglyph_buses(count);
}

void AudioStreamPlayerAnaglyph::play_oneshot(
	Ref<AudioStream> stream,
	Vector3 global_position,
	float volume_db,
	Ref<AnaglyphEffectData> anaglyph_settings,
	StringName bus
) {
	if (Engine::get_singleton()->is_editor_hint()) {
		AnaglyphHelpers::print_warning("Attempted to play Anaglyph oneshot in the editor. This is only supported when playing.");
		return;
	}
	if (stream == nullptr) {
		AnaglyphHelpers::print_error("Could not play_oneshot a sound (provided stream was `null`).");
		return;
	}

	MainLoop* loop = Engine::get_singleton()->get_main_loop();
	SceneTree* tree = (SceneTree*)loop;
	if (tree == nullptr) {
		AnaglyphHelpers::print_error("Could not play_oneshot a sound (could not find the scene tree).");
		return;
	}
	// The hierarchy Window : Viewport : Node is not exposed...
	Node* parent = (Node*)tree->get_root();
	if (parent == nullptr) {
		AnaglyphHelpers::print_error("Could not play_oneshot a sound (could not find the scene root).");
		return;
	}

	AudioStreamPlayerAnaglyph* node = memnew(AudioStreamPlayerAnaglyph);
	if (anaglyph_settings == nullptr) {
		Ref<AnaglyphEffectData> default_data;
		default_data.instantiate();
		node->set_anaglyph_data(default_data);
	}
	else {
		node->set_anaglyph_data(anaglyph_settings->duplicate());
	}
	node->set_dupe_protection(false);
	node->set_delete_on_finish(true);
	parent->add_child(node, true, INTERNAL_MODE_BACK);
	
	node->set_global_position(global_position);
	// These setters only afterwards as the children only get created on
	// _enter_tree()
	node->set_stream(stream);
	node->set_volume_db(volume_db);
	node->set_bus(bus);
	node->play();
}

void AudioStreamPlayerAnaglyph::_bind_methods() {
	ADD_GROUP("Shared stream settings", "");
	REGISTER(OBJECT, stream, AudioStreamPlayerAnaglyph, "stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream");
	REGISTER(FLOAT, volume_db, AudioStreamPlayerAnaglyph, "volume_db", PROPERTY_HINT_RANGE, "-80,24,suffix:dB");
	REGISTER(FLOAT, gain_reduction_fallback_db, AudioStreamPlayerAnaglyph, "fallback_reduction_db", PROPERTY_HINT_RANGE, "0,80,suffix:dB");
	REGISTER(FLOAT, unit_size, AudioStreamPlayerAnaglyph, "meters_per_unit", PROPERTY_HINT_RANGE, "0.01,100,0.01,suffix:m");
	REGISTER(FLOAT, pitch_scale, AudioStreamPlayerAnaglyph, "pitch_scale", PROPERTY_HINT_RANGE, "0.01,4,0.01,or_greater");
	ClassDB::bind_method(D_METHOD("is_playing"), &AudioStreamPlayerAnaglyph::get_playing);
	ClassDB::bind_method(D_METHOD("set_playing", "enable"), &AudioStreamPlayerAnaglyph::set_playing);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playing", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_playing", "is_playing");
	REGISTER(BOOL, stream_paused, AudioStreamPlayerAnaglyph, "pause", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, autoplay, AudioStreamPlayerAnaglyph, "autoplay", PROPERTY_HINT_NONE, "");
	// This one's hint string is filled by validate_property
	REGISTER(STRING_NAME, bus, AudioStreamPlayerAnaglyph, "bus", PROPERTY_HINT_ENUM, "");

	ADD_GROUP("Anaglyph settings", "");
	REGISTER(FLOAT, max_anaglyph_range, AudioStreamPlayerAnaglyph, "max_anaglyph_range", PROPERTY_HINT_RANGE, "0,10,0.01,suffix:m");
	REGISTER(INT, forcing, AudioStreamPlayerAnaglyph, "forcing", PROPERTY_HINT_ENUM, "None,Anaglyph On,Anaglyph Off");
	REGISTER_USAGE(OBJECT, anaglyph_data, AudioStreamPlayerAnaglyph, "anaglyph_data", PROPERTY_HINT_RESOURCE_TYPE, "AnaglyphEffectData", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT);

	ADD_GROUP("Misc settings", "");
	REGISTER(BOOL, dupe_protection, AudioStreamPlayerAnaglyph, "dupe_protection", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, delete_on_finish, AudioStreamPlayerAnaglyph, "delete_on_finish", PROPERTY_HINT_NONE, "");

	BIND_ENUM_CONSTANT(FORCE_NONE);
	BIND_ENUM_CONSTANT(FORCE_ANAGLYPH_ON);
	BIND_ENUM_CONSTANT(FORCE_ANAGLYPH_OFF);

	ClassDB::bind_method(D_METHOD("play", "from_position"), &AudioStreamPlayerAnaglyph::play, DEFVAL(0.0));
	ClassDB::bind_method(D_METHOD("seek", "to_position"), &AudioStreamPlayerAnaglyph::seek);
	ClassDB::bind_method(D_METHOD("stop"), &AudioStreamPlayerAnaglyph::stop);
	ClassDB::bind_method(D_METHOD("get_playback_position"), &AudioStreamPlayerAnaglyph::get_playback_position);

	ClassDB::bind_static_method("AudioStreamPlayerAnaglyph", D_METHOD("get_anaglyph_enabled"), AudioStreamPlayerAnaglyph::get_anaglyph_enabled);
	ClassDB::bind_static_method("AudioStreamPlayerAnaglyph", D_METHOD("set_anaglyph_enabled", "anaglyph_enabled"), AudioStreamPlayerAnaglyph::set_anaglyph_enabled);

	ClassDB::bind_static_method("AudioStreamPlayerAnaglyph", D_METHOD("get_max_anaglyph_buses"), AudioStreamPlayerAnaglyph::get_max_anaglyph_buses);
	ClassDB::bind_static_method("AudioStreamPlayerAnaglyph", D_METHOD("set_max_anaglyph_buses", "count"), AudioStreamPlayerAnaglyph::set_max_anaglyph_buses);

	ClassDB::bind_static_method("AudioStreamPlayerAnaglyph", D_METHOD("prepare_anaglyph_buses", "count"), AudioStreamPlayerAnaglyph::prepare_anaglyph_buses);
	
	ClassDB::bind_static_method(
		"AudioStreamPlayerAnaglyph",
		D_METHOD("play_oneshot", "audio_stream", "global_position", "volume_db", "anaglyph_settings", "bus"),
		&AudioStreamPlayerAnaglyph::play_oneshot,
		DEFVAL(0.0), DEFVAL(nullptr), DEFVAL("Master")
	);

	ADD_SIGNAL(MethodInfo("finished"));
	ClassDB::bind_method(D_METHOD("_finish_signal_handler_internal_do_not_call"), &AudioStreamPlayerAnaglyph::finish_signal);
}

void AudioStreamPlayerAnaglyph::_validate_property(PropertyInfo& p_property) const {
	// this thing complains about ambiguity with literals...
	if (p_property.name == String("bus")) {
		String options;
		for (int i = 0; i < AudioServer::get_singleton()->get_bus_count(); i++) {
			if (i > 0) {
				options += ",";
			}
			options += AudioServer::get_singleton()->get_bus_name(i);
		}
		p_property.hint_string = options;
	}
}

void AudioStreamPlayerAnaglyph::borrow_anaglyph() {
	// In case the user forgets to return
	if (!borrowed_bus.is_empty()) {
		return_anaglyph();
	}
	borrowed_bus = AnaglyphBusManager::get_singleton()->borrow_anaglyph_bus(user_bus, anaglyph_data, borrowed_effect);
}

void AudioStreamPlayerAnaglyph::return_anaglyph() {
	if (borrowed_bus != user_bus && !borrowed_bus.is_empty()) {
		AnaglyphBusManager::get_singleton()->return_anaglyph_bus(borrowed_bus);
	}
	borrowed_bus = "";
	borrowed_effect = Ref<AnaglyphEffect>(nullptr);
}

void AudioStreamPlayerAnaglyph::finish_signal() {
	return_anaglyph();
	emit_signal("finished");
	if (delete_on_finish) {
		this->queue_free();
	}
}