#include "anaglyph_effect.h"
#include "anaglyph_dll_bridge.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

AnaglyphEffectInstance::AnaglyphEffectInstance() { }

AnaglyphEffectInstance::~AnaglyphEffectInstance() { }

void AnaglyphEffectInstance::_bind_methods() { }

void AnaglyphEffectInstance::_process(const void* p_src_frames, AudioFrame* p_dst_frames, int32_t p_frame_count) {
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

Ref<Resource> AnaglyphEffect::duplicate_including_anaglyph(bool p_subresources) const {
	Ref<AnaglyphEffect> res = duplicate(p_subresources);
	if (res->state.samplerate != 0) {
		// If we have a sample-rate, this thing is properly instantiated and
		// needs its anaglyph state changed as well (otherwise there is no
		// point in duplication).
		AnaglyphBridge::Create(&res->state);
	}
	return res;
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
	AnaglyphBridge::SetParamScaled(&state, 18, percentage, 0, 100);
}
float AnaglyphEffect::get_wet() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 18, 0, 100);
}

void AnaglyphEffect::set_gain(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 20, value, -40, 15);
}
float AnaglyphEffect::get_gain() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 20, -40, 15);
}

void AnaglyphEffect::set_hrtf_id(const float value) {
	AnaglyphBridge::SetParam(&state, 15, value);
}
float AnaglyphEffect::get_hrtf_id() {
	return AnaglyphBridge::GetParamDirect(&state, 15);
}

void AnaglyphEffect::set_use_custom_circumference(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 8, value);
}
bool AnaglyphEffect::get_use_custom_circumference() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 8);
}

void AnaglyphEffect::set_head_circumference(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 25, value, 20, 80);
}
float AnaglyphEffect::get_head_circumference() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 25, 20, 80);
}

void AnaglyphEffect::set_responsiveness(const float value) {
	AnaglyphBridge::SetParam(&state, 32, value);
}
float AnaglyphEffect::get_responsiveness() {
	return AnaglyphBridge::GetParamDirect(&state, 32);
}

void AnaglyphEffect::set_bypass_binaural(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 4, value);
}
bool AnaglyphEffect::get_bypass_binaural() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 4);
}

void AnaglyphEffect::set_bypass_parallax(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 5, value);
}
bool AnaglyphEffect::get_bypass_parallax() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 5);
}

void AnaglyphEffect::set_bypass_shadow(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 1, value);
}
bool AnaglyphEffect::get_bypass_shadow() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 1);
}

void AnaglyphEffect::set_bypass_micro_oscillations(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 9, value);
}
bool AnaglyphEffect::get_bypass_micro_oscillations() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 9);
}

void AnaglyphEffect::set_min_attenuation(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 30, value, 0.1, 10);
	float max = AnaglyphBridge::GetParamScaledDirect(&state, 31, 0.1, 10);
	if (max < value) {
		set_max_attenuation(value);
	}
}
float AnaglyphEffect::get_min_attenuation() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 30, 0.1, 10);
}

void AnaglyphEffect::set_max_attenuation(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 31, value, 0.1, 10);
	float min = AnaglyphBridge::GetParamScaledDirect(&state, 30, 0.1, 10);
	if (min > value) {
		set_min_attenuation(value);
	}
}
float AnaglyphEffect::get_max_attenuation() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 31, 0.1, 10);
}

void AnaglyphEffect::set_attenuation_exponent(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 19, value, 0, 2);
}
float AnaglyphEffect::get_attenuation_exponent() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 19, 0, 2);
}

void AnaglyphEffect::set_bypass_attenuation(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 3, value);
}
bool AnaglyphEffect::get_bypass_attenuation() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 3);
}

void AnaglyphEffect::set_room_id(const float value) {
	AnaglyphBridge::SetParam(&state, 16, value);
}
float AnaglyphEffect::get_room_id() {
	return AnaglyphBridge::GetParamDirect(&state, 16);
}

void AnaglyphEffect::set_reverb_type(const int value) {
	float float_value = (float)value;
	AnaglyphBridge::SetParamScaled(&state, 13, value, 0, 3);
}
int AnaglyphEffect::get_reverb_type() {
	return (int)AnaglyphBridge::GetParamScaledDirect(&state, 13, 0, 3);
}

void AnaglyphEffect::set_reverb_gain(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 21, value, -40, 15);
}
float AnaglyphEffect::get_reverb_gain() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 21, -40, 15);
}

void AnaglyphEffect::set_reverb_EQ(const Vector3 value) {
	AnaglyphBridge::SetParamScaled(&state, 22, value.x, -40, 15);
	AnaglyphBridge::SetParamScaled(&state, 23, value.y, -40, 15);
	AnaglyphBridge::SetParamScaled(&state, 24, value.z, -40, 15);
}
Vector3 AnaglyphEffect::get_reverb_EQ() {
	float x = AnaglyphBridge::GetParamScaledDirect(&state, 22, -40, 15);
	float y = AnaglyphBridge::GetParamScaledDirect(&state, 23, -40, 15);
	float z = AnaglyphBridge::GetParamScaledDirect(&state, 24, -40, 15);
	return Vector3(x, y, z);
}

void AnaglyphEffect::set_bypass_reverb(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 6, value);
}
bool AnaglyphEffect::get_bypass_reverb() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 6);
}

void AnaglyphEffect::set_azimuth(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 27, fmodf(value + 180, 360) - 180, -180, 180);
}
float AnaglyphEffect::get_azimuth() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 27, -180, 180);
}

void AnaglyphEffect::set_elevation(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 26, value, -90, 90);
}
float AnaglyphEffect::get_elevation() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 26, -90, 90);
}

void AnaglyphEffect::set_distance(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 28, value, 0.1, 10);
}
float AnaglyphEffect::get_distance() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 28, 0.1, 10);
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