#include "anaglyph_bus_manager.h"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

AnaglyphBusManager* AnaglyphBusManager::singleton = nullptr;
char* AnaglyphBusManager::a_bus_name = "[Anaglyph_Bus]";
char* AnaglyphBusManager::s_bus_name = "[Silent_Bus]";

StringName AnaglyphBusManager::add_bus(StringName base_name) {
	String name = base_name;
	int attempts = 1;
	int num_buses = audio->get_bus_count();
	while (true) {
		bool name_free = true;
		for (int i = 0; i < num_buses; i++) {
			if (audio->get_bus_name(i) == name) {
				name_free = false;
				break;
			}
		}
		if (!name_free) {
			attempts++;
			name = base_name + String(" ") + String(itos(attempts));
			attempts++;
		}
		else {
			break;
		}
	}

	// This method neither returns the index nor the name.
	// On top of that, it emits a "stuff changed" signal.
	// I don't know if the layout can change under my nose, but just create a
	// new bus, and assume it's at the end.
	int insert_index = num_buses;
	audio->add_bus(-1);
	audio->set_bus_name(insert_index, name);
	godot::UtilityFunctions::print("Added Anaglyph audio bus ", base_name);
	return name;
}

int AnaglyphBusManager::guarantee_bus(StringName name) {
	int index = get_bus_index(name);
	if (index == -1) {
		add_bus(name);
		return get_bus_index(name);
	}
	else {
		return index;
	}
}

int AnaglyphBusManager::get_bus_index(StringName name) {
	int num_buses = audio->get_bus_count();
	for (int i = 0; i < num_buses; i++) {
		if (audio->get_bus_name(i) == name) {
			return i;
		}
	}
	return -1;
}

AnaglyphBusManager* AnaglyphBusManager::get_singleton() {
	if (singleton == nullptr) {
		singleton = new AnaglyphBusManager();
		if (singleton == nullptr) {
			godot::UtilityFunctions::push_error("Couldn't allocate bus manager, expect a crash *very* soon.");
		}
	}
	return singleton;
}

AnaglyphBusManager::AnaglyphBusManager() {
	anaglyph_buses = Vector<StringName>();
	audio = AudioServer::get_singleton();
}

AnaglyphBusManager::~AnaglyphBusManager() {
	// I don't *think* have to free Vectors?
	// I mean, it's a singleton, I don't have to care anyways.
}

StringName AnaglyphBusManager::get_anaglyph_bus(StringName base_bus) {
	// Some fancy lending logic here at some point that also uses anaglyph_buses
	StringName name = StringName(a_bus_name);
	int index = guarantee_bus(name);

	// Reroute it into the base bus
	audio->set_bus_send(index, base_bus);
	return name;
}

StringName AnaglyphBusManager::get_silent_bus() {
	StringName name = StringName(s_bus_name);
	int index = guarantee_bus(name);
	audio->set_bus_mute(index, true);
	return s_bus_name;
}
