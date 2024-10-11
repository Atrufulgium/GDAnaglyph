#include "anaglyph_effect.h"
#include "anaglyph_dll_bridge.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

AnaglyphEffectInstance::AnaglyphEffectInstance() { }

AnaglyphEffectInstance::~AnaglyphEffectInstance() { }

void AnaglyphEffectInstance::_bind_methods() { }

void AnaglyphEffectInstance::_process(const void* p_src_frames, AudioFrame* p_dst_frames, int32_t p_frame_count) {
	// (Again, the editor sets it but it does not _necessarily_ exist.)
	if (base->effect_data == nullptr) {
		base->effect_data.instantiate();
	}

	// TODO: Why is this const void* and not const AudioFrame*?
	// Assuming const AudioFrame* for now, and I'll see whether it crashes.
	AnaglyphBridge::Process(&(base->state), (AudioFrame*)p_src_frames, p_dst_frames, (unsigned int)p_frame_count);
}

bool AnaglyphEffectInstance::_process_silence() const {
	// Anaglyph can -- invisible to the host -- have quite some latency.
	// With heavier .sofa files, on my hardware, it can get up to a second.
	// Not processing silence would end the sound [latency] seconds to soon.
	return true;
}

AnaglyphEffect::AnaglyphEffect() {
	// Ensure Anaglyph is loaded if you try to add it as an effect.
	UnityAudioEffectDefinition* defs = AnaglyphBridge::GetEffectData();

	if (defs == nullptr) {
		godot::UtilityFunctions::push_warning("Anaglyph dll did not load correctly. This Audio Effect won't do anything.");
	}

	UnityAudioEffectState st{};
	state = st;
	AnaglyphBridge::Create(&state);
}

AnaglyphEffect::~AnaglyphEffect() {
	AnaglyphBridge::Release(&state);
}

Ref<AudioEffectInstance> AnaglyphEffect::_instantiate() {
	Ref<AnaglyphEffectInstance> ins;
	ins.instantiate();
	ins->base = Ref<AnaglyphEffect>(this);

	return ins;
}

Vector3 AnaglyphEffect::calculate_polar_position(Node3D* audio_source, Node3D* audio_listener) {
	// World-space difference between the two sources
	Vector3 global_delta
		= audio_source->get_global_position()
		- audio_listener->get_global_position();
	
	// Rotate into camera... microphone?-space.
	// "Basis" is the 3x3 rotation/scaling part of the transform.
	// We only want to invert the rotational part, so grab the quaternion
	// separately and apply the inverse ("xform_inv"...?) to our global.
	// The docs note the quaternion must be normalized, but surely it's
	// normalized if the source is a transform?

	// Note that we also need to take into account different handedness.
	// This is effectively a flip in local space.
	// (idk i didn't think too long about this it *sounds*/behaves correctly)
	Quaternion quat = audio_listener->get_global_basis().get_rotation_quaternion();
	Vector3 relative_pos = quat.xform_inv(global_delta);
	relative_pos.z *= -1;

	// Now the usual "from cartesian to polar coords".
	float dist = relative_pos.length();
	if (dist < 0.001) {
		dist = 0.001;
	}
	float azim;
	if (relative_pos.x == 0 && relative_pos.z == 0) {
		azim = 0; // atan2 could get (0,0) which throws
	}
	else {
		azim = atan2f(relative_pos.x, relative_pos.z);
	}
	float elev = asinf(relative_pos.y / dist); // (arg guaranteed in [-1,1])
	const float rad2deg = 57.2957805;
	Vector3 res = Vector3(azim * rad2deg, elev * rad2deg, dist);
	return res;
}

void AnaglyphEffect::set_wet(const float percentage) {
	ensure_effect_data_exists();
	effect_data->set_wet(percentage);
	AnaglyphBridge::SetParamScaled(&state, 18, effect_data->get_wet(), 0, 100);
}
float AnaglyphEffect::get_wet() {
	ensure_effect_data_exists();
	return effect_data->get_wet();
}

