#include "anaglyph_effect.h"
#include "anaglyph_dll_bridge.h"
#include "helpers.h"

using namespace godot;

AudioFrame* AnaglyphEffect::warmup_buffer = nullptr;

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
		AnaglyphHelpers::print_warning("Anaglyph dll did not load correctly. This Audio Effect won't do anything.");
		return;
	}

	UnityAudioEffectState st{};
	state = st;
	AnaglyphBridge::Create(&state);
	
	// Ensure the model is prepared.
	// (Not freeing because (1) this can get called over the lifetime a bunch'a
	//  times, and (2) godot shadowed free because I'm not supposed to use native
	//  memory like this, and (3) it's only 4~16kb getting leaked if this never
	//  gets used again.)
	if (warmup_buffer == nullptr) {
		warmup_buffer = (AudioFrame*) calloc(AnaglyphBridge::get_dsp_buffer_size(), sizeof(AudioFrame));
	}
	if (warmup_buffer != nullptr) {
		AnaglyphBridge::Process(&state, warmup_buffer, warmup_buffer, AnaglyphBridge::get_dsp_buffer_size());
		warmup_buffer = nullptr;
	}
}

AnaglyphEffect::~AnaglyphEffect() {
	AnaglyphBridge::Release(&state);
	if (effect_data != nullptr) {
		effect_data->most_recent_effect = nullptr;
	}
}

Ref<AudioEffectInstance> AnaglyphEffect::_instantiate() {
	Ref<AnaglyphEffectInstance> ins;
	ins.instantiate();
	ins->base = Ref<AnaglyphEffect>(this);

	return ins;
}

void AnaglyphEffect::set_wet(const float percentage) {
	ensure_effect_data_exists();
	effect_data->set_wet(percentage);
	send_wet();
}
void AnaglyphEffect::send_wet() {
	AnaglyphBridge::SetParamScaled(&state, 18, effect_data->get_wet(), 0, 100);
}
float AnaglyphEffect::get_wet() {
	ensure_effect_data_exists();
	return effect_data->get_wet();
}

void AnaglyphEffect::set_gain(const float value) {
	ensure_effect_data_exists();
	effect_data->set_gain(value);
	send_gain();
}
void AnaglyphEffect::send_gain() {
	AnaglyphBridge::SetParamScaled(&state, 20, effect_data->get_gain(), -40, 15);
}
float AnaglyphEffect::get_gain() {
	ensure_effect_data_exists();
	return effect_data->get_gain();
}

void AnaglyphEffect::set_hrtf_id(const float value) {
	ensure_effect_data_exists();
	effect_data->set_hrtf_id(value);
	send_hrtf_id();
}
void AnaglyphEffect::send_hrtf_id() {
	AnaglyphBridge::SetParam(&state, 15, effect_data->get_hrtf_id());
}
float AnaglyphEffect::get_hrtf_id() {
	ensure_effect_data_exists();
	return effect_data->get_hrtf_id();
}

void AnaglyphEffect::set_use_custom_circumference(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_use_custom_circumference(value);
	send_use_custom_circumference();
}
void AnaglyphEffect::send_use_custom_circumference() {
	AnaglyphBridge::SetParamBool(&state, 8, effect_data->get_use_custom_circumference());
}
bool AnaglyphEffect::get_use_custom_circumference() {
	ensure_effect_data_exists();
	return effect_data->get_use_custom_circumference();
}

void AnaglyphEffect::set_head_circumference(const float value) {
	ensure_effect_data_exists();
	effect_data->set_head_circumference(value);
	send_head_circumference();
}
void AnaglyphEffect::send_head_circumference() {
	AnaglyphBridge::SetParamScaled(&state, 25, effect_data->get_head_circumference(), 20, 80);
}
float AnaglyphEffect::get_head_circumference() {
	ensure_effect_data_exists();
	return effect_data->get_head_circumference();
}

void AnaglyphEffect::set_responsiveness(const float value) {
	ensure_effect_data_exists();
	effect_data->set_responsiveness(value);
	send_responsiveness();
}
void AnaglyphEffect::send_responsiveness() {
	AnaglyphBridge::SetParam(&state, 32, effect_data->get_responsiveness());
}
float AnaglyphEffect::get_responsiveness() {
	ensure_effect_data_exists();
	return effect_data->get_responsiveness();
}

void AnaglyphEffect::set_bypass_binaural(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_binaural(value);
	send_bypass_binaural();
}
void AnaglyphEffect::send_bypass_binaural() {
	AnaglyphBridge::SetParamBool(&state, 4, effect_data->get_bypass_binaural());
}
bool AnaglyphEffect::get_bypass_binaural() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_binaural();
}

void AnaglyphEffect::set_bypass_parallax(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_parallax(value);
	send_bypass_parallax();
}
void AnaglyphEffect::send_bypass_parallax() {
	AnaglyphBridge::SetParamBool(&state, 5, effect_data->get_bypass_parallax());
}
bool AnaglyphEffect::get_bypass_parallax() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_parallax();
}

void AnaglyphEffect::set_bypass_shadow(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_shadow(value);
	send_bypass_shadow();
}
void AnaglyphEffect::send_bypass_shadow() {
	AnaglyphBridge::SetParamBool(&state, 1, effect_data->get_bypass_shadow());
}
bool AnaglyphEffect::get_bypass_shadow() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_shadow();
}

