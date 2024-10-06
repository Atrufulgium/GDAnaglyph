#include "audio_stream_player_anaglyph.h"
#include "anaglyph_bus_manager.h"
#include "gdanaglyph_bridge.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

bool AudioStreamPlayerAnaglyph::anaglyph_enabled = true;

AudioStreamPlayerAnaglyph::AudioStreamPlayerAnaglyph() {
	runtime_players = Players{};

	// DON'T instantiate Ref<Resource> and let the engine do the work.
	// (See https://github.com/godotengine/godot/blob/1917bc3454e58fc56750b00e04aa25cb94d8d266/core/object/class_db.cpp#L2176 )
	//anaglyph_state.instantiate();
	volume = 0;
	pitch_scale = 1;
	max_polyphony = 1;
	bus = StringName("Master");
	playback_type = AudioServer::PLAYBACK_TYPE_DEFAULT;

	max_anaglyph_range = 10;
	forcing = FORCE_NONE;
}

AudioStreamPlayerAnaglyph::~AudioStreamPlayerAnaglyph() { }

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

PackedStringArray AudioStreamPlayerAnaglyph::_get_configuration_warnings() const {
	PackedStringArray warnings = Node3D::_get_configuration_warnings();

	Players players = Players{};

	if (!get_players(players)) {
		warnings.push_back("An AudioStreamPlayerAnaglyph requires exactly two children:\n- An AudioStreamPlayer for when Anaglyph is enabled; and\n- A fallback AudioStreamPlayer3D when Anaglyph is disabled.");
	}
	return warnings;
}

void AudioStreamPlayerAnaglyph::_enter_tree() {
	Players players = Players{};

	if (!get_players(players) && get_child_count() == 0) {
		Node* scene_root = get_tree()->get_edited_scene_root();

		AudioStreamPlayer* a = memnew(AudioStreamPlayer);
		a->set_name("Anaglyph AudioStream");
		add_child(a, true);
		a->set_owner(scene_root);
		
		AudioStreamPlayer3D* b = memnew(AudioStreamPlayer3D);
		b->set_name("Fallback AudioStream");
		add_child(b, true);
		b->set_owner(scene_root);
	}
}

void AudioStreamPlayerAnaglyph::_ready() {
	// Only run when playing.
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	if (!get_players(runtime_players)) {
		runtime_players = Players{};
		godot::UtilityFunctions::push_warning("Malformed tree structure in AudioStreamPlayerAnaglyph.\nIt has been disabled, please fix it in the editor.\n(It requires two children, an AudioStreamPlayer for Anaglyph, and a fallback AudioStreamPlayer3D.)");
		get_parent()->remove_child(this);
	}
	user_bus = bus;
}

void AudioStreamPlayerAnaglyph::_process(double delta) {
	// Only run when playing.
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	// If the tree is not setup properly, don't do anything.
	Players players = Players{};
	if (!get_players(players)) {
		return;
	}

	// Decide whether to process Anaglyph or the fallback.
	// Switching between Anaglyph and the fallback should be as smooth as
	// possible as it can happen at any time for a variety of reasons.
	// Priority: global override > local override > default behaviour.
	bool use_anaglyph = true;
	// TODO: Take `max_anaglyph_range` into account
	if (forcing == FORCE_ANAGLYPH_ON) {
		use_anaglyph = true;
	}
	else if (forcing == FORCE_ANAGLYPH_OFF) {
		use_anaglyph = false;
	}
	if (!get_anaglyph_enabled()) {
		use_anaglyph = false;
	}

	// To ensure both are synced in playback, we don't remove the node from the
	// tree or anything, we just send the inactive node's audio to a muted bus.
	// TODO: Work with signals ffs
	// TODO: Use a proper lending system instead of just having one anaglyph bus.
	StringName anaglyph_bus = AnaglyphBusManager::get_singleton()->get_anaglyph_bus(user_bus);
	StringName silent_bus = AnaglyphBusManager::get_singleton()->get_silent_bus();
	if (use_anaglyph) {
		// TODO: Add and set GDAnaglyph properties
		runtime_players.anaglyph->set_bus(anaglyph_bus);
		runtime_players.fallback->set_bus(silent_bus);
	}
	else {
		runtime_players.anaglyph->set_bus(silent_bus);
		runtime_players.fallback->set_bus(user_bus);
	}
}

