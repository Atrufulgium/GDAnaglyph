#ifndef GDANAGLYPH_HELPERS
#define GDANAGLYPH_HELPERS

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

	// This class holds miscelanneous static helper methods.
	class AnaglyphHelpers {
	private:
		static String get_prefix() {
			return "[GDAnaglyph " + Time::get_singleton()->get_time_string_from_system() + "] ";
		}

	public:
		// Print debug information, only when --verbose
		template <typename... Args> static void print(const Args&... args) {
			UtilityFunctions::print_verbose(get_prefix(), args...);
		}

		template <typename... Args> static void print_warning(const Args&... args) {
			UtilityFunctions::push_warning(get_prefix(), args...);
		}

		template <typename... Args> static void print_error(const Args&... args) {
			UtilityFunctions::push_error(get_prefix(), args...);
		}

		// Returns in the Vector3 the azimuth [x], elevation [y], and distance [z]
		// so that their respective getters/setters can use them.
		static Vector3 calculate_polar_position(Node3D* audio_source, Node3D* audio_listener) {
			// World-space difference between the two sources
			Vector3 global_delta
				= audio_source->get_global_position()
				- audio_listener->get_global_position();

			// Rotate into camera... microphone?-space.
			// "Basis" is the 3x3 rotation/scaling part of the transform.
			// We only want to invert the rotational part, so grab the quaternion
			// separately and apply the inverse ("xform_inv"...?) to our global.
			// The docs note the quaternion must be normalized, but surely it's
			// normalized if the source is a transform?

			// Note that we also need to take into account different handedness.
			// This is effectively a flip in local space.
			// (idk i didn't think too long about this it *sounds*/behaves correctly)
			Quaternion quat = audio_listener->get_global_basis().get_rotation_quaternion();
			Vector3 relative_pos = quat.xform_inv(global_delta);
			relative_pos.z *= -1;

			// Now the usual "from cartesian to polar coords".
			float dist = relative_pos.length();
			if (dist < 0.001) {
				dist = 0.001;
			}
			float azim;
			if (relative_pos.x == 0 && relative_pos.z == 0) {
				azim = 0; // atan2 could get (0,0) which throws
			}
			else {
				azim = atan2f(relative_pos.x, relative_pos.z);
			}
			float elev = asinf(relative_pos.y / dist); // (arg guaranteed in [-1,1])
			const float rad2deg = 57.2957805;
			Vector3 res = Vector3(azim * rad2deg, elev * rad2deg, dist);
			return res;
		}
	};

}

#endif // GDANAGLYPH_HELPERS