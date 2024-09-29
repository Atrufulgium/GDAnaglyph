#ifndef GDANAGLYPH_BRIDGE
#define GDANAGLYPH_BRIDGE

#include "AudioPluginInterface.h"

#include <godot_cpp/classes/audio_frame.hpp>
#include <string>

namespace godot {
	// This anaglyph dll is originally meant for Unity.
	// This class handles reading out the data from the dll. See
	// AudioPluginInterface.h
	class AnaglyphBridge {
	private:
		// The actual reference that the dll gives us.
		// This is constant throughout the lifetime of the program.
		static UnityAudioEffectDefinition* anaglyph_definition;

		// Whether locating the dll/the entry point failed, and we should not
		// try again. Ensure that whenever this is true, *anaglyph_definition
		// is the nullpointer.
		static bool loading_failed;
		
		// Relative path to the dll.
		// Note that anaglyph is picky, and that all the data needs to be
		// stored relative to the dll in the following file structure:
		// AnaglyphBridge::dll_path
		//  |- audioplugin_Anaglyph.dll
		//  |- .DS_Store
		//  '- anaglyph_plugin_data
		//     |- .DS_Store
		//     '- all .sofa files
		static std::string dll_path;

		// Unfortunately Godot and Unity disagree on buffer sizes.
		// Transposing the buffer in place is very messy, so I need temporary
		// helper buffers. This is the size limit for each channel.
		static const int buffer_translate_limit = 1000;
		
		// The workhorse of GetEffectData();
		static UnityAudioEffectDefinition* GetDataFromDLL();

		// Disables anaglyph in case something goes wrong.
		static void DisableAnaglyph(std::string msg);

	public:
		// Anaglyph is defined as a Unity plugin. A bunch of data needs to be
		// read from the dll, which will get put in here.
		// Loading may fail, in which case `nullptr` is returned.
		// The first time this is called, Anaglyph may show its own errors in
		// its own UI on screen.
		static UnityAudioEffectDefinition* GetEffectData();

		// Create a new DSP instance.
		static UNITY_AUDIODSP_RESULT Create(UnityAudioEffectState* state);

		// Release an existing DSP instance.
		static UNITY_AUDIODSP_RESULT Release(UnityAudioEffectState* state);
		
		// Reset all parameters to their default values.
		static UNITY_AUDIODSP_RESULT Reset(UnityAudioEffectState* state);
		
		// Apply the effect.
		static UNITY_AUDIODSP_RESULT Process(UnityAudioEffectState* state, const AudioFrame* inbuffer, AudioFrame* outbuffer, unsigned int length);
		
		// Set a float param, as determined by its index.
		static UNITY_AUDIODSP_RESULT SetParam(UnityAudioEffectState* state, int index, float value);
		
		// Get a float param value, as determined by its index.
		static UNITY_AUDIODSP_RESULT GetParam(UnityAudioEffectState* state, int index, float* value);
		
		// Set a float param that lives on [min,max] by its index.
		static UNITY_AUDIODSP_RESULT SetParamScaled(UnityAudioEffectState* state, int index, float value, float min, float max);
		
		// Get a float param that lives on [min,max] by its index.
		static UNITY_AUDIODSP_RESULT GetParamScaled(UnityAudioEffectState* state, int index, float* value, float min, float max);

	};
}

#endif // GDANAGLYPH_BRIDGE