#include "anaglyph_export_plugin.h"
#include "audio_stream_player_anaglyph.h"
#include "anaglyph_dll_bridge.h"
#include "anaglyph_effect.h"
#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdanaglyph_module(ModuleInitializationLevel p_level) {
	// idk how module init works lol
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(AnaglyphExportPlugin);
		GDREGISTER_CLASS(AnaglyphPlugin);

		EditorPlugins::add_by_type<AnaglyphPlugin>();
	}
	else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(AnaglyphEffectData);
		GDREGISTER_CLASS(AnaglyphEffect);
		GDREGISTER_CLASS(AnaglyphEffectInstance);
		GDREGISTER_CLASS(AudioStreamPlayerAnaglyph);

		// Might as well load the dll at the start.
		// Note that we won't unload the dll at any point. Let it be cleaned up
		// together with the entire program.
		// TODO: Godot doesn't seem to print any debug data on load.
		AnaglyphBridge::GetEffectData();
	}

}

void uninitialize_gdanaglyph_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
	// Initialization.
	GDExtensionBool GDE_EXPORT gdanaglyph_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_gdanaglyph_module);
		init_obj.register_terminator(uninitialize_gdanaglyph_module);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}