void AnaglyphEffect::set_gain(const float value) {
	ensure_effect_data_exists();
	effect_data->set_gain(value);
	AnaglyphBridge::SetParamScaled(&state, 20, effect_data->get_gain(), -40, 15);
}
float AnaglyphEffect::get_gain() {
	ensure_effect_data_exists();
	return effect_data->get_gain();
}

void AnaglyphEffect::set_hrtf_id(const float value) {
	ensure_effect_data_exists();
	effect_data->set_hrtf_id(value);
	AnaglyphBridge::SetParam(&state, 15, effect_data->get_hrtf_id());
}
float AnaglyphEffect::get_hrtf_id() {
	ensure_effect_data_exists();
	return effect_data->get_hrtf_id();
}

void AnaglyphEffect::set_use_custom_circumference(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_use_custom_circumference(value);
	AnaglyphBridge::SetParamBool(&state, 8, effect_data->get_use_custom_circumference());
}
bool AnaglyphEffect::get_use_custom_circumference() {
	ensure_effect_data_exists();
	return effect_data->get_use_custom_circumference();
}

void AnaglyphEffect::set_head_circumference(const float value) {
	ensure_effect_data_exists();
	effect_data->set_head_circumference(value);
	AnaglyphBridge::SetParamScaled(&state, 25, effect_data->get_head_circumference(), 20, 80);
}
float AnaglyphEffect::get_head_circumference() {
	ensure_effect_data_exists();
	return effect_data->get_head_circumference();
}

void AnaglyphEffect::set_responsiveness(const float value) {
	ensure_effect_data_exists();
	effect_data->set_responsiveness(value);
	AnaglyphBridge::SetParam(&state, 32, effect_data->get_responsiveness());
}
float AnaglyphEffect::get_responsiveness() {
	ensure_effect_data_exists();
	return effect_data->get_responsiveness();
}

void AnaglyphEffect::set_bypass_binaural(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_binaural(value);
	AnaglyphBridge::SetParamBool(&state, 4, effect_data->get_bypass_binaural());
}
bool AnaglyphEffect::get_bypass_binaural() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_binaural();
}

void AnaglyphEffect::set_bypass_parallax(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_parallax(value);
	AnaglyphBridge::SetParamBool(&state, 5, effect_data->get_bypass_parallax());
}
bool AnaglyphEffect::get_bypass_parallax() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_parallax();
}

void AnaglyphEffect::set_bypass_shadow(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_shadow(value);
	AnaglyphBridge::SetParamBool(&state, 1, effect_data->get_bypass_shadow());
}
bool AnaglyphEffect::get_bypass_shadow() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_shadow();
}

void AnaglyphEffect::set_bypass_micro_oscillations(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_micro_oscillations(value);
	AnaglyphBridge::SetParamBool(&state, 9, effect_data->get_bypass_micro_oscillations());
}
bool AnaglyphEffect::get_bypass_micro_oscillations() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_micro_oscillations();
}

void AnaglyphEffect::set_min_attenuation(const float value) {
	ensure_effect_data_exists();
	effect_data->set_min_attenuation(value);
	AnaglyphBridge::SetParamScaled(&state, 30, effect_data->get_min_attenuation(), 0.1, 10);
	AnaglyphBridge::SetParamScaled(&state, 31, effect_data->get_max_attenuation(), 0.1, 10);
}
float AnaglyphEffect::get_min_attenuation() {
	ensure_effect_data_exists();
	return effect_data->get_min_attenuation();
}

void AnaglyphEffect::set_max_attenuation(const float value) {
	ensure_effect_data_exists();
	effect_data->set_max_attenuation(value);
	AnaglyphBridge::SetParamScaled(&state, 30, effect_data->get_min_attenuation(), 0.1, 10);
	AnaglyphBridge::SetParamScaled(&state, 31, effect_data->get_max_attenuation(), 0.1, 10);
}
float AnaglyphEffect::get_max_attenuation() {
	ensure_effect_data_exists();
	return effect_data->get_max_attenuation();
}

