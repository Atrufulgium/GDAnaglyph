#include "gdanaglyph_bridge.h"

#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <windows.h>
#include <libloaderapi.h>

using namespace godot;

UnityAudioEffectDefinition* AnaglyphBridge::anaglyph_definition = nullptr;
bool AnaglyphBridge::loading_failed = false;
std::string AnaglyphBridge::dll_path = ".\\Anaglyph\\audioplugin_Anaglyph.dll";

typedef int(AUDIO_CALLING_CONVENTION* GetAudioEffectDefinitions)(UnityAudioEffectDefinition*** descptr);

UnityAudioEffectDefinition* AnaglyphBridge::GetEffectData() {
	if (anaglyph_definition != nullptr) {
		return anaglyph_definition;
	}
	if (loading_failed) {
		return nullptr;
	}

	UnityAudioEffectDefinition* def = GetDataFromDLL();
	if (def == nullptr) {
		loading_failed = true;
		return def;
	}
	godot::UtilityFunctions::print("Finished processing Anaglyph dll!\nDll: ", def->name, " Version: ", def->pluginversion);
	return def;
}

UnityAudioEffectDefinition* AnaglyphBridge::GetDataFromDLL() {
	godot::UtilityFunctions::print("Loading Anaglyph dll at ", dll_path.c_str());
	HMODULE dll = LoadLibraryA(dll_path.c_str());
	ERR_FAIL_NULL_V_MSG(dll, nullptr, "Did not find Anaglyph dll.");

	GetAudioEffectDefinitions call = (GetAudioEffectDefinitions)GetProcAddress(
		dll,
		"UnityGetAudioEffectDefinitions"
	);
	ERR_FAIL_NULL_V_MSG(call, nullptr, "Did not find Anaglyph entry point in dll.");

	UnityAudioEffectDefinition** defs = nullptr;
	int effects = call(&defs);

	if (effects != 1)
		godot::UtilityFunctions::push_warning("Expected Anaglyph to have 1 effect, but got ", effects, " effects instead.\nThis _may_ not be fatal, but likely is.");
	anaglyph_definition = *defs;

	if (anaglyph_definition->pluginversion != 2308)
		godot::UtilityFunctions::push_warning("Expected Anaglyph version 0.9.4c (internal version 2308), but got internal version ", anaglyph_definition->pluginversion, " instead.\nWhile this still may work properly, this is not supported and may crash.");
	return anaglyph_definition;
}

