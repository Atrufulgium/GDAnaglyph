## A base class for random distributions. This does not do anything on its own,
## use one of the child classes.
##
## A distribution can sample an arbitrary type, based on the given
## [code]pdf[/code] and [code]sample[/code] functions. The random state can be
## controlled with [code]rng[/code]. By default, the rng starts as random.
class_name Distribution

var rng : RandomNumberGenerator

## The probability density function of this distribution. Loosely speaking,
## this is the "chance" that [code]x[/code] gets chosen.
func pdf(_x) -> float:
	push_error("You are calling Distribution.pdf(), and not using a derived type. This does not do anything.")
	return NAN

## Samples a value according to the distribution. If you sample, for instance,
## a million values, the resulting graph will look quite closely like
## [method pdf].
func sample() -> Variant:
	push_error("You are calling Distribution.sample(), and not using a derived type. This does not do anything.")
	return null

func _init(p_rng : RandomNumberGenerator = null) -> void:
	if p_rng == null:
		rng = RandomNumberGenerator.new()
	else:
		rng = p_rng

## A base class for random numeric distributions. This does not do anything on
## its own, use one of the child classes.
##
## A numeric distribution can sample floats, based on the given distribution
## information contained in its [method Distribution.pdf].
class NumericDistribution extends Distribution:
	
	## Must be implemented by extending classes.
	func pdf(_x : float) -> float:
		push_error("You are calling NumericDistribution.pdf(), and not using a derived type. This does not do anything.")
		return NAN
	
	func sample() -> float:
		# Fallback implementation: do some N-R. Not very efficient...
		# This relies on `mean` and `variance` being finite, and `cdf` existing.
		var r = rng.randf()
		var x = mean()
		var c = cdf(x)
		var p = pdf(x)
		if not (is_finite(x) and is_finite(c) and is_finite(p)):
			return NAN
		for i in 25:
			if p != 0 and abs(c-r) > 0.001:
				break
			x -= (c - r) / p
			c = cdf(x)
			p = pdf(x)
		return x
			
	
	## The cumulative density function. Loosely, the chance that [code]x[/code],
	## or a value lower than [code]x[/code] will be sampled.
	func cdf(x : float):
		# Fallback implementation: just add up a range "left up until x".
		# Start enough standard deviations to the left
		var ξ = mean() - 4 * sqrt(variance())
		var δ = (x - ξ) * 0.0001;
		# Trapezoid on [ξ_i, ξ_{i+1}]
		var sum = pdf(ξ) + pdf(x);
		ξ += δ
		while ξ <= x - δ:
			@warning_ignore("shadowed_variable")
			var sample = pdf(ξ)
			if is_finite(sample):
				sum += 2 * sample
			ξ += δ
		return sum * δ + 0.5
	
	## The average sampled value. Must be implemented by extended classes. [br]
	## This is not necessarily finite.
	func mean() -> float:
		push_error("You are calling NumericDistribution.mean(), and not using a derived type. This does not do anything.")
		return NAN
	
	## The mathematical variance of the values. This is the average of the
	## (squared) distance to the mean of samples. Must be implemented by
	## extended classes. [br]
	## This it not necessarily finite.
	func variance() -> float:
		push_error("You are calling NumericDistribution.variance(), and not using a derived type. This does not do anything.")
		return NAN

## An exp(λ)-distribution.
class Exponential extends NumericDistribution:
	
	var lambda : float
	
	func _init(λ : float = 1.0, p_rng : RandomNumberGenerator = null):
		super(p_rng)
		if λ <= 0:
			λ = 0.01
		lambda = λ
	
	func pdf(x : float) -> float:
		if x < 0:
			return 0
		return lambda * exp(-lambda * x)
	
	func cdf(x : float) -> float:
		if x < 0:
			return 0
		return 1 - exp(-lambda * x)
	
	func sample() -> float:
		var u = rng.randf()
		return -log(1 - u) / lambda
	
	func mean() -> float:
		return 1/lambda
	
	func variance() -> float:
		return 1/(lambda*lambda)

