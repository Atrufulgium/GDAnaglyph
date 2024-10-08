// Anaglyph is picky about its whereabouts, so I can't just include the dll.
// So manually copy over Anaglyph from demo/resources to <export path>/ whenever
// the project is being exported.

#ifndef GDANAGLYPH_EXPORT
#define GDANAGLYPH_EXPORT

#include "gdanaglyph.h"

#include <godot_cpp/classes/editor_export_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>

namespace godot {
	class AnaglyphExportPlugin : public EditorExportPlugin {
		GDCLASS(AnaglyphExportPlugin, EditorExportPlugin);

		String path;
		
		// Recursively copies everything from "source" dir to "target" dir.
		static Error copy_dir_absolute(const String& source, const String& target);

		// Recursively deletes all .dll, .sofa, and .DS_Store files in a directory.
		// In other words, all filetypes relevant to Anaglyph.
		// If `mock` is true, it simply logs all files it would've deleted.
		static Error clear_folder_absolute(const String& target, bool mock = false);

	protected:
		static void _bind_methods();

	public:
		String _get_name() const override;
		void _export_begin(const PackedStringArray& features, bool is_debug, const String& path, uint32_t flags) override;
		void _export_end() override;
	};

	class AnaglyphPlugin : public EditorPlugin {
		GDCLASS(AnaglyphPlugin, EditorPlugin);

		Ref<AnaglyphExportPlugin> export_plugin;

	protected:
		static void _bind_methods();

	public:
		void _enter_tree() override;
		void _exit_tree() override;
	};
}

#endif // GDANAGLYPH_EXPORT