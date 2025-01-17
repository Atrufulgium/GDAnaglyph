#include "anaglyph_bus_manager.h"
#include "helpers.h"

using namespace godot;

AnaglyphBusManager* AnaglyphBusManager::singleton = nullptr;
char* AnaglyphBusManager::a_bus_name = "[Anaglyph_Bus]";
char* AnaglyphBusManager::s_bus_name = "[Silent_Bus]";

int AnaglyphBusManager::total_bus_count() const {
	return anaglyph_buses.size() + used_anaglyph_buses;
}

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
	AnaglyphHelpers::print("Added Anaglyph audio bus ", name);
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
			AnaglyphHelpers::print_error("Couldn't allocate bus manager, expect a crash *very* soon.");
		}
	}
	return singleton;
}

AnaglyphBusManager::AnaglyphBusManager() {
	anaglyph_buses = Vector<StringName>();
	audio = AudioServer::get_singleton();
	used_anaglyph_buses = 0;
	max_anaglyph_buses = 4;
}

AnaglyphBusManager::~AnaglyphBusManager() {
	// I don't *think* have to free Vectors?
	// I mean, it's a singleton, I don't have to care anyways.
}

void AnaglyphBusManager::prepare_anaglyph_buses(int count) {
	int maximum_added = max_anaglyph_buses - total_bus_count();
	if (count < 0)
		count = 0;
	if (count > maximum_added)
		count = maximum_added;

	for (int i = 0; i < count; i++) {
		anaglyph_buses.push_back(add_bus(StringName(a_bus_name)));
		// Preparation is pretty lazy and only happens on borrow.
		// Force this by borrowing what we just created.
		// (Yeah ik creating refs in a loop like this is suboptimal, but this
		//  loop iterates at most, like, 3 times.)
		Ref<AnaglyphEffectData> placeholder_data = memnew(AnaglyphEffectData);
		Ref<AnaglyphEffect> placeholder_effect = nullptr;
		StringName borrow = borrow_anaglyph_bus("Master", placeholder_data, placeholder_effect);
		return_anaglyph_bus(borrow);
	}
}

StringName AnaglyphBusManager::borrow_anaglyph_bus(
	const StringName& base_bus,
	const Ref<AnaglyphEffectData>& anaglyph_data,
	Ref<AnaglyphEffect> &out_effect
) {
	// Grab or create a bus.
	// Grabbing may fail if AudioServer::set_bus_layout did a thing.
	StringName name;
	int index = -1;
	if (anaglyph_buses.size() > 0) {
		int last = anaglyph_buses.size() - 1;
		name = anaglyph_buses.get(last);
		index = get_bus_index(name);
		if (index == -1) {
			// We couldn't find the bus, yet it existed in the vector.
			// This means AudioServer::set_bus_layout did a thing.
			// All of our storage is invalidated and we need to
			// restart from scratch.
			anaglyph_buses.clear();
		}
		else {
			anaglyph_buses.remove_at(last);
		}
	}
	if (index == -1) {
		// Either the pool was empty or everything was invalidated.
		// Either way, grab a new bus if it doesn't push us past the limit.

		if (total_bus_count() < max_anaglyph_buses) {
			name = add_bus(StringName(a_bus_name));
			index = get_bus_index(name);
		}
		else {
			out_effect = Ref<AnaglyphEffect>(nullptr);
			return base_bus;
		}
	}

	// We added a new Anaglyph bus, so add to the active count
	used_anaglyph_buses++;

	// Set the anaglyph data.
	if (audio->get_bus_effect_count(index) == 0) {
		Ref<AnaglyphEffect> effect = memnew(AnaglyphEffect);
		effect->set_effect_data(anaglyph_data);
		audio->add_bus_effect(index, effect);
		out_effect = effect;
	}
	else {
		Ref<AnaglyphEffect> effect = audio->get_bus_effect(index, 0);
		if (effect == nullptr) {
			AnaglyphHelpers::print_error("Internal Anaglyph busses have been messed with... Uhh... Don't do that.");
		}
		else {
			effect->set_effect_data(anaglyph_data);
		}
		out_effect = effect;
	}

	// Reroute it into the base bus
	audio->set_bus_send(index, base_bus);
	return name;
}

void AnaglyphBusManager::return_anaglyph_bus(const StringName& anaglyph_bus) {
	// We're assuming proper input.
	// Just return it to the list if the list isn't too full.
	// Otherwise, delete the bus instead.
	// (That may happen if the user reduces max_anaglyph_buses during runtime.)
	used_anaglyph_buses--;
	bool push = total_bus_count() < max_anaglyph_buses;
	if (push) {
		// Source: returns `true` on failure.
		push &= !anaglyph_buses.push_back(anaglyph_bus);
	}
	if (!push) {
		int index = get_bus_index(anaglyph_bus);
		if (index >= 0) {
			audio->remove_bus(index);
			AnaglyphHelpers::print("Removed Anaglyph audio bus ", anaglyph_bus);
		}
	}
}

StringName AnaglyphBusManager::get_silent_bus() {
	StringName name = StringName(s_bus_name);
	int index = guarantee_bus(name);
	audio->set_bus_mute(index, true);
	return s_bus_name;
}

void AnaglyphBusManager::set_max_anaglyph_buses(int max) {
	// By default, we won't prepare the new space, so we only need to do
	// anything when we decrease size.
	// When decreasing, we need to ensure
	//   anaglyph_buses.size() + used_anaglyph_buses < max
	// We can only change the former.
	// (Note that this is always true when increasing.)
	if (total_bus_count() >= max) {
		int new_size = max - used_anaglyph_buses;
		if (new_size < 0) {
			new_size = 0;
		}
		int delta = anaglyph_buses.size() - new_size;
		if (delta > 0) {
			AnaglyphHelpers::print("Removed ", delta, " Anaglyph audio buses");
		}
		anaglyph_buses.resize(new_size);
	}
	max_anaglyph_buses = max;
}

int AnaglyphBusManager::get_max_anaglyph_buses() {
	return max_anaglyph_buses;
}