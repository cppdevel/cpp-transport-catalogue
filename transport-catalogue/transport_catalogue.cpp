#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddStop(std::string_view stop_name, geo::Coordinates coordinates) {
		Stop stop;
		stop.name = stop_name;
		stop.coordinates = coordinates;
		stops_.push_back(std::move(stop));
		stopname_to_stop_.insert({ stops_.back().name, &stops_.back() });
	}

	void TransportCatalogue::AddStopToBus(std::vector<std::string_view> stops_from_request, std::string_view bus_name) {
		for (auto stop : stops_from_request) {
			auto bus = FindBus(bus_name);
			bus->stops.push_back(stopname_to_stop_.at(stop));
		}
	}

	void TransportCatalogue::AddBusToStop(std::vector<std::string_view> stops_from_request, std::string_view bus_name) {
		for (auto stop : stops_from_request) {
			stopname_to_stop_.at(stop)->buses.push_back(FindBus(bus_name));
		}
	}

	Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
		if (stopname_to_stop_.empty() || stopname_to_stop_.count(stop_name) == 0) {
			return nullptr;
		}
		return stopname_to_stop_.at(stop_name);
	}

	void TransportCatalogue::AddBus(std::string_view bus_name, BusType bus_type_from_request) {
		Bus bus;
		bus.name = bus_name;
		if (bus_type_from_request == BusType::CIRCULAR) {
			bus.bus_type = BusType::CIRCULAR;
		}
		else {
			bus.bus_type = BusType::ORDINARY;
		}
		buses_.push_back(std::move(bus));
		busname_to_bus_.insert({ buses_.back().name, &buses_.back() });
	}

	Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
		if (busname_to_bus_.empty() || busname_to_bus_.count(bus_name) == 0) {
			return nullptr;
		}
		return busname_to_bus_.at(bus_name);
	}

	namespace detail {

		std::vector<std::string> GetSortedUniqueBuses(Stop* stop) {
			std::unordered_set<Bus*> unique_buses_from_stop;
			unique_buses_from_stop.insert(stop->buses.begin(), stop->buses.end());
			std::vector<std::string> sorted_unique_buses;
			for (auto bus : unique_buses_from_stop) {
				sorted_unique_buses.push_back(bus->name);
			}
			std::sort(sorted_unique_buses.begin(), sorted_unique_buses.end());
			return sorted_unique_buses;
		}

		int CalculateStops(Bus* bus) {
			return static_cast<int>(bus->stops.size());
		}

		int CalculateUniqueStops(Bus* bus) {
			std::unordered_set<Stop*> unique_stops_set;
			unique_stops_set.insert(bus->stops.begin(), bus->stops.end());
			return static_cast<int>(unique_stops_set.size());
		}

		double CalculateRouteLength(Bus* bus) {
			double route_length = 0.0;
			for (auto i = 1; i < bus->stops.size(); ++i) {
				auto left_stop = bus->stops[i - 1], right_stop = bus->stops[i];
				route_length += geo::ComputeDistance(left_stop->coordinates, right_stop->coordinates);
			}
			return route_length;
		}

	}

}