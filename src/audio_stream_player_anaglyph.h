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
			// The player to use on the fun path with Anaglyph.
			AudioStreamPlayer* anaglyph_input;
			// The player to use in case Anaglyph fails.
			AudioStreamPlayer3D* fallback;
		};

		// I require a single child that is an AudioStreamPlayer.
		// This method either gets it, or returns nullptr on a malformed tree.
		bool get_players(Players& players) const;

		Ref<AudioStream> audio_stream;
		float volume;
		float pitch_scale;
		int max_polyphony;
		StringName bus;
		AudioServer::PlaybackType playback_type;
		
		float max_anaglyph_range;
		ForceStream forcing;
		Ref<GDAnaglyph> anaglyph_state;

	protected:
		static void _bind_methods();

		void _validate_property(PropertyInfo& p_property) const;

		// Some properties are shared between the two players, and to be set in
		// this node. This copies that data over to the child nodes.
		void copy_shared_properties() const;

	public:
		AudioStreamPlayerAnaglyph();
		~AudioStreamPlayerAnaglyph();

		virtual PackedStringArray _get_configuration_warnings() const override;
		virtual void _enter_tree() override;

		// TODO: Methods for playback
		// TODO: Global anaglyph "off" button

		// Shared properties
		void set_stream(Ref<AudioStream> audio_stream);
		Ref<AudioStream> get_stream() const;

		void set_volume_db(float volume);
		float get_volume_db() const;

		void set_pitch_scale(float pitch_scale);
		float get_pitch_scale() const;

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
	};
}

VARIANT_ENUM_CAST(AudioStreamPlayerAnaglyph::ForceStream);

#endif //GDANAGLYPH_PLAYER