void AnaglyphBridge::DisableAnaglyph(std::string msg) {
	anaglyph_definition = nullptr;
	if (loading_failed == false) {
		loading_failed = true;
		godot::UtilityFunctions::push_error(msg.c_str());
	}
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::Create(UnityAudioEffectState* state) {
	GetEffectData(); // Just to ensure anaglyph is properly loaded.
	// In Unity's examples, only the state's *effectdata was written to.
	// It feels safe to assume the rest is input.
	if (anaglyph_definition == nullptr)
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;

	// Godots sample rate can be either 44.1 or 48, take note.
	state->structsize = sizeof(UnityAudioEffectState);
	state->samplerate = AudioServer::get_singleton()->get_mix_rate();
	state->flags = UnityAudioEffectStateFlags_IsPlaying;
	state->internal = malloc(sizeof(float) * dsp_buffer_size * 2);
	state->dspbuffersize = 512; // Is this exact, or is an upper bound sufficient? Does this need to be set on create, or on process?
	state->hostapiversion = UNITY_AUDIO_PLUGIN_API_VERSION;

	UNITY_AUDIODSP_RESULT res = anaglyph_definition->create(state);
	if (res == UNITY_AUDIODSP_ERR_UNSUPPORTED) {
		DisableAnaglyph("Internal Anaglyph error while initializing. Anaglyph has been disabled.");
	}
	Reset(state);

	return res;
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::Release(UnityAudioEffectState* state) {
	free(state->internal);
	if (anaglyph_definition == nullptr) {
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}
	UNITY_AUDIODSP_RESULT res = anaglyph_definition->release(state);
	return res;
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::Reset(UnityAudioEffectState* state) {
	if (anaglyph_definition == nullptr) {
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}
	anaglyph_definition->reset(state);

	// Set the params not exposed to their lifetime-constant values.
	// (See gdanaglyph.h for he meaning of these magic numbers.)
	SetParam(state, 0, 0);
	SetParam(state, 2, 0);
	SetParam(state, 7, 0);
	SetParam(state, 10, 0);
	SetParam(state, 11, 0);
	SetParam(state, 12, 0);
	SetParam(state, 14, 0);
	SetParam(state, 17, 1);
	SetParam(state, 29, 0);
	return UNITY_AUDIODSP_OK;
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::Process(UnityAudioEffectState* state, const AudioFrame* inbuffer, AudioFrame* outbuffer, unsigned int length) {
	// We only need to do something if anaglyph exists, otherwise just copy
	// over the data without changes.
	if (anaglyph_definition == nullptr) {
		for (int i = 0; i < length; i++) {
			outbuffer[i] = inbuffer[i];
		}
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}
	// The layout of audio in Unity is as follows:
	// L1 L2 L3 ... R1 R2 R3...
	// The layout of audio in Godot is as follows:
	// L1 R1 L2 R2 L3 R3 ...
	// We need to do a translation pass.

	int limit = AnaglyphBridge::dsp_buffer_size;
	if (length > limit) {
		DisableAnaglyph("Expected a DSP buffer size of at most 4096, but got something larger. Disabling Anaglyph.");
		for (int i = 0; i < length; i++) {
			outbuffer[i] = inbuffer[i];
		}
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}

	// Use `outbuffer`'s memory to store the Anaglyph input.
	// Use `state->internal`'s memory to store the Anaglyph output.
	// Finally convert this output into `outbuffer`.
	float* anaglyph_in_l = (float*)outbuffer;
	float* anaglyph_in_r = anaglyph_in_l + length;
	float* anaglyph_out_l = (float*)state->internal;
	float* anaglyph_out_r = anaglyph_out_l + length;
	for (int i = 0; i < length; i++) {
		anaglyph_in_l[i] = inbuffer[i].left;
		anaglyph_in_r[i] = inbuffer[i].right;
	}

	state->dspbuffersize = length;
	UNITY_AUDIODSP_RESULT res = anaglyph_definition->process(state, anaglyph_in_l, anaglyph_out_l, length, 2, 2);

	if (res == UNITY_AUDIODSP_ERR_UNSUPPORTED) {
		DisableAnaglyph("Something unexpected went wrong while running Anaglyph. Anaglyph has been disabled.");
		for (int i = 0; i < length; i++) {
			outbuffer[i] = inbuffer[i];
		}
		return res;
	}

	for (int i = 0; i < length; i++) {
		AudioFrame frame;
		frame.left = anaglyph_out_l[i];
		frame.right = anaglyph_out_r[i];
		outbuffer[i] = frame;
	}
	return res;
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::SetParam(UnityAudioEffectState* state, int index, float value) {
	if (anaglyph_definition == nullptr) {
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}
	value = CLAMP(value, 0, 1);
	return anaglyph_definition->setfloatparameter(state, index, value);
}	

UNITY_AUDIODSP_RESULT AnaglyphBridge::GetParam(UnityAudioEffectState* state, int index, float* value)  {
	if (anaglyph_definition == nullptr) {
		return UNITY_AUDIODSP_ERR_UNSUPPORTED;
	}
	// It seems to be the case NativeAudio SDK devs are expected to handle
	// either outputs being null. Nice.
	// TODO: Godot calls the getter periodically. If this lags (idk why the
	// Unity version of the plugin lags, maybe something similar), keep track
	// of Anaglyph's data locally, and only do sets. (Except for resets.)
	return anaglyph_definition->getfloatparameter(state, index, value, nullptr);
}

float AnaglyphBridge::GetParamDirect(UnityAudioEffectState* state, int index) {
	float value = 0;
	GetParam(state, index, &value);
	return value;
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::SetParamScaled(UnityAudioEffectState* state, int index, float value, float min, float max) {
	return SetParam(state, index, (value - min) / (max - min));
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::GetParamScaled(UnityAudioEffectState* state, int index, float* value, float min, float max) {
	UNITY_AUDIODSP_RESULT res = GetParam(state, index, value);
	*value = *value * (max - min) + min;
	return res;
}

float AnaglyphBridge::GetParamScaledDirect(UnityAudioEffectState* state, int index, float min, float max) {
	float value = 0;
	GetParamScaled(state, index, &value, min, max);
	return value;
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::SetParamBool(UnityAudioEffectState* state, int index, bool value) {
	if (value) {
		return SetParam(state, index, 1);
	}
	else {
		return SetParam(state, index, 0);
	}
}

UNITY_AUDIODSP_RESULT AnaglyphBridge::GetParamBool(UnityAudioEffectState* state, int index, bool* value) {
	float float_value = 0;
	UNITY_AUDIODSP_RESULT res = GetParam(state, index, &float_value);
	if (float_value == 0) {
		*value = false;
	}
	else {
		*value = true;
	}
	return res;
}

bool AnaglyphBridge::GetParamBoolDirect(UnityAudioEffectState* state, int index) {
	bool value = false;
	GetParamBool(state, index, &value);
	return value;
}