## Funky little distribution that's quite useful. It is quite similar to a lot
## of distributions that do not have a closed form.
## See [url=https://en.wikipedia.org/wiki/Kumaraswamy_distribution#Related_distributions]wikipedia[/url]
## for more info.
class Kuramaswamy extends NumericDistribution:
	
	var a : float
	var b : float
	
	func _init(p_a : float = 1.0, p_b : float = 1.0, p_rng : RandomNumberGenerator = null):
		super(p_rng)
		if p_a <= 0:
			p_a = 0.01
		if p_b <= 0:
			p_b = 0.01
		a = p_a
		b = p_b
	
	func pdf(x : float) -> float:
		if x < 0 or x > 1:
			return 0
		return a * b * pow(x, a-1) * pow(1 - pow(x,a), b-1)
	
	func cdf(x : float) -> float:
		if x < 0: return 0
		if x > 1: return 1
		return 1 - pow(1 - pow(x,a), b)
	
	func sample() -> float:
		# See https://rdrr.io/github/wasquith/lmomco/man/quakur.html
		var u = rng.randf()
		return pow(1 - pow(1-u, 1/b), 1/a)
	
	func mean() -> float:
		return moment(1)
	
	func variance() -> float:
		var m = moment(1)
		return moment(2) - m * m
	
	func moment(n : int) -> float:
		return (b * MathUtility.gamma(1 + n / a)
			* MathUtility.gamma(b)
			/ MathUtility.gamma(1 + b + n/a))

## The good 'ol normal distribution we all know and love.
class Normal extends NumericDistribution:
	
	# (Just use mean() and variance() lol)
	var μ : float
	var σ : float
	
	@warning_ignore("shadowed_variable")
	func _init(mean : float = 0.0, sigma : float = 1.0, p_rng : RandomNumberGenerator = null):
		super(p_rng)
		μ = mean
		σ = sigma
	
	func pdf(x : float) -> float:
		var Var = σ**2
		var scaling = 1.0 / sqrt(2 * Var * PI)
		return scaling * exp(-(x - μ)**2 / (2 * Var))
	
	# Infamously no cdf :( just hav'ta use the ugly fallback
	
	func sample() -> float:
		# godot does this for us how kind <3
		# not having to waste keystrokes B-M'ing myself
		return rng.randfn(μ, σ)
	
	func mean() -> float:
		return μ
	
	func variance() -> float:
		return σ**2

## A random number generator on [bl]lower,upper[br].
class Uniform extends NumericDistribution:
	
	var lower : float
	var upper : float
	
	func _init(p_lower : float = 0.0, p_upper : float = 1.0, p_rng : RandomNumberGenerator = null):
		super(p_rng)
		lower = p_lower
		upper = p_upper
	
	func pdf(x : float) -> float:
		if x < lower or x > upper:
			return 0
		return 1 / (upper - lower)
	
	func cdf(x : float) -> float:
		return clamp((x - lower) / (upper - lower), 0, 1)
	
	func sample() -> float:
		return rng.randf() * (upper - lower) + lower
	
	func mean() -> float:
		return (upper + lower) * 0.5
	
	func variance() -> float:
		return (upper - lower)**2 / 12

## A random Vector2 on the edge of a circle.
class UniformCircle extends Distribution:
	
	var radius : float
	
	func _init(p_radius : float, p_rng : RandomNumberGenerator = null):
		super(p_rng)
		if p_radius < 0:
			p_radius = -p_radius
		radius = p_radius
	
	func pdf(x : Vector2) -> float:
		x /= radius
		if abs(x.normalized() - x).length_squared() > 1e-3:
			return 0
		return 1.0 / (2 * PI * radius)
	
	func sample() -> Vector2:
		var angle = 2 * PI * rng.randf()
		return Vector2(cos(angle), sin(angle)) * radius

## A random Vector2 on the edge of, or inside a circle.
class UniformDisk extends Distribution:
	
	var radius : float
	
	func _init(p_radius : float, p_rng : RandomNumberGenerator = null):
		super(p_rng)
		if p_radius < 0:
			p_radius = -p_radius
		radius = p_radius
	
	func pdf(x : Vector2) -> float:
		x /= radius
		if x.length_squared() > 1.001:
			return 0;
		return 1.0 / (PI * radius**2)
	
	func sample() -> Vector2:
		var angle = 2 * PI * rng.randf()
		var r = sqrt(rng.randf()) * radius
		return Vector2(cos(angle), sin(angle)) * r

## Represents a uniform sample on some triangle in 3D space
class UniformTriangle extends Distribution:
	
	var triangle : Triangle
	
	func _init(p_triangle : Triangle, p_rng : RandomNumberGenerator = null):
		super(p_rng)
		triangle = p_triangle
	
	func pdf(_x : Vector3) -> float:
		# TODO: Care about this
		push_error("The pdf of UniformTriangle is not implemented yet.")
		return NAN
	
	func sample() -> Vector3:
		# Just pick random barycentric coordinates that flip back into the
		# triangle when ending up outside. One of them is fixed 0.
		# Sketch it out, it works.
		var u1 = rng.randf()
		var u2 = rng.randf()
		if u1 + u2 > 1:
			u1 = 1 - u1
			u2 = 1 - u2
		return triangle._a + u1 * triangle._b_minus_a + u2 * triangle._c_minus_a

