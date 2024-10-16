## Represents a triangle in 3D space.
class_name Triangle

var _a : Vector3
var _b_minus_a : Vector3
var _c_minus_a : Vector3

func _init(a : Vector3, b : Vector3, c : Vector3):
	_a = a
	_b_minus_a = b - a;
	_c_minus_a = c - a;

func area() -> float:
	return _b_minus_a.cross(_c_minus_a).length() * 0.5

## Whether or not a given point would hit this triangle when moving straight
## up (i.e. whether point.y lies below this triangle).
func point_lies_below(point : Vector3) -> bool:
	# TODO: Projecting down by ignoring y may introduce degenerate tris.
	# Do I care?
	
	var p_minus_a : Vector3 = point - _a
	
	var b_minus_a2 = Vector2(_b_minus_a.x, _b_minus_a.z)
	var c_minus_a2 = Vector2(_c_minus_a.x, _c_minus_a.z)
	var p_minus_a2 = Vector2(p_minus_a.x, p_minus_a.z)
	
	# Bary coords
	var dot_bb = b_minus_a2.dot(b_minus_a2)
	var dot_bc = b_minus_a2.dot(c_minus_a2)
	var dot_bp = b_minus_a2.dot(p_minus_a2)
	var dot_cc = c_minus_a2.dot(c_minus_a2)
	var dot_cp = c_minus_a2.dot(p_minus_a2)
	
	var denom = 1.0 / (dot_bb * dot_cc - dot_bc * dot_bc)
	var γ = (dot_bb * dot_cp - dot_bc * dot_bp) * denom
	var β = (dot_cc * dot_bp - dot_bc * dot_cp) * denom
	var α = 1 - β - γ
	
	# First check whether entirely outside triangle looking from above
	if α < 0 or β < 0 or γ < 0:
		return false
	
	var projection = (
		  α * _a
		+ β * (_b_minus_a + _a)
		+ γ * (_c_minus_a + _a)
	)
	return projection.z <= point.z