void AudioStreamPlayerAnaglyph::copy_shared_properties() const {
	Players players = Players{};
	// Don't do anything if the setup is incorrect.
	if (!get_players(players)) {
		return;
	}

	players.anaglyph->set_stream(audio_stream);
	players.anaglyph->set_volume_db(volume);
	players.anaglyph->set_pitch_scale(pitch_scale);
	players.anaglyph->set_autoplay(autoplay);
	players.anaglyph->set_bus(bus);
	players.anaglyph->set_playback_type(playback_type);

	players.fallback->set_stream(audio_stream);
	players.fallback->set_volume_db(volume);
	players.fallback->set_pitch_scale(pitch_scale);
	players.fallback->set_autoplay(autoplay);
	players.fallback->set_bus(bus);
	players.fallback->set_playback_type(playback_type);
}

void AudioStreamPlayerAnaglyph::set_stream(Ref<AudioStream> p_audio_stream) {
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
	bus = p_bus;
	copy_shared_properties();
}

StringName AudioStreamPlayerAnaglyph::get_bus() const {
	return bus;
}

void AudioStreamPlayerAnaglyph::set_playback_type(AudioServer::PlaybackType p_playback_type) {
	playback_type = p_playback_type;
	copy_shared_properties();
}

AudioServer::PlaybackType AudioStreamPlayerAnaglyph::get_playback_type() const {
	return playback_type;
}

void AudioStreamPlayerAnaglyph::set_max_anaglyph_range(float meters) {
	max_anaglyph_range = meters;
}

float AudioStreamPlayerAnaglyph::get_max_anaglyph_range() const {
	return max_anaglyph_range;
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

void AudioStreamPlayerAnaglyph::set_anaglyph_state(Ref<GDAnaglyph> p_anaglyph_state) {
	anaglyph_state = p_anaglyph_state;
	copy_shared_properties();
}

Ref<GDAnaglyph> AudioStreamPlayerAnaglyph::get_anaglyph_state() const {
	return anaglyph_state;
}

void AudioStreamPlayerAnaglyph::set_anaglyph_enabled(bool enabled) {
	anaglyph_enabled = enabled;
}

bool AudioStreamPlayerAnaglyph::get_anaglyph_enabled() {
	if (AnaglyphBridge::GetEffectData() == nullptr)
		return false;
	return anaglyph_enabled;
}

void AudioStreamPlayerAnaglyph::_bind_methods() {
	ADD_GROUP("Shared stream settings", "");
	REGISTER(OBJECT, stream, AudioStreamPlayerAnaglyph, "stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream");
	REGISTER(FLOAT, volume_db, AudioStreamPlayerAnaglyph, "volume_db", PROPERTY_HINT_RANGE, "-80,24,suffix:dB");
	REGISTER(FLOAT, pitch_scale, AudioStreamPlayerAnaglyph, "pitch_scale", PROPERTY_HINT_RANGE, "0.01,4,0.01,or_greater");
	REGISTER(BOOL, autoplay, AudioStreamPlayerAnaglyph, "autoplay", PROPERTY_HINT_NONE, "");
	// This one's hint string is filled by validate_property
	REGISTER(STRING_NAME, bus, AudioStreamPlayerAnaglyph, "bus", PROPERTY_HINT_ENUM, "");
	REGISTER(INT, playback_type, AudioStreamPlayerAnaglyph, "playback_type", PROPERTY_HINT_ENUM, "Default,Stream,Sample");

	ADD_GROUP("Anaglyph settings", "");
	REGISTER(FLOAT, max_anaglyph_range, AudioStreamPlayerAnaglyph, "max_anaglyph_range", PROPERTY_HINT_RANGE, "0,10,0.01,suffix:m");
	REGISTER(INT, forcing, AudioStreamPlayerAnaglyph, "forcing", PROPERTY_HINT_ENUM, "None,Anaglyph On,Anaglyph Off");
	REGISTER_USAGE(OBJECT, anaglyph_state, AudioStreamPlayerAnaglyph, "anaglyph_state", PROPERTY_HINT_RESOURCE_TYPE, "GDAnaglyph", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT);

	BIND_ENUM_CONSTANT(FORCE_NONE);
	BIND_ENUM_CONSTANT(FORCE_ANAGLYPH_ON);
	BIND_ENUM_CONSTANT(FORCE_ANAGLYPH_OFF);

	ClassDB::bind_static_method("AudioStreamPlayerAnaglyph", D_METHOD("get_anaglyph_enabled"), AudioStreamPlayerAnaglyph::get_anaglyph_enabled);
	ClassDB::bind_static_method("AudioStreamPlayerAnaglyph", D_METHOD("set_anaglyph_enabled", "anaglyph_enabled"), AudioStreamPlayerAnaglyph::set_anaglyph_enabled);
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
