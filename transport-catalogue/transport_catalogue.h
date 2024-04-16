#pragma once

#include "domain.h"
#include "geo.h"

#include <algorithm>
#include <deque>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport_catalogue {

	struct DistanceHasher {
		std::hash<const void*> ptr_hasher;
		size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*> stop_ptr_pair) const {
			return ptr_hasher(stop_ptr_pair.first) + ptr_hasher(stop_ptr_pair.second) * 12;
		}
	};

	class TransportCatalogue {
	public:
		void AddStop(std::string_view stop_name, geo::Coordinates coordinates);
		void AddStopToBus(const std::vector<std::string_view>& stops_from_request, std::string_view bus_name);
		void AddBusToStop(const std::vector<std::string_view>& stops_from_request, std::string_view bus_name);
		domain::Stop* FindStop(std::string_view stop_name) const;
		std::map<std::string_view, domain::Stop*> GetSortedStops() const;

		void SetDistance(std::vector<domain::Distance> distances_from_request);
		int GetDistance(const domain::Stop* stop_from, const domain::Stop* stop_to) const;

		void AddBus(std::string_view bus_name, domain::BusType bus_type);
		domain::Bus* FindBus(std::string_view bus_name) const;
		std::map<std::string_view, domain::Bus*> GetSortedBuses() const;

	private:
		std::deque<domain::Stop> stops_;
		std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_;

		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, DistanceHasher> distances_;

		std::deque<domain::Bus> buses_;
		std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;
	};

	namespace detail {

		std::vector<std::string> GetSortedUniqueBuses(const domain::Stop* stop);
		int CalculateStops(const domain::Bus* bus);
		int CalculateUniqueStops(const domain::Bus* bus);
		double CalculateRouteGeographicalLength(const domain::Bus* bus);
		int CalculateRouteRoadLength(const TransportCatalogue& catalogue, const domain::Bus* bus);
		double CalculateRouteCurvature(const TransportCatalogue& catalogue, const domain::Bus* bus);

	} // namespace detail

} // namespace transport_catalogue