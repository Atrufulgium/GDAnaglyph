#include "gdanaglyph.h"
#include "gdanaglyph_bridge.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

GDAnaglyphInstance::GDAnaglyphInstance() { }

GDAnaglyphInstance::~GDAnaglyphInstance() { }

void GDAnaglyphInstance::_bind_methods() { }

void GDAnaglyphInstance::_process(const void* p_src_frames, AudioFrame* p_dst_frames, int32_t p_frame_count) {
	// TODO: Why is this const void* and not const AudioFrame*?
	// Assuming const AudioFrame* for now, and I'll see whether it crashes.
	AnaglyphBridge::Process(&(base->state), (AudioFrame*)p_src_frames, p_dst_frames, (unsigned int)p_frame_count);
}

bool GDAnaglyphInstance::_process_silence() const {
	// Anaglyph can -- invisible to the host -- have quite some latency.
	// With heavier .sofa files, on my hardware, it can get up to a second.
	// Not processing silence would end the sound [latency] seconds to soon.
	return true;
}

GDAnaglyph::GDAnaglyph() {
	// Ensure Anaglyph is loaded if you try to add it as an effect.
	UnityAudioEffectDefinition* defs = AnaglyphBridge::GetEffectData();

	if (defs == nullptr) {
		godot::UtilityFunctions::push_warning("Anaglyph dll did not load correctly. This Audio Effect won't do anything.");
	}

	UnityAudioEffectState st{};
	state = st;
	AnaglyphBridge::Create(&state);
}

GDAnaglyph::~GDAnaglyph() {
	AnaglyphBridge::Release(&state);
}

Ref<AudioEffectInstance> GDAnaglyph::_instantiate() {
	Ref<GDAnaglyphInstance> ins;
	ins.instantiate();
	ins->base = Ref<GDAnaglyph>(this);

	return ins;
}

void GDAnaglyph::set_wet(const float percentage) {
	AnaglyphBridge::SetParamScaled(&state, 18, percentage, 0, 100);
}
float GDAnaglyph::get_wet() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 18, 0, 100);
}

void GDAnaglyph::set_gain(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 20, value, -40, 15);
}
float GDAnaglyph::get_gain() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 20, -40, 15);
}

void GDAnaglyph::set_hrtf_id(const float value) {
	AnaglyphBridge::SetParam(&state, 15, value);
}
float GDAnaglyph::get_hrtf_id() {
	return AnaglyphBridge::GetParamDirect(&state, 15);
}

void GDAnaglyph::set_use_custom_circumference(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 8, value);
}
bool GDAnaglyph::get_use_custom_circumference() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 8);
}

void GDAnaglyph::set_head_circumference(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 25, value, 20, 80);
}
float GDAnaglyph::get_head_circumference() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 25, 20, 80);
}

void GDAnaglyph::set_responsiveness(const float value) {
	AnaglyphBridge::SetParam(&state, 32, value);
}
float GDAnaglyph::get_responsiveness() {
	return AnaglyphBridge::GetParamDirect(&state, 32);
}

void GDAnaglyph::set_bypass_binaural(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 4, value);
}
bool GDAnaglyph::get_bypass_binaural() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 4);
}

void GDAnaglyph::set_bypass_parallax(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 5, value);
}
bool GDAnaglyph::get_bypass_parallax() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 5);
}

void GDAnaglyph::set_bypass_shadow(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 1, value);
}
bool GDAnaglyph::get_bypass_shadow() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 1);
}

void GDAnaglyph::set_bypass_micro_oscillations(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 9, value);
}
bool GDAnaglyph::get_bypass_micro_oscillations() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 9);
}

void GDAnaglyph::set_min_attenuation(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 30, value, 0.1, 10);
	float max = AnaglyphBridge::GetParamScaledDirect(&state, 31, 0.1, 10);
	if (max < value) {
		set_max_attenuation(value);
	}
}
float GDAnaglyph::get_min_attenuation() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 30, 0.1, 10);
}

void GDAnaglyph::set_max_attenuation(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 31, value, 0.1, 10);
	float min = AnaglyphBridge::GetParamScaledDirect(&state, 30, 0.1, 10);
	if (min > value) {
		set_min_attenuation(value);
	}
}
float GDAnaglyph::get_max_attenuation() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 31, 0.1, 10);
}

void GDAnaglyph::set_attenuation_exponent(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 19, value, 0, 2);
}
float GDAnaglyph::get_attenuation_exponent() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 19, 0, 2);
}

