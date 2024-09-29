#include "gdanaglyph.h"
#include "gdanaglyph_bridge.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

GDAnaglyphInstance::GDAnaglyphInstance() {
	UnityAudioEffectState st{};
	state = st;
	AnaglyphBridge::Create(&state);
}

GDAnaglyphInstance::~GDAnaglyphInstance() {
	AnaglyphBridge::Release(&state);
}

void GDAnaglyphInstance::_bind_methods() {

}

void GDAnaglyphInstance::_process(const void* p_src_frames, AudioFrame* p_dst_frames, int32_t p_frame_count) {
	// TODO: Why is this const void* and not const AudioFrame*?
	// Assuming const AudioFrame* for now, and I'll see whether it crashes.
	AnaglyphBridge::Process(&state, (AudioFrame*)p_src_frames, p_dst_frames, (unsigned int)p_frame_count);
}

bool GDAnaglyphInstance::_process_silence() const {
	return true;
}

GDAnaglyph::GDAnaglyph() {

}

GDAnaglyph::~GDAnaglyph() {

}

Ref<AudioEffectInstance> GDAnaglyph::_instantiate() {
	Ref<GDAnaglyphInstance> ins;
	ins.instantiate();
	ins->base = Ref<GDAnaglyphInstance>(this);

	UnityAudioEffectDefinition* defs = AnaglyphBridge::GetEffectData();

	if (defs == nullptr) {
		godot::UtilityFunctions::push_warning("Anaglyph dll did not load correctly. This Audio Effect won't do anything.");
	}

	return ins;
}

void GDAnaglyph::_bind_methods() {

}