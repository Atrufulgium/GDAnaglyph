#ifndef GDANAGLYPH_BUSES
#define GDANAGLYPH_BUSES

#include "gdanaglyph.h"

#include <godot_cpp/classes/audio_server.hpp>

namespace godot {
	class AnaglyphBusManager {

	private:
		static AnaglyphBusManager* singleton;

		// TODO: Take into account AudioServer::set_bus_layout

		Vector<StringName> anaglyph_buses;
		const int max_anaglyph_buses = 8;
		
		// These were formerly a StringName, but godot crashes on trying to
		// static-init most of its types.
		static char* a_bus_name;
		static char* s_bus_name;

		AudioServer* audio;

		// Adds an audio bus, and returns its index.
		// If the name is taken, it adds a digit until it isn't taken any more.
		StringName add_bus(StringName base_name);
		// Guarantees the existence of a bus, and returns its index.
		int guarantee_bus(StringName name);
		// Godot is name-first reorder-second.
		// But AudioServer works with indices only.
		// Returns -1 if it does not exist.
		int get_bus_index(StringName name);

	public:
		static AnaglyphBusManager* get_singleton();

		AnaglyphBusManager();
		~AnaglyphBusManager();

		// Tries to get a free anaglyph'd bus, which gets its output rerouted
		// into the base bus.
		// If there is no free bus, directly returns the base bus.
		StringName get_anaglyph_bus(StringName base_bus, Ref<GDAnaglyph> anaglyph_data);
		// Gets a muted bus.
		StringName get_silent_bus();
	};
}

#endif // GDANAGLYPH_BUSES