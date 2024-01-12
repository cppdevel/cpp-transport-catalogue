#pragma once

#include <algorithm>
#include <deque>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

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

	class TransportCatalogue {
	public:
		void AddStop(std::string_view stop_name, geo::Coordinates coordinates);
		void AddStopToBus(std::vector<std::string_view> stops_from_request, std::string_view bus_name);
		void AddBusToStop(std::vector<std::string_view> stops_from_request, std::string_view bus_name);
		Stop* FindStop(std::string_view stop_name) const;

		void AddBus(std::string_view bus_name, BusType bus_type);
		Bus* FindBus(std::string_view bus_name) const;

	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;

		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
	};

	namespace detail {

		std::vector<std::string> GetSortedUniqueBuses(Stop* stop);
		int CalculateStops(Bus* bus);
		int CalculateUniqueStops(Bus* bus);
		double CalculateRouteLength(Bus* bus);

	}

}