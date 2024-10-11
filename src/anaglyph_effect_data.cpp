#include "anaglyph_effect.h"
#include "anaglyph_dll_bridge.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

AnaglyphEffectData::AnaglyphEffectData() {
	// These are the defaults in the Anaglyph dll.
	// Too lazy to load them from there properly.
	// (Also convenient in case I want different defaults.)
	wet = 100;
	gain = 0;

	hrtf_id = 0;
	use_custom_circumference = false;
	head_circumference = 57.5;
	responsiveness = 0.04;
	bypass_binaural = false;

	bypass_parallax = false;
	bypass_shadow = false;
	bypass_micro_oscillations = false;

	min_attenuation = 0.1;
	max_attenuation = 10;
	attenuation_exponent = 1;
	bypass_attenuation = false;

	room_id = 0.5;
	reverb_type = ANAGLYPH_REVERB_2D;
	reverb_gain = 0;
	reverb_EQ = Vector3(0, 0, 0);
	bypass_reverb = false;

	azimuth = 0;
	elevation = 0;
	distance = 0.3;
}

AnaglyphEffectData::~AnaglyphEffectData() { }

void AnaglyphEffectData::set_wet(const float percentage) {
	wet = CLAMP(percentage, 0, 100);
}
float AnaglyphEffectData::get_wet() {
	return wet;
}

void AnaglyphEffectData::set_gain(const float value) {
	gain = CLAMP(value, -40, 15);
}
float AnaglyphEffectData::get_gain() {
	return gain;
}

void AnaglyphEffectData::set_hrtf_id(const float value) {
	hrtf_id = CLAMP(value, 0, 1);
}
float AnaglyphEffectData::get_hrtf_id() {
	return hrtf_id;
}

void AnaglyphEffectData::set_use_custom_circumference(const bool value) {
	use_custom_circumference = value;
}
bool AnaglyphEffectData::get_use_custom_circumference() {
	return use_custom_circumference;
}

void AnaglyphEffectData::set_head_circumference(const float value) {
	head_circumference = CLAMP(value, 20, 80);
}
float AnaglyphEffectData::get_head_circumference() {
	return head_circumference;
}

void AnaglyphEffectData::set_responsiveness(const float value) {
	responsiveness = CLAMP(value, 0, 1);
}
float AnaglyphEffectData::get_responsiveness() {
	return responsiveness;
}

void AnaglyphEffectData::set_bypass_binaural(const bool value) {
	bypass_binaural = value;
}
bool AnaglyphEffectData::get_bypass_binaural() {
	return bypass_binaural;
}

void AnaglyphEffectData::set_bypass_parallax(const bool value) {
	bypass_parallax = value;
}
bool AnaglyphEffectData::get_bypass_parallax() {
	return bypass_parallax;
}

void AnaglyphEffectData::set_bypass_shadow(const bool value) {
	bypass_shadow = value;
}
bool AnaglyphEffectData::get_bypass_shadow() {
	return bypass_shadow;
}

void AnaglyphEffectData::set_bypass_micro_oscillations(const bool value) {
	bypass_micro_oscillations = value;
}
bool AnaglyphEffectData::get_bypass_micro_oscillations() {
	return bypass_micro_oscillations;
}

void AnaglyphEffectData::set_min_attenuation(const float value) {
	min_attenuation = CLAMP(value, 0.1, 10);
	float max = max_attenuation;
	if (max < value) {
		set_max_attenuation(value);
	}
}
float AnaglyphEffectData::get_min_attenuation() {
	return min_attenuation;
}

void AnaglyphEffectData::set_max_attenuation(const float value) {
	max_attenuation = CLAMP(value, 0.1, 10);
	float min = min_attenuation;
	if (min > value) {
		set_min_attenuation(value);
	}
}
float AnaglyphEffectData::get_max_attenuation() {
	return max_attenuation;
}

void AnaglyphEffectData::set_attenuation_exponent(const float value) {
	attenuation_exponent = CLAMP(value, 0, 2);
}
float AnaglyphEffectData::get_attenuation_exponent() {
	return attenuation_exponent;
}

void AnaglyphEffectData::set_bypass_attenuation(const bool value) {
	bypass_attenuation = value;
}
bool AnaglyphEffectData::get_bypass_attenuation() {
	return bypass_attenuation;
}

void AnaglyphEffectData::set_room_id(const float value) {
	room_id = CLAMP(value, 0, 1);
}
float AnaglyphEffectData::get_room_id() {
	return room_id;
}

void AnaglyphEffectData::set_reverb_type(const AnaglyphReverbType value) {
	reverb_type = value;
}
AnaglyphEffectData::AnaglyphReverbType AnaglyphEffectData::get_reverb_type() {
	return reverb_type;
}

