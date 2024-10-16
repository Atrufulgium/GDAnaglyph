## Contains some miscellaneous mathematical functions.
class_name MathUtility

## Computes log([url=https://en.wikipedia.org/wiki/Gamma_function]Γ(x)[/url]).
static func log_gamma(x : float) -> float:
	# From https://mrob.com/pub/ries/lanczos-gamma.html
	var coeffs = [
		1.000000000190015,
		76.18009172947146,
		-86.50532032941677,
		24.01409824083091,
		-1.231739572450155,
		0.001208650973866179,
		-0.5395239384953e-5
	]
	x -= 1
	var b = x + 5.5
	var sum = 0.0
	for i in range(7, 0, -1):
		sum += coeffs[i] / (x + i)
	sum += coeffs[0]
	return ((0.91893853320467274178 + log(sum)) - b) + log(b) * (x + 0.5)

## Computes [url=https://en.wikipedia.org/wiki/Gamma_function]Γ(x)[/url].
static func gamma(x : float) -> float:
	if x < 0.5:
		return PI / (sin(PI * x)) * gamma(1-x)
	return exp(log_gamma(x))
