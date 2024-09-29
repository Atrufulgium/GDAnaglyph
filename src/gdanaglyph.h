#ifndef GDANAGLYPH
#define GDANAGLYPH

#include "AudioPluginInterface.h"

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_frame.hpp>

namespace godot {

	class GDAnaglyph;

	class GDAnaglyphInstance : public AudioEffectInstance {
		GDCLASS(GDAnaglyphInstance, AudioEffectInstance);
		friend class GDAnaglyph;

		Ref<GDAnaglyph> base;
		UnityAudioEffectState state;

		Vector<AudioFrame> audio_buffer;
		unsigned int buffer_pos;
		unsigned int buffer_mask;

	protected:
		static void _bind_methods();

	public:
		GDAnaglyphInstance();
		~GDAnaglyphInstance();

		// (really AudioFrame* p_src_frames)

		// From the Godot docs:
		// Called by the AudioServer to process this effect. When
		// `_process_silence` is not overridden or it returns false,
		// this method is called only when the bus is active.
		virtual void _process(const void* p_src_frames, AudioFrame* p_dst_frames, int32_t p_frame_count) override;
		virtual bool _process_silence() const override;
	};

	class GDAnaglyph : public AudioEffect {
		GDCLASS(GDAnaglyph, AudioEffect);
		friend class GDAnagylphInstance;

	protected:
		static void _bind_methods();

	public:
		GDAnaglyph();
		~GDAnaglyph();

		virtual Ref<AudioEffectInstance> _instantiate() override;
	};
	
}

#endif //GDANAGLYPH