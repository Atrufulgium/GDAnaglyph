#ifndef GDANAGLYPH_BUSES
#define GDANAGLYPH_BUSES

#include "gdanaglyph.h"

#include <godot_cpp/classes/audio_server.hpp>

namespace godot {
	class AnaglyphBusManager {

	private:
		static AnaglyphBusManager* singleton;

		Vector<StringName> anaglyph_buses;
		// Maximum allowed Anaglyph buses. Beyond this, no new buses are
		// introduced, and instead a fallback should be used.
		// If this is reduced, it won't stop existing Anaglyph buses from
		// playing, but when returned, will delete them from the pool.
		int max_anaglyph_buses;
		int used_anaglyph_buses;
		
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
		// But AudioServer works with indices only, so you could say it's
		// reorder-first name-second. This converts between the two.
		// Returns -1 if no bus with this name exist.
		int get_bus_index(StringName name);

	public:
		static AnaglyphBusManager* get_singleton();

		AnaglyphBusManager();
		~AnaglyphBusManager();

		// Anaglyph needs some time to warm up. If you do not want to deal
		// with this, you can prepare some anaglyphs. This will ensure that
		// at least `count` buses are on standby (up until we have
		// `max_anaglyph_buses` buses).
		void prepare_anaglyph_buses(int count);

		// Tries to get a free anaglyph'd bus, which gets its output rerouted
		// into the base bus.
		// If there is no free bus, directly returns the base bus.
		StringName borrow_anaglyph_bus(const StringName& base_bus, const Ref<GDAnaglyph>& anaglyph_data);
		// Once you're done with a bus, return it.
		// This does not do any validation.
		void return_anaglyph_bus(const StringName& anaglyph_bus);
		// Gets a muted bus.
		StringName get_silent_bus();

		void set_max_anaglyph_buses(int max);
		int get_max_anaglyph_buses();
	};
}

#endif // GDANAGLYPH_BUSES