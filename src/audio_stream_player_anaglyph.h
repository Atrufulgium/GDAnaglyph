#ifndef GDANAGLYPH_PLAYER
#define GDANAGLYPH_PLAYER

#include "gdanaglyph.h"
#include "register_macro.h"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace godot {
	// An AudioStreamPlayer that pushes its audio through Anaglyph buses.
	class AudioStreamPlayerAnaglyph : public Node3D {
		GDCLASS(AudioStreamPlayerAnaglyph, Node3D);

	public:
		enum ForceStream {
			FORCE_NONE = 0,
			FORCE_ANAGLYPH_ON = 1,
			FORCE_ANAGLYPH_OFF = 2
		};

	private:
		struct Players {
			// The player to use when Anaglyph is enabled.
			AudioStreamPlayer* anaglyph;
			// The player to use in case Anaglyph fails.
			AudioStreamPlayer3D* fallback;
		};

		// I require a single child that is an AudioStreamPlayer.
		// This method either gets it, or returns nullptr on a malformed tree.
		// In the editor, you repeatedly need to do this check to inform the
		// end-user whether everything's correct.
		// In runtime, it ought to be correct, and then the children are stored.
		bool get_players(Players& players) const;
		// The same as `get_players` but additionally returns false if we're
		// running in the editor.
		bool get_players_runtime(Players& players) const;
		Players runtime_players;
		// We overwrite the buses of the children during runtime. The initial
		// bus the user chose during the editor will be stored here.
		StringName user_bus;

		Ref<AudioStream> audio_stream;
		float volume;
		float pitch_scale;
		bool autoplay;
		int max_polyphony;
		StringName bus;
		AudioServer::PlaybackType playback_type;
		
		float max_anaglyph_range;
		ForceStream forcing;
		Ref<GDAnaglyph> anaglyph_state;

		static bool anaglyph_enabled;

		// Some properties are shared between the two players, and to be set in
		// this node. This copies that data over to the child nodes.
		void copy_shared_properties() const;
		// Tries to find what Camera/AudioListener3D should be listening to this.
		Node3D* get_listener_node() const;

	protected:
		static void _bind_methods();

		void _validate_property(PropertyInfo& p_property) const;

	public:
		AudioStreamPlayerAnaglyph();
		~AudioStreamPlayerAnaglyph();

		PackedStringArray _get_configuration_warnings() const override;
		void _enter_tree() override;
		void _ready() override;
		void _process(double delta) override;

		// These have a bunch of special behaviour, so delegate them to the
		// child players without keeping track locally.
		// These methods will also be responsible for ensuring proper buses.
		// Just the `finish` signal isn't enough, as that only happens on a
		// full and succesful playback.

		void play(float from_position = 0.0);
		void seek(float to);
		void stop();
		void set_playing(bool playing);
		bool get_playing() const;
		float get_playback_position() const;

		void set_stream_paused(bool paused);
		bool get_stream_paused() const;

		// Shared properties
		void set_stream(Ref<AudioStream> audio_stream);
		Ref<AudioStream> get_stream() const;

		void set_volume_db(float volume);
		float get_volume_db() const;

		void set_pitch_scale(float pitch_scale);
		float get_pitch_scale() const;

		void set_autoplay(bool autoplay);
		bool get_autoplay() const;

		void set_bus(const StringName& bus);
		StringName get_bus() const;

		void set_playback_type(AudioServer::PlaybackType playback_type);
		AudioServer::PlaybackType get_playback_type() const;

		// Anaglyph
		void set_max_anaglyph_range(float meters);
		float get_max_anaglyph_range() const;

		void set_forcing(ForceStream forcing);
		ForceStream get_forcing() const;

		void set_anaglyph_state(Ref<GDAnaglyph> anaglyph_state);
		Ref<GDAnaglyph> get_anaglyph_state() const;

		// Only does something if the dll is loaded properly.
		// Otherwise, anaglyph is force-disabled and this automatically
		// returns `false`, at least until enabled again.
		static void set_anaglyph_enabled(bool enabled);
		static bool get_anaglyph_enabled();
	};
}

VARIANT_ENUM_CAST(AudioStreamPlayerAnaglyph::ForceStream);

#endif //GDANAGLYPH_PLAYER