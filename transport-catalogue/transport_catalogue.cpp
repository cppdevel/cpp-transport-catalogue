#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddStop(std::string_view stop_name, geo::Coordinates coordinates) {
		domain::Stop stop;
		stop.name = stop_name;
		stop.coordinates = coordinates;
		stops_.push_back(std::move(stop));
		stopname_to_stop_.insert({ stops_.back().name, &stops_.back() });
	}

	void TransportCatalogue::AddStopToBus(const std::vector<std::string_view>& stops_from_request, std::string_view bus_name) {
		for (auto stop : stops_from_request) {
			auto bus = FindBus(bus_name);
			bus->stops.push_back(stopname_to_stop_.at(stop));
		}
	}

	void TransportCatalogue::AddBusToStop(const std::vector<std::string_view>& stops_from_request, std::string_view bus_name) {
		for (auto stop : stops_from_request) {
			stopname_to_stop_.at(stop)->buses.push_back(FindBus(bus_name));
		}
	}

	domain::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
		if (stopname_to_stop_.empty() || stopname_to_stop_.count(stop_name) == 0) {
			return nullptr;
		}
		return stopname_to_stop_.at(stop_name);
	}

	std::map<std::string_view, domain::Stop*> TransportCatalogue::GetSortedStops() const {
		std::map<std::string_view, domain::Stop*> sorted_stops;
		for (auto& stop : stopname_to_stop_) {
			sorted_stops.emplace(stop);
		}
		return sorted_stops;
	}

	void TransportCatalogue::SetDistance(std::vector<domain::Distance> distances_from_request) {
		for (auto dist : distances_from_request) {
			distances_[std::make_pair(dist.stop_from, dist.stop_to)] = dist.distance;
		}
	}

	int TransportCatalogue::GetDistance(const domain::Stop* stop_from, const domain::Stop* stop_to) const {
		if (distances_.count({ stop_from, stop_to }) > 0) {
			return distances_.at(std::make_pair(stop_from, stop_to));
		}
		if (distances_.count({ stop_to, stop_from }) > 0) {
			return distances_.at(std::make_pair(stop_to, stop_from));
		}
		return 0;
	}

	void TransportCatalogue::AddBus(std::string_view bus_name, domain::BusType bus_type_from_request) {
		domain::Bus bus;
		bus.name = bus_name;
		if (bus_type_from_request == domain::BusType::CIRCULAR) {
			bus.bus_type = domain::BusType::CIRCULAR;
		}
		else {
			bus.bus_type = domain::BusType::LINEAR;
		}
		buses_.push_back(std::move(bus));
		busname_to_bus_.insert({ buses_.back().name, &buses_.back() });
	}

	domain::Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
		if (busname_to_bus_.empty() || busname_to_bus_.count(bus_name) == 0) {
			return nullptr;
		}
		return busname_to_bus_.at(bus_name);
	}

	std::map<std::string_view, domain::Bus*> TransportCatalogue::GetSortedBuses() const {
		std::map<std::string_view, domain::Bus*> sorted_buses;
		for (auto& bus : busname_to_bus_) {
			sorted_buses.emplace(bus);
		}
		return sorted_buses;
	}

	namespace detail {

		std::vector<std::string> GetSortedUniqueBuses(const domain::Stop* stop) {
			std::unordered_set<domain::Bus*> unique_buses_from_stop;
			unique_buses_from_stop.insert(stop->buses.begin(), stop->buses.end());
			std::vector<std::string> sorted_unique_buses;
			for (auto bus : unique_buses_from_stop) {
				sorted_unique_buses.push_back(bus->name);
			}
			std::sort(sorted_unique_buses.begin(), sorted_unique_buses.end());
			return sorted_unique_buses;
		}

		int CalculateStops(const domain::Bus* bus) {
			return static_cast<int>(bus->stops.size());
		}

		int CalculateUniqueStops(const domain::Bus* bus) {
			std::unordered_set<domain::Stop*> unique_stops_set;
			unique_stops_set.insert(bus->stops.begin(), bus->stops.end());
			return static_cast<int>(unique_stops_set.size());
		}

		double CalculateRouteGeographicalLength(const domain::Bus* bus) {
			double geographical_length = 0.0;
			for (uint64_t i = 1; i < bus->stops.size(); ++i) {
				auto left_stop = bus->stops[i - 1], right_stop = bus->stops[i];
				geographical_length += geo::ComputeDistance(left_stop->coordinates, right_stop->coordinates);
			}
			return geographical_length;
		}

		int CalculateRouteRoadLength(const TransportCatalogue& catalogue, const domain::Bus* bus) {
			int road_length = 0;
			for (uint64_t i = 0; i < bus->stops.size() - 1; ++i) {
				road_length += catalogue.GetDistance(bus->stops[i], bus->stops[i + 1]);
			}
			return road_length;
		}

		double CalculateRouteCurvature(const TransportCatalogue& catalogue, const domain::Bus* bus) {
			return static_cast<double>(CalculateRouteRoadLength(catalogue, bus) / CalculateRouteGeographicalLength(bus));
		}

	}

} // namespace transport_catalogue