## Represents a uniform sample from the boundary of a mesh, where sampled points
## lie on a triangle on the mesh.
##
## Uses a mesh to sample random points. It is best to keep the mesh relatively
## simple -- chances are, if you're using RNG, you don't need [i]that[/i] much
## precision anyways. [br]
## Note that while every surface point has an equal chance of being chosen, if
## there is simply more surface near some point (e.g. because of folds), the net
## effect is that there will be more samples there, than elsewhere.
class UniformMeshBoundary extends Distribution:
	
	var _weights : PackedFloat32Array
	var _faces : PackedVector3Array
	
	func _init(mesh : Mesh, p_rng : RandomNumberGenerator = null) -> void:
		if mesh == null:
			push_error("Invalid UniformMeshBoundary -- given mesh was null.")
			return
		super(p_rng)
		
		_faces = mesh.get_faces()
		var tri_count : int = len(_faces) / 3
		_weights = PackedFloat32Array()
		_weights.resize(tri_count)
		for i in tri_count:
			var a = _faces[3*i]
			var b = _faces[3*i + 1]
			var c = _faces[3*i + 2]
			_weights[i] = Triangle.new(a, b, c).area()
	
	func pdf(_x : Vector3) -> float:
		# TODO: Care about this even more than [method UniformTriangle.pdf].
		# Checking whether a point is very close to a triangle is collision
		# detection I don't feel like doing. EIther way, once I do feel like
		# that, return 𝟙(x∈∂mesh)/total_area with a small margin for float
		# shenanigans. Yes yes ik ik, this is a measure zero set, but I can't
		# just define a method that takes as input a Vector3 on the boundary
		# of the manifold now, can I?
		push_error("The pdf of UniformMeshBoundary is not implemented yet.")
		return NAN
	
	func sample() -> Vector3:
		var i = rng.rand_weighted(_weights)
		var a = _faces[3*i]
		var b = _faces[3*i + 1]
		var c = _faces[3*i + 2]
		var tri = Triangle.new(a,b,c)
		return Distribution.UniformTriangle.new(tri, rng).sample()

## Represents a uniform sample from the inside of a mesh.
##
## Uses a mesh to sample random points. This mesh must be a manifold mesh (in
## the 3d modeling sense, not the mathematical sense), as otherwise results will
## be unpredictable. This is not checked. [br]
## This works best if the mesh is quite voluminous compared to its AABB, as this
## is simply implemented with rejection sampling. [br] 
## [b]Note[/b]: This method is very lazy, and when sampling, compares to
## [i]every[/i] other triangle. For this reason, only use this on meshes that
## aren't too complex. Chances are, if you're using RNG, you don't need
## [i]that[/i] much precision anyways.
class UniformMeshVolume extends Distribution:
	
	var _mesh : Mesh
	
	func _init(mesh : Mesh, p_rng : RandomNumberGenerator = null):
		if mesh == null:
			push_error("Invalid UniformMeshBoundary -- given mesh was null.")
			return
		super(p_rng)
		_mesh = mesh
	
	func pdf(_x : Vector3) -> float:
		# Haaaahahaha this is computationally difficult, even if your mesh is
		# convex. The classic "yeah  estimate it up to ε in O(1/ε) time is the
		# best math can do" type dealio.
		# TODO: If I care, implement that still.
		push_error("The pdf of UniformMeshVolume is not implemented yet.")
		return NAN
	
	func sample() -> Vector3:
		# 99 attempts of rejection sampling
		# We are on the inside if we cross an odd amount of tris when moving up
		var aabb = _mesh.get_aabb()
		var low = aabb.position
		var high = aabb.position + aabb.size
		
		var faces = _mesh.get_faces()
		var tri_count = len(faces) / 3
		
		for attempt in 99:
			var point = Vector3(
				Distribution.Uniform.new(low.x, high.x, rng).sample(),
				Distribution.Uniform.new(low.y, high.y, rng).sample(),
				Distribution.Uniform.new(low.z, high.z, rng).sample()
			)
			var intersections = 0
			
			for i in tri_count:
				var a = faces[3*i]
				var b = faces[3*i + 1]
				var c = faces[3*i + 2]
				if Triangle.new(a, b, c).point_lies_below(point):
					intersections += 1
			
			if intersections % 2 == 1:
				return point
		
		push_error("Either you're *very* unlucky, or the mesh you're trying to sample is by far not voluminous enough.")
		return Vector3(NAN, NAN, NAN)