void AnaglyphEffect::set_bypass_micro_oscillations(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_micro_oscillations(value);
	send_bypass_micro_oscillations();
}
void AnaglyphEffect::send_bypass_micro_oscillations() {
	AnaglyphBridge::SetParamBool(&state, 9, effect_data->get_bypass_micro_oscillations());
}
bool AnaglyphEffect::get_bypass_micro_oscillations() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_micro_oscillations();
}

void AnaglyphEffect::set_min_attenuation(const float value) {
	ensure_effect_data_exists();
	effect_data->set_min_attenuation(value);
	send_min_attenuation();
}
void AnaglyphEffect::send_min_attenuation() {
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
	send_max_attenuation();
}
void AnaglyphEffect::send_max_attenuation() {
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
	send_attenuation_exponent();
}
void AnaglyphEffect::send_attenuation_exponent() {
	AnaglyphBridge::SetParamScaled(&state, 19, effect_data->get_attenuation_exponent(), 0, 2);
}
float AnaglyphEffect::get_attenuation_exponent() {
	ensure_effect_data_exists();
	return effect_data->get_attenuation_exponent();
}

void AnaglyphEffect::set_bypass_attenuation(const bool value) {
	ensure_effect_data_exists();
	effect_data->set_bypass_attenuation(value);
	send_bypass_attenuation();
}
void AnaglyphEffect::send_bypass_attenuation() {
	AnaglyphBridge::SetParamBool(&state, 3, effect_data->get_bypass_attenuation());
}
bool AnaglyphEffect::get_bypass_attenuation() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_attenuation();
}

void AnaglyphEffect::set_room_id(const float value) {
	ensure_effect_data_exists();
	effect_data->set_room_id(value);
	send_room_id();
}
void AnaglyphEffect::send_room_id() {
	AnaglyphBridge::SetParam(&state, 16, effect_data->get_room_id());
}
float AnaglyphEffect::get_room_id() {
	ensure_effect_data_exists();
	return effect_data->get_room_id();
}

void AnaglyphEffect::set_reverb_type(const AnaglyphEffectData::AnaglyphReverbType value) {
	ensure_effect_data_exists();
	effect_data->set_reverb_type(value);
	send_reverb_type();
}
void AnaglyphEffect::send_reverb_type() {
	AnaglyphBridge::SetParamScaled(&state, 13, (float)effect_data->get_reverb_type(), 0, 3);
}
AnaglyphEffectData::AnaglyphReverbType AnaglyphEffect::get_reverb_type() {
	ensure_effect_data_exists();
	return effect_data->get_reverb_type();
}

void AnaglyphEffect::set_reverb_gain(const float value) {
	ensure_effect_data_exists();
	effect_data->set_reverb_gain(value);
	send_reverb_gain();
}
void AnaglyphEffect::send_reverb_gain() {
	AnaglyphBridge::SetParamScaled(&state, 21, effect_data->get_reverb_gain(), -40, 15);
}
float AnaglyphEffect::get_reverb_gain() {
	ensure_effect_data_exists();
	return effect_data->get_reverb_gain();
}

void AnaglyphEffect::set_reverb_EQ(const Vector3 value) {
	ensure_effect_data_exists();
	effect_data->set_reverb_EQ(value);
	send_reverb_EQ();
}
void AnaglyphEffect::send_reverb_EQ() {
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
	send_bypass_reverb();
}
void AnaglyphEffect::send_bypass_reverb() {
	AnaglyphBridge::SetParamBool(&state, 6, effect_data->get_bypass_reverb());
}
bool AnaglyphEffect::get_bypass_reverb() {
	ensure_effect_data_exists();
	return effect_data->get_bypass_reverb();
}

void AnaglyphEffect::set_azimuth(const float value) {
	ensure_effect_data_exists();
	effect_data->set_azimuth(value);
	send_azimuth();
}
void AnaglyphEffect::send_azimuth() {
	AnaglyphBridge::SetParamScaled(&state, 27, effect_data->get_azimuth(), -180, 180);
}
float AnaglyphEffect::get_azimuth() {
	ensure_effect_data_exists();
	return effect_data->get_azimuth();
}

void AnaglyphEffect::set_elevation(const float value) {
	ensure_effect_data_exists();
	effect_data->set_elevation(value);
	send_elevation();
}
void AnaglyphEffect::send_elevation() {
	AnaglyphBridge::SetParamScaled(&state, 26, effect_data->get_elevation(), -90, 90);
}
float AnaglyphEffect::get_elevation() {
	ensure_effect_data_exists();
	return effect_data->get_elevation();
}

void AnaglyphEffect::set_distance(const float value) {
	ensure_effect_data_exists();
	effect_data->set_distance(value);
	send_distance();
}
void AnaglyphEffect::send_distance() {
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
		AnaglyphHelpers::print_warning("Tried to `AnaglyphEffect.set_effect_data(null)`. This is not allowed; ignoring the call.");
		return;
	}
	if (effect_data != nullptr) {
		effect_data->most_recent_effect = nullptr;
	}
	effect_data = data;
	effect_data->most_recent_effect = this;
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

	ClassDB::bind_method(D_METHOD("set_effect_data", "data"), &AnaglyphEffect::set_effect_data);

	// Steal the helper method into this class.
	ClassDB::bind_static_method("AnaglyphEffect", D_METHOD("calculate_polar_position", "source", "listener"), &AnaglyphHelpers::calculate_polar_position);
}