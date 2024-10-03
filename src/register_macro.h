#ifndef GDANAGLYPH_REGISTER_MACRO
#define GDANAGLYPH_REGISTER_MACRO

// Abuse how "adjacent " "strings" get concatenated.

#define STRINGIFY(text) #text

// This registration is so much typing, I hate it.
// To register e.g. `set_distance(const float arg)` in GDAnaglyph, you need to do:
// REGISTER(FLOAT, distance, GDAnaglyph, "godot_name", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m")
#define REGISTER(variant, prop, thistype, godotparam, hint_type, hint_str) \
	ClassDB::bind_method(D_METHOD("get_" STRINGIFY(prop)), &thistype::get_##prop); \
	ClassDB::bind_method(D_METHOD("set_" STRINGIFY(prop), godotparam), &thistype::set_##prop); \
	ADD_PROPERTY(PropertyInfo(Variant::##variant, STRINGIFY(prop), hint_type, hint_str), "set_" STRINGIFY(prop), "get_" STRINGIFY(prop));

// A variant of the REGISTER macro that also uses the PROPERTY_USAGE flags.
// To register e.g. `set_distance(const float arg)` in GDAnaglyph, you need to do:
// REGISTER(FLOAT, distance, GDAnaglyph, "godot_name", PROPERTY_HINT_RANGE, "0.1,10,0.1,suffix:m", PROPERTY_USAGE_DEFAULT)
#define REGISTER_USAGE(variant, prop, thistype, godotparam, hint_type, hint_str, usage) \
	ClassDB::bind_method(D_METHOD("get_" STRINGIFY(prop)), &thistype::get_##prop); \
	ClassDB::bind_method(D_METHOD("set_" STRINGIFY(prop), godotparam), &thistype::set_##prop); \
	ADD_PROPERTY(PropertyInfo(Variant::##variant, STRINGIFY(prop), hint_type, hint_str, usage), "set_" STRINGIFY(prop), "get_" STRINGIFY(prop));


#endif // GDANAGLYPH_REGISTER_MACRO