void AnaglyphEffect::set_attenuation_exponent(const float value) {
	ensure_effect_data_exists();
	effect_data->set_attenuation_exponent(value);
	AnaglyphBridge::SetParamScaled(&state, 19, effect_data->get_attenuation_exponent(), 0, 2);
}
float AnaglyphEffect::get_attenuation_exponent() {
	ensure_effect_data_exists();
	return effect_data->get_attenuation_exponent();
}

void AnaglyphEffect::set_bypass_attenuation(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_attenuation(value);
	AnaglyphBridge::SetParamBool(&state, 3, effect_data->get_bypass_attenuation());
}
bool AnaglyphEffect::get_bypass_attenuation() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_attenuation();
}

void AnaglyphEffect::set_room_id(const float value) {
	ensure_effect_data_exists();
	effect_data->set_room_id(value);
	AnaglyphBridge::SetParam(&state, 16, effect_data->get_room_id());
}
float AnaglyphEffect::get_room_id() {
	ensure_effect_data_exists();
	return effect_data->get_room_id();
}

void AnaglyphEffect::set_reverb_type(const AnaglyphEffectData::AnaglyphReverbType value) {
	ensure_effect_data_exists();
	effect_data->set_reverb_type(value);
	AnaglyphBridge::SetParamScaled(&state, 13, (float)effect_data->get_reverb_type(), 0, 3);
}
AnaglyphEffectData::AnaglyphReverbType AnaglyphEffect::get_reverb_type() {
	ensure_effect_data_exists();
	return effect_data->get_reverb_type();
}

void AnaglyphEffect::set_reverb_gain(const float value) {
	ensure_effect_data_exists();
	effect_data->set_reverb_gain(value);
	AnaglyphBridge::SetParamScaled(&state, 21, effect_data->get_reverb_gain(), -40, 15);
}
float AnaglyphEffect::get_reverb_gain() {
	ensure_effect_data_exists();
	return effect_data->get_reverb_gain();
}

void AnaglyphEffect::set_reverb_EQ(const Vector3 value) {
	ensure_effect_data_exists();
	effect_data->set_reverb_EQ(value);
	Vector3 v = effect_data->get_reverb_EQ();
	AnaglyphBridge::SetParamScaled(&state, 22, v.x, -40, 15);
	AnaglyphBridge::SetParamScaled(&state, 23, v.y, -40, 15);
	AnaglyphBridge::SetParamScaled(&state, 24, v.z, -40, 15);
}
Vector3 AnaglyphEffect::get_reverb_EQ() {
	ensure_effect_data_exists();
	return effect_data->get_reverb_EQ();
}

void AnaglyphEffect::set_bypass_reverb(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_reverb(value);
	AnaglyphBridge::SetParamBool(&state, 6, effect_data->get_bypass_reverb());
}
bool AnaglyphEffect::get_bypass_reverb() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_reverb();
}

void AnaglyphEffect::set_azimuth(const float value) {
	ensure_effect_data_exists();
	effect_data->set_azimuth(value);
	AnaglyphBridge::SetParamScaled(&state, 27, effect_data->get_azimuth(), -180, 180);
}
float AnaglyphEffect::get_azimuth() {
	ensure_effect_data_exists();
	return effect_data->get_azimuth();
}

void AnaglyphEffect::set_elevation(const float value) {
	ensure_effect_data_exists();
	effect_data->set_elevation(value);
	AnaglyphBridge::SetParamScaled(&state, 26, effect_data->get_elevation(), -90, 90);
}
float AnaglyphEffect::get_elevation() {
	ensure_effect_data_exists();
	return effect_data->get_elevation();
}

void AnaglyphEffect::set_distance(const float value) {
	ensure_effect_data_exists();
	effect_data->set_distance(value);
	AnaglyphBridge::SetParamScaled(&state, 28, effect_data->get_distance(), 0.1, 10);
}
float AnaglyphEffect::get_distance() {
	ensure_effect_data_exists();
	return effect_data->get_distance();
}

void AnaglyphEffect::ensure_effect_data_exists() {
	if (effect_data == nullptr) {
		Ref<AnaglyphEffectData> data = Ref<AnaglyphEffectData>{};
		data.instantiate();
		set_effect_data(data);
	}
}

