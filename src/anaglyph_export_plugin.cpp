#include "anaglyph_export_plugin.h"
#include "helpers.h"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

String AnaglyphExportPlugin::_get_name() const {
	return "Anaglyph Folder Exporter Plugin";
}

void AnaglyphExportPlugin::_export_begin(const PackedStringArray& features, bool is_debug, const String& p_path, uint32_t flags) {
	path = p_path;
}

void AnaglyphExportPlugin::_export_end() {
	// The path we get can be... loads.
	// On windows alone it can be:
	// - An absolute path ("C:/my/path/executable.exe")
	// - A path relative to the project ("build/executable.exe")
	// The latter is somewhat awkward.
	String target_path = path;
	if (!DirAccess::dir_exists_absolute(path)) {
		target_path = "res://" + path;
	}
	String full_source_path = ProjectSettings::get_singleton()->globalize_path("res://Anaglyph");
	String full_target_path = ProjectSettings::get_singleton()->globalize_path(target_path).get_base_dir() + "/Anaglyph";
	
	if (DirAccess::dir_exists_absolute(full_target_path)) {
		clear_folder_absolute(full_target_path);
	}
	// Godot only copies file per file, bleh.
	Error res = copy_dir_absolute(full_source_path, full_target_path);
	if (res != OK) {
		AnaglyphHelpers::print_error("Copying Anaglyph failed: ", res, "\nTry manually copying the `Anaglyph/` directory (itself) next to the built executable.");
	}
}

Error AnaglyphExportPlugin::copy_dir_absolute(const String& source, const String& target) {
	if (!DirAccess::dir_exists_absolute(source)) {
		AnaglyphHelpers::print_error("Copying directory failed -- ", source, " does not exist.");
		return ERR_FILE_NOT_FOUND;
	}
	Error res;
	if (!DirAccess::dir_exists_absolute(target)) {
		res = DirAccess::make_dir_recursive_absolute(target);
		if (res != OK) {
			return res;
		}
	}

	PackedStringArray files = DirAccess::get_files_at(source);
	for (int i = 0; i < files.size(); i++) {
		String s = source + String("/") + files[i];
		String t = target + String("/") + files[i];
		res = DirAccess::copy_absolute(s, t);
		if (res != OK) {
			AnaglyphHelpers::print_error("Could not copy file: Error ", res, "\n - Source: ", s, "\n - Target:", t);
			return res;
		}
	}

	PackedStringArray dirs = DirAccess::get_directories_at(source);
	for (int i = 0; i < dirs.size(); i++) {
		String s = source + String("/") + dirs[i];
		String t = target + String("/") + dirs[i];
		res = copy_dir_absolute(s, t);
		if (res != OK) {
			return res;
		}
	}
	return OK;
}

Error AnaglyphExportPlugin::clear_folder_absolute(const String& target, bool mock) {
	if (!DirAccess::dir_exists_absolute(target)) {
		AnaglyphHelpers::print_error("Clearing directory failed -- ", target, " does not exist.");
		return ERR_FILE_NOT_FOUND;
	}
	Error res;

	PackedStringArray files = DirAccess::get_files_at(target);
	for (int i = 0; i < files.size(); i++) {
		String ext = files[i].get_extension();
		if (ext == "dll" || ext == "sofa" || ext == "DS_Store") {
			String t = target + String("/") + files[i];
			if (mock) {
				AnaglyphHelpers::print("Would've deleted file ", t);
			}
			else {
				res = DirAccess::remove_absolute(t);
				if (res != OK) {
					AnaglyphHelpers::print_error("Could not delete file: Error ", res, "\n - Target: ", t);
					return res;
				}
			}
		}
	}

	PackedStringArray dirs = DirAccess::get_directories_at(target);
	for (int i = 0; i < dirs.size(); i++) {
		String t = target + String("/") + dirs[i];
		res = clear_folder_absolute(t, mock);
		if (res != OK) {
			return res;
		}
	}
	return OK;
}

void AnaglyphExportPlugin::_bind_methods() { }

void AnaglyphPlugin::_enter_tree() {
	if (export_plugin == nullptr) {
		export_plugin.instantiate();
	}
	add_export_plugin(export_plugin);
}

void AnaglyphPlugin::_exit_tree() {
	remove_export_plugin(export_plugin);
	export_plugin = Ref<AnaglyphExportPlugin>();
}

void AnaglyphPlugin::_bind_methods() { }