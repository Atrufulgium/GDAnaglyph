## Represents a [url=https://en.wikipedia.org/wiki/Poisson_point_process]Poisson process[/url]
##
## This is a process that randomly decides that an event happens. This has a few nice properties:
## - Given that an event happened in some timeframe, [i]when[/i] it happened will be uniformly
##   random on that timeframe.
## - The amount of time between events is completely independent from each other.
## - The amount of events per interval is fairly predictable and can be configured.
class_name PoissonProcess

var inter_arrival_distr : Distribution.Exponential
var seconds_to_next : float

func _init(avg_events_per_minute : float, rng : RandomNumberGenerator = null) -> void:
	if avg_events_per_minute <= 0:
		avg_events_per_minute = 0.01
	inter_arrival_distr = Distribution.Exponential.new(avg_events_per_minute / 60, rng)
	seconds_to_next = inter_arrival_distr.sample()

## Advance the internal clock of this Poisson process by [code]seconds[/code].
## This will return how many events happened in that timeframe.
func advance_time(seconds : float) -> int:
	if seconds < 0:
		seconds = 0
	var events : int = 0
	seconds_to_next -= seconds
	while seconds_to_next <= 0:
		events += 1
		seconds_to_next += inter_arrival_distr.sample()
	return events
