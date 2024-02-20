#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {

	struct Bus;

	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
		std::vector<Bus*> buses;
	};

	enum class BusType {
		DEFAULT,
		CIRCULAR,
		ORDINARY
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
		BusType bus_type = BusType::DEFAULT;
	};

	struct Distance {
		Stop* stop_from;
		Stop* stop_to;
		int distance = 0;
	};

} // namespace domain