void AnaglyphEffect::set_effect_data(Ref<AnaglyphEffectData> data) {
	if (data == nullptr) {
		UtilityFunctions::push_warning("Tried to `AnaglyphEffect.set_effect_data(null)`. This is not allowed; ignoring the call.");
		return;
	}
	effect_data = data->duplicate();
	// hoo boyoboy time for this list again *again*
	set_wet(get_wet());
	set_gain(get_gain());

	set_hrtf_id(get_hrtf_id());
	set_use_custom_circumference(get_use_custom_circumference());
	set_head_circumference(get_head_circumference());
	set_responsiveness(get_responsiveness());
	set_bypass_binaural(get_bypass_binaural());

	set_bypass_parallax(get_bypass_parallax());
	set_bypass_shadow(get_bypass_shadow());
	set_bypass_micro_oscillations(get_bypass_micro_oscillations());

	set_min_attenuation(get_min_attenuation());
	set_max_attenuation(get_max_attenuation());
	set_attenuation_exponent(get_attenuation_exponent());
	set_bypass_attenuation(get_bypass_attenuation());

	set_room_id(get_room_id());
	set_reverb_type(get_reverb_type());
	set_reverb_gain(get_reverb_gain());
	set_reverb_EQ(get_reverb_EQ());
	set_bypass_reverb(get_bypass_reverb());

	set_azimuth(get_azimuth());
	set_elevation(get_elevation());
	set_distance(get_distance());
}

void AnaglyphEffect::_bind_methods() {
	// (See https://docs.godotengine.org/en/latest/classes/class_%40globalscope.html#enum-globalscope-propertyhint
	//  for how the hint string works.)
	REGISTER(FLOAT, wet, AnaglyphEffect, "percentage", PROPERTY_HINT_RANGE, "0,100,0.1,suffix:%");
	REGISTER(FLOAT, gain, AnaglyphEffect, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");

	ADD_GROUP("Binaural Personalisation", "");
	REGISTER(FLOAT, hrtf_id, AnaglyphEffect, "id", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(BOOL, use_custom_circumference, AnaglyphEffect, "value", PROPERTY_HINT_NONE, "");
	REGISTER(FLOAT, head_circumference, AnaglyphEffect, "cm", PROPERTY_HINT_RANGE, "20,80,0.1,suffix:cm");
	REGISTER(FLOAT, responsiveness, AnaglyphEffect, "value", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(BOOL, bypass_binaural, AnaglyphEffect, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Localisation", "");
	REGISTER(BOOL, bypass_parallax, AnaglyphEffect, "bypass", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, bypass_shadow, AnaglyphEffect, "bypass", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, bypass_micro_oscillations, AnaglyphEffect, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Attenuation", "");
	REGISTER(FLOAT, min_attenuation, AnaglyphEffect, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
	REGISTER(FLOAT, max_attenuation, AnaglyphEffect, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
	REGISTER(FLOAT, attenuation_exponent, AnaglyphEffect, "exponent", PROPERTY_HINT_RANGE, "0,2,0.1");
	REGISTER(BOOL, bypass_attenuation, AnaglyphEffect, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Reverb", "");
	REGISTER(FLOAT, room_id, AnaglyphEffect, "id", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(INT, reverb_type, AnaglyphEffect, "type", PROPERTY_HINT_ENUM, "OMNI:0,2D:1,3D 1st:2, 3D 2nd:3");
	REGISTER(FLOAT, reverb_gain, AnaglyphEffect, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");
	REGISTER(VECTOR3, reverb_EQ, AnaglyphEffect, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");
	REGISTER(BOOL, bypass_reverb, AnaglyphEffect, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Position", "");
	REGISTER(FLOAT, azimuth, AnaglyphEffect, "angle", PROPERTY_HINT_RANGE, "-180,180,0.1,or_greater,or_less,degrees");
	REGISTER(FLOAT, elevation, AnaglyphEffect, "angle", PROPERTY_HINT_RANGE, "-90,90,0.1,degrees");
	REGISTER(FLOAT, distance, AnaglyphEffect, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
}