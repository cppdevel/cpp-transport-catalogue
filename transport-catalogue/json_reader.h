#pragma once

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <map>
#include <sstream>

namespace json_reader {

	class JsonReader {
	public:
		JsonReader(std::istream& input)
			: document_(json::Load(input))
		{
		}

		json::Node GetBaseRequests() const;
		json::Node GetRenderSettings() const;
		json::Node GetStatRequests() const;
		json::Node GetRoutingSettings() const;

		request_handler::StopStat GetStopFromRequest(const json::Dict& dict) const;
		std::vector<domain::Distance> GetDistancesFromRequest(const request_handler::StopStat& stop_stat, transport_catalogue::TransportCatalogue& catalogue) const;
		request_handler::BusStat GetBusFromRequest(const json::Dict& dict) const;

		map_renderer::MapRenderer GetMapRenderer(const json::Dict& dict) const;

		void FillTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue);

		json::Node BuildStopRequest(const json::Dict& dict, const transport_catalogue::TransportCatalogue& catalogue) const;
		json::Node BuildBusRequest(const json::Dict& dict, const transport_catalogue::TransportCatalogue& catalogue) const;
		json::Node BuildMapRequest(const json::Dict& dict, const transport_catalogue::TransportCatalogue& catalogue, const map_renderer::MapRenderer& map_renderer) const;
		json::Node BuildRouteRequest(const json::Dict& dict, const transport_router::TransportRouter& transport_router) const;

		void PrintStat(const transport_catalogue::TransportCatalogue& catalogue) const;

	private:
		json::Document document_;
	};

} // namespace json_reader