void GDAnaglyph::set_bypass_attenuation(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 3, value);
}
bool GDAnaglyph::get_bypass_attenuation() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 3);
}

void GDAnaglyph::set_room_id(const float value) {
	AnaglyphBridge::SetParam(&state, 16, value);
}
float GDAnaglyph::get_room_id() {
	return AnaglyphBridge::GetParamDirect(&state, 16);
}

void GDAnaglyph::set_reverb_type(const int value) {
	float float_value = (float)value;
	AnaglyphBridge::SetParamScaled(&state, 13, value, 0, 3);
}
int GDAnaglyph::get_reverb_type() {
	return (int)AnaglyphBridge::GetParamScaledDirect(&state, 13, 0, 3);
}

void GDAnaglyph::set_reverb_gain(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 21, value, -40, 15);
}
float GDAnaglyph::get_reverb_gain() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 21, -40, 15);
}

void GDAnaglyph::set_reverb_EQ(const Vector3 value) {
	AnaglyphBridge::SetParamScaled(&state, 22, value.x, -40, 15);
	AnaglyphBridge::SetParamScaled(&state, 23, value.y, -40, 15);
	AnaglyphBridge::SetParamScaled(&state, 24, value.z, -40, 15);
}
Vector3 GDAnaglyph::get_reverb_EQ() {
	float x = AnaglyphBridge::GetParamScaledDirect(&state, 22, -40, 15);
	float y = AnaglyphBridge::GetParamScaledDirect(&state, 23, -40, 15);
	float z = AnaglyphBridge::GetParamScaledDirect(&state, 24, -40, 15);
	return Vector3(x, y, z);
}

void GDAnaglyph::set_bypass_reverb(const bool value) {
	AnaglyphBridge::SetParamBool(&state, 6, value);
}
bool GDAnaglyph::get_bypass_reverb() {
	return AnaglyphBridge::GetParamBoolDirect(&state, 6);
}

void GDAnaglyph::set_elevation(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 26, value, -90, 90);
}
float GDAnaglyph::get_elevation() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 26, -90, 90);
}

void GDAnaglyph::set_azimuth(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 27, fmodf(value + 180, 360) - 180, -180, 180);
}
float GDAnaglyph::get_azimuth() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 27, -180, 180);
}

void GDAnaglyph::set_distance(const float value) {
	AnaglyphBridge::SetParamScaled(&state, 28, value, 0.1, 10);
}
float GDAnaglyph::get_distance() {
	return AnaglyphBridge::GetParamScaledDirect(&state, 28, 0.1, 10);
}

void GDAnaglyph::_bind_methods() {
	// (See https://docs.godotengine.org/en/latest/classes/class_%40globalscope.html#enum-globalscope-propertyhint
	//  for how the hint string works.)
	REGISTER(FLOAT, wet, "percentage", PROPERTY_HINT_RANGE, "0,100,0.1,suffix:%");
	REGISTER(FLOAT, gain, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");

	ADD_GROUP("Binaural Personalisation", "");
	REGISTER(FLOAT, hrtf_id, "id", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(BOOL, use_custom_circumference, "value", PROPERTY_HINT_NONE, "");
	REGISTER(FLOAT, head_circumference, "cm", PROPERTY_HINT_RANGE, "20,80,0.1,suffix:cm");
	REGISTER(FLOAT, responsiveness, "value", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(BOOL, bypass_binaural, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Localisation", "");
	REGISTER(BOOL, bypass_parallax, "bypass", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, bypass_shadow, "bypass", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, bypass_micro_oscillations, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Attenuation", "");
	REGISTER(FLOAT, min_attenuation, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
	REGISTER(FLOAT, max_attenuation, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
	REGISTER(FLOAT, attenuation_exponent, "exponent", PROPERTY_HINT_RANGE, "0,2,0.1");
	REGISTER(BOOL, bypass_attenuation, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Reverb", "");
	REGISTER(FLOAT, room_id, "id", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(INT, reverb_type, "type", PROPERTY_HINT_ENUM, "OMNI:0,2D:1,3D 1st:2, 3D 2nd:3");
	REGISTER(FLOAT, reverb_gain, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");
	REGISTER(VECTOR3, reverb_EQ, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");
	REGISTER(BOOL, bypass_reverb, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Position", "");
	REGISTER(FLOAT, elevation, "angle", PROPERTY_HINT_RANGE, "-90,90,0.1,degrees");
	REGISTER(FLOAT, azimuth, "angle", PROPERTY_HINT_RANGE, "-180,180,0.1,or_greater,or_less,degrees");
	REGISTER(FLOAT, distance, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
}