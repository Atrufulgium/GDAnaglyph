#ifndef GDANAGLYPH_EFFECT_DATA
#define GDANAGLYPH_EFFECT_DATA

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

	// I don't like that I need to do this, but I need to capture "anaglyph
	// states are bound to buses, while the effect data can be passed freely
	// from/to where-ever" somehow. Yet I still want AnaglyphEffect to have
	// getters and setters (as only having a method that updates *all* data
	// inside anaglyph feels off).
	// So massive code duplication it is.
	// Fortunately, Anaglyph is stable, so I won't have to update both places
	// often.

	// This represents parameters of an AnaglyphEffect, that may not necessarily
	// be synchronised to an instance inside Anaglyph's dll.
	class AnaglyphEffectData : public Resource {
		GDCLASS(AnaglyphEffectData, Resource);

	public:
		enum AnaglyphReverbType {
			ANAGLYPH_REVERB_OMNI = 0,
			ANAGLYPH_REVERB_2D = 1,
			ANAGLYPH_REVERB_3D_1 = 2,
			ANAGLYPH_REVERB_3D_2 = 3
		};

	private:
		float wet;
		float gain;

		float hrtf_id;
		bool use_custom_circumference;
		float head_circumference;
		float responsiveness;
		bool bypass_binaural;
		
		bool bypass_parallax;
		bool bypass_shadow;
		bool bypass_micro_oscillations;
		
		float min_attenuation;
		float max_attenuation;
		float attenuation_exponent;
		bool bypass_attenuation;
		
		float room_id;
		AnaglyphReverbType reverb_type;
		float reverb_gain;
		Vector3 reverb_EQ;
		bool bypass_reverb;
		
		float azimuth;
		float elevation;
		float distance;

	protected:
		static void _bind_methods();

	public:
		AnaglyphEffectData();
		~AnaglyphEffectData();

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
		void set_reverb_type(const AnaglyphReverbType type);
		AnaglyphReverbType get_reverb_type();

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
	};

}

VARIANT_ENUM_CAST(AnaglyphEffectData::AnaglyphReverbType);

#endif // GDANAGLYPH_EFFECT_DATA