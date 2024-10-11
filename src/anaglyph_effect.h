#ifndef GDANAGLYPH
#define GDANAGLYPH

#include "AudioPluginInterface.h"
#include "anaglyph_effect_data.h"
#include "register_macro.h"

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

	class AnaglyphEffect;

	class AnaglyphEffectInstance : public AudioEffectInstance {
		GDCLASS(AnaglyphEffectInstance, AudioEffectInstance);
		friend class AnaglyphEffect;

		Ref<AnaglyphEffect> base;

	protected:
		static void _bind_methods();

	public:
		AnaglyphEffectInstance();
		~AnaglyphEffectInstance();

		// (really AudioFrame* p_src_frames)

		// From the Godot docs:
		// Called by the AudioServer to process this effect. When
		// `_process_silence` is not overridden or it returns false,
		// this method is called only when the bus is active.
		void _process(const void* p_src_frames, AudioFrame* p_dst_frames, int32_t p_frame_count) override;
		bool _process_silence() const override;
	};

	class AnaglyphEffect : public AudioEffect {
		GDCLASS(AnaglyphEffect, AudioEffect);
		friend class AnaglyphEffectInstance;
		friend class AnaglyphEffectData;

		UnityAudioEffectState state;
		Ref<AnaglyphEffectData> effect_data;

		void ensure_effect_data_exists();

		// The following methods send the current data to Anaglyph.
		void send_wet();
		void send_gain();
		
		void send_hrtf_id();
		void send_use_custom_circumference();
		void send_head_circumference();
		void send_responsiveness();
		void send_bypass_binaural();
		
		void send_bypass_parallax();
		void send_bypass_shadow();
		void send_bypass_micro_oscillations();
		
		void send_min_attenuation();
		void send_max_attenuation();
		void send_attenuation_exponent();
		void send_bypass_attenuation();
		
		void send_room_id();
		void send_reverb_type();
		void send_reverb_gain();
		void send_reverb_EQ();
		void send_bypass_reverb();
		
		void send_azimuth();
		void send_elevation();
		void send_distance();

	protected:
		static void _bind_methods();

	public:
		AnaglyphEffect();
		~AnaglyphEffect();

		Ref<AudioEffectInstance> _instantiate() override;
		// So, this stupid `duplicate` method is virtual in the engine itself,
		// but not in the cpp bindings? What's that about.
		Ref<Resource> duplicate_including_anaglyph(bool p_subresources = false) const;

		// Returns in the Vector3 the azimuth [x], elevation [y], and distance [z]
		// so that their respective getters/setters can use them.
		static Vector3 calculate_polar_position(Node3D* audio_source, Node3D* audio_listener);

		// Sets all effect data and sends it to Anaglyph.
		// This also causes the future updates of `data` to be sent to Anaglyph
		// as well (until the next `set_effect_data()`.
		void set_effect_data(Ref<AnaglyphEffectData> data);

		// Below are the same properties as in anaglyph_effect_data.h,
		// re-exposed. The difference is that these don't just set the data
		// internally, but also send the data to Anaglyph.

		// ======================
		// === The usual ones ===
		// ======================
		// The wet/dry, as a percentage [0,100].
		void set_wet(const float percentage);
		float get_wet();

		// The gain, as dB [-40,15].
		void set_gain(const float dB);
		float get_gain();

		// =======================
		// === Personalisation ===
		// =======================
		// HRTF ID, unfortunately as of yet a float [0,1].
		void set_hrtf_id(const float id);
		float get_hrtf_id();
		
		// Whether to use custom head circumference
		void set_use_custom_circumference(const bool value);
		bool get_use_custom_circumference();

		// Head circumference, as cm [20, 80].
		void set_head_circumference(const float cm);
		float get_head_circumference();

		// Crossfade responsiveness as magic number [0,1].
		// Low values (e.g. 0.05) ensure responsiveness but may give artifacts.
		// High values (e.g. 0.4) reduce artifacts, but are delayed wrt movement.
		void set_responsiveness(const float amount);
		float get_responsiveness();

		// Whether to bypass the HRIR.
		void set_bypass_binaural(const bool bypass);
		bool get_bypass_binaural();

		// =========================
		// === Localisation help ===
		// =========================
		// Whether to bypass parallax.
		void set_bypass_parallax(const bool bypass);
		bool get_bypass_parallax();

		// Whether to bypass your head blocking audio.
		void set_bypass_shadow(const bool bypass);
		bool get_bypass_shadow();

		// Whether to bypass micro oscillations.
		void set_bypass_micro_oscillations(const bool bypass);
		bool get_bypass_micro_oscillations();

		// ============================
		// === Distance Attenuation ===
		// ============================
		// Minimum distance attenuation in meters [0.1,10].
		void set_min_attenuation(const float meters);
		float get_min_attenuation();

		// Maximum distance attenuation in meters [0.1,10].
		void set_max_attenuation(const float meters);
		float get_max_attenuation();

		// Distance attenuation exponent [0,2].
		void set_attenuation_exponent(const float exponent);
		float get_attenuation_exponent();

		// Whether to bypass distance attenuation.
		void set_bypass_attenuation(const bool bypass);
		bool get_bypass_attenuation();

		// ==============
		// === Reverb ===
		// ==============
		// Room id, unfortunately as of yet a float [0,1].
		void set_room_id(const float id);
		float get_room_id();

		// Reverb type, one of "OMNI" (0), "2D" (1), "3D 1st" (2), "3D 2nd" (3).
		void set_reverb_type(const AnaglyphEffectData::AnaglyphReverbType type);
		AnaglyphEffectData::AnaglyphReverbType get_reverb_type();

		// Reverb gain, as dB [-40,15].
		void set_reverb_gain(const float dB);
		float get_reverb_gain();

		// Reverb EQ, as dB [-40,15] low, mid, high-vector.
		void set_reverb_EQ(const Vector3 dB);
		Vector3 get_reverb_EQ();

		// Whether to bypass reverb.
		void set_bypass_reverb(const bool bypass);
		bool get_bypass_reverb();

		// ================
		// === Position ===
		// ================
		// Azimuth, in degrees[ -180,180]. 0 is "forward", 90 is "right".
		void set_azimuth(const float angle);
		float get_azimuth();

		// Elevation, in degrees[-90,90]. 0 is "on the same plane".
		void set_elevation(const float angle);
		float get_elevation();

		// Distance, in meters [0.1,10].
		void set_distance(const float meters);
		float get_distance();

		// Not exposing/implementing the following:
		// 00 - Bypass                - Unnecessary as Godot also has one.
		// 02 - Bypass ITD            - Not exposed in the VST either.
		// 07 - Reverb Only           - Seems superfluous with bypass hrtf.
		// 10 - Bypass Interpolation  - Disables IR crossfade. Exposed in VST, but seems somewhat to complex a usecase for end-users.
		// 11 - Bypass Doppler        - Not exposed in the VST either.
		// 12 - Bypass Air Absorbance - Not exposed in the VST either.
		// 14 - View ID               - VST camera property, irrelevant.
		// 17 - Channel Mapping       - I don't see the use for anything but stereo to mono.
		// 29 - Zoom				  - VST camera property, irrelevant.
	};
}

#endif //GDANAGLYPH