void AnaglyphEffectData::set_reverb_gain(const float value) {
	reverb_gain = CLAMP(value, -40, 15);
}
float AnaglyphEffectData::get_reverb_gain() {
	return reverb_gain;
}

void AnaglyphEffectData::set_reverb_EQ(const Vector3 value) {
	reverb_EQ.x = CLAMP(value.x, -40, 15);
	reverb_EQ.y = CLAMP(value.y, -40, 15);
	reverb_EQ.z = CLAMP(value.z, -40, 15);
}
Vector3 AnaglyphEffectData::get_reverb_EQ() {
	return reverb_EQ;
}

void AnaglyphEffectData::set_bypass_reverb(const bool value) {
	bypass_reverb = value;
}
bool AnaglyphEffectData::get_bypass_reverb() {
	return bypass_reverb;
}

void AnaglyphEffectData::set_azimuth(const float value) {
	azimuth = CLAMP(fmodf(value + 180, 360) - 180, -180, 180);
}
float AnaglyphEffectData::get_azimuth() {
	return azimuth;
}

void AnaglyphEffectData::set_elevation(const float value) {
	elevation = CLAMP(value, -90, 90);
}
float AnaglyphEffectData::get_elevation() {
	return elevation;
}

void AnaglyphEffectData::set_distance(const float value) {
	distance = CLAMP(value, 0.1, 10);
}
float AnaglyphEffectData::get_distance() {
	return distance;
}

void AnaglyphEffectData::_bind_methods() {
	// (See https://docs.godotengine.org/en/latest/classes/class_%40globalscope.html#enum-globalscope-propertyhint
	//  for how the hint string works.)
	REGISTER(FLOAT, wet, AnaglyphEffectData, "percentage", PROPERTY_HINT_RANGE, "0,100,0.1,suffix:%");
	REGISTER(FLOAT, gain, AnaglyphEffectData, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");

	ADD_GROUP("Binaural Personalisation", "");
	REGISTER(FLOAT, hrtf_id, AnaglyphEffectData, "id", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(BOOL, use_custom_circumference, AnaglyphEffectData, "value", PROPERTY_HINT_NONE, "");
	REGISTER(FLOAT, head_circumference, AnaglyphEffectData, "cm", PROPERTY_HINT_RANGE, "20,80,0.1,suffix:cm");
	REGISTER(FLOAT, responsiveness, AnaglyphEffectData, "value", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(BOOL, bypass_binaural, AnaglyphEffectData, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Localisation", "");
	REGISTER(BOOL, bypass_parallax, AnaglyphEffectData, "bypass", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, bypass_shadow, AnaglyphEffectData, "bypass", PROPERTY_HINT_NONE, "");
	REGISTER(BOOL, bypass_micro_oscillations, AnaglyphEffectData, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Attenuation", "");
	REGISTER(FLOAT, min_attenuation, AnaglyphEffectData, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
	REGISTER(FLOAT, max_attenuation, AnaglyphEffectData, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");
	REGISTER(FLOAT, attenuation_exponent, AnaglyphEffectData, "exponent", PROPERTY_HINT_RANGE, "0,2,0.1");
	REGISTER(BOOL, bypass_attenuation, AnaglyphEffectData, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Reverb", "");
	REGISTER(FLOAT, room_id, AnaglyphEffectData, "id", PROPERTY_HINT_RANGE, "0,1");
	REGISTER(INT, reverb_type, AnaglyphEffectData, "type", PROPERTY_HINT_ENUM, "OMNI:0,2D:1,3D 1st:2, 3D 2nd:3");
	REGISTER(FLOAT, reverb_gain, AnaglyphEffectData, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");
	REGISTER(VECTOR3, reverb_EQ, AnaglyphEffectData, "dB", PROPERTY_HINT_RANGE, "-40,15,0.1,suffix:dB");
	REGISTER(BOOL, bypass_reverb, AnaglyphEffectData, "bypass", PROPERTY_HINT_NONE, "");

	ADD_GROUP("Position", "");
	REGISTER(FLOAT, azimuth, AnaglyphEffectData, "angle", PROPERTY_HINT_RANGE, "-180,180,0.1,or_greater,or_less,degrees");
	REGISTER(FLOAT, elevation, AnaglyphEffectData, "angle", PROPERTY_HINT_RANGE, "-90,90,0.1,degrees");
	REGISTER(FLOAT, distance, AnaglyphEffectData, "meters", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m");

	BIND_ENUM_CONSTANT(ANAGLYPH_REVERB_OMNI);
	BIND_ENUM_CONSTANT(ANAGLYPH_REVERB_2D);
	BIND_ENUM_CONSTANT(ANAGLYPH_REVERB_3D_1);
	BIND_ENUM_CONSTANT(ANAGLYPH_REVERB_3D_2);
}