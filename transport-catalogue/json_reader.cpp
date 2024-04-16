#include "json_reader.h"

namespace json_reader {

	json::Node JsonReader::GetBaseRequests() const {
		if (document_.GetRoot().AsMap().count("base_requests")) {
			return document_.GetRoot().AsMap().at("base_requests");
		}
		return document_.GetRoot();
	}

	json::Node JsonReader::GetRenderSettings() const {
		if (document_.GetRoot().AsMap().count("render_settings")) {
			return document_.GetRoot().AsMap().at("render_settings");
		}
		return document_.GetRoot();
	}

	json::Node JsonReader::GetStatRequests() const {
		if (document_.GetRoot().AsMap().count("stat_requests")) {
			return document_.GetRoot().AsMap().at("stat_requests");
		}
		return document_.GetRoot();
	}

	json::Node JsonReader::GetRoutingSettings() const {
		if (document_.GetRoot().AsMap().count("routing_settings")) {
			return document_.GetRoot().AsMap().at("routing_settings");
		}
		return document_.GetRoot();
	}

	request_handler::StopStat JsonReader::GetStopFromRequest(const json::Dict& dict) const {
		std::string_view stop_name = dict.at("name").AsString();
		geo::Coordinates coordinates = { dict.at("latitude").AsDouble(), dict.at("longitude").AsDouble() };
		std::map<std::string_view, int> road_distances;
		for (auto& [key, value] : dict.at("road_distances").AsMap()) {
			road_distances.emplace(key, value.AsInt());
		}
		return request_handler::StopStat{ stop_name, coordinates, road_distances };
	}

	std::vector<domain::Distance> JsonReader::GetDistancesFromRequest(const request_handler::StopStat& stop_stat, transport_catalogue::TransportCatalogue& catalogue) const {
		std::vector<domain::Distance> distances_to_get;
		if (!(stop_stat.road_distances.empty())) {
			for (auto& distance : stop_stat.road_distances) {
				domain::Distance dist{ catalogue.FindStop(stop_stat.name), catalogue.FindStop(distance.first), distance.second };
				distances_to_get.push_back(dist);
			}
		}
		return distances_to_get;
	}

	request_handler::BusStat JsonReader::GetBusFromRequest(const json::Dict& dict) const {
		std::string_view bus_name = dict.at("name").AsString();
		bool is_roundtrip = dict.at("is_roundtrip").AsBool();
		std::vector<std::string_view> stops;
		for (auto& stop : dict.at("stops").AsArray()) {
			stops.push_back(stop.AsString());
		}
		if (!is_roundtrip) {
			stops.insert(stops.end(), std::next(stops.rbegin()), stops.rend());
		}
		return request_handler::BusStat{ bus_name, stops, is_roundtrip };
	}

	map_renderer::MapRenderer JsonReader::GetMapRenderer(const json::Dict& dict) const {
		map_renderer::RenderSettings render_settings;
		render_settings.width = dict.at("width").AsDouble();
		render_settings.height = dict.at("height").AsDouble();
		render_settings.padding = dict.at("padding").AsDouble();
		render_settings.line_width = dict.at("line_width").AsDouble();
		render_settings.stop_radius = dict.at("stop_radius").AsDouble();
		render_settings.bus_label_font_size = dict.at("bus_label_font_size").AsInt();

		json::Array bus_label_offset = dict.at("bus_label_offset").AsArray();
		render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };

		render_settings.stop_label_font_size = dict.at("stop_label_font_size").AsInt();

		json::Array stop_label_offset = dict.at("stop_label_offset").AsArray();
		render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

		if (dict.at("underlayer_color").IsString()) {
			render_settings.underlayer_color = svg::Color(dict.at("underlayer_color").AsString());
		}
		if (dict.at("underlayer_color").IsArray()) {
			json::Array underlayer_colors = dict.at("underlayer_color").AsArray();
			if (underlayer_colors.size() == 3) {
				auto color = svg::Color(svg::Rgb(underlayer_colors[0].AsInt(), underlayer_colors[1].AsInt(), underlayer_colors[2].AsInt()));
				render_settings.underlayer_color = color;
			}
			if (underlayer_colors.size() == 4) {
				auto color = svg::Color(svg::Rgba(underlayer_colors[0].AsInt(), underlayer_colors[1].AsInt(), underlayer_colors[2].AsInt(), underlayer_colors[3].AsDouble()));
				render_settings.underlayer_color = color;
			}
		}

		render_settings.underlayer_width = dict.at("underlayer_width").AsDouble();

		json::Array color_palette = dict.at("color_palette").AsArray();
		for (auto color : color_palette) {
			if (color.IsString()) {
				render_settings.color_palette.emplace_back(svg::Color(color.AsString()));
			}
			if (color.IsArray()) {
				auto colors = color.AsArray();
				if (colors.size() == 3) {
					auto data = svg::Color(svg::Rgb(colors[0].AsInt(), colors[1].AsInt(), colors[2].AsInt()));
					render_settings.color_palette.emplace_back(data);
				}
				if (colors.size() == 4) {
					auto data = svg::Color(svg::Rgba(colors[0].AsInt(), colors[1].AsInt(), colors[2].AsInt(), colors[3].AsDouble()));
					render_settings.color_palette.emplace_back(data);
				}
			}
		}

		return map_renderer::MapRenderer(std::move(render_settings));
	}

	void JsonReader::FillTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
		json::Array base_requests = GetBaseRequests().AsArray();

		for (auto& request : base_requests) {
			if (request.AsMap().at("type").AsString() == "Stop") {
				request_handler::StopStat stop_to_add = GetStopFromRequest(request.AsMap());
				catalogue.AddStop(stop_to_add.name, stop_to_add.coordinates);
			}
		}
		for (auto& request : base_requests) {
			if (request.AsMap().at("type").AsString() == "Stop") {
				request_handler::StopStat stop_to_add = GetStopFromRequest(request.AsMap());
				catalogue.SetDistance(GetDistancesFromRequest(stop_to_add, catalogue));
			}
		}
		for (auto& request : base_requests) {
			if (request.AsMap().at("type").AsString() == "Bus") {
				request_handler::BusStat bus_to_add = GetBusFromRequest(request.AsMap());
				bus_to_add.is_roundtrip == true ? catalogue.AddBus(bus_to_add.name, domain::BusType::CIRCULAR) : catalogue.AddBus(bus_to_add.name, domain::BusType::LINEAR);
				catalogue.AddStopToBus(bus_to_add.stops, bus_to_add.name);
				catalogue.AddBusToStop(bus_to_add.stops, bus_to_add.name);
			}
		}
	}

	json::Node JsonReader::BuildStopRequest(const json::Dict& dict, const transport_catalogue::TransportCatalogue& catalogue) const {
		json::Node answer;
		int request_id = dict.at("id").AsInt();
		auto stop = catalogue.FindStop(dict.at("name").AsString());
		if (stop == nullptr) {
			answer =
				json::Builder{}
				.StartDict()
				.Key("error_message").Value("not found")
				.Key("request_id").Value(request_id)
				.EndDict()
				.Build();
		}
		else {
			json::Array buses;
			std::vector<std::string> unique_buses = transport_catalogue::detail::GetSortedUniqueBuses(stop);
			for (auto& bus_name : unique_buses) {
				buses.push_back(bus_name);
			}
			answer =
				json::Builder{}
				.StartDict()
				.Key("buses").Value(buses)
				.Key("request_id").Value(request_id)
				.EndDict()
				.Build();
		}
		return answer;
	}

	json::Node JsonReader::BuildBusRequest(const json::Dict& dict, const transport_catalogue::TransportCatalogue& catalogue) const {
		json::Node answer;
		auto request_id = dict.at("id").AsInt();
		auto bus = catalogue.FindBus(dict.at("name").AsString());
		if (bus == nullptr) {
			answer =
				json::Builder{}
				.StartDict()
				.Key("error_message").Value("not found")
				.Key("request_id").Value(request_id)
				.EndDict()
				.Build();
		}
		else {
			auto curvature = transport_catalogue::detail::CalculateRouteCurvature(catalogue, bus);
			auto route_length = transport_catalogue::detail::CalculateRouteRoadLength(catalogue, bus);
			auto stop_count = transport_catalogue::detail::CalculateStops(bus);
			auto unique_stop_count = transport_catalogue::detail::CalculateUniqueStops(bus);
			answer =
				json::Builder{}
				.StartDict()
				.Key("curvature").Value(curvature)
				.Key("route_length").Value(route_length)
				.Key("stop_count").Value(stop_count)
				.Key("unique_stop_count").Value(unique_stop_count)
				.Key("request_id").Value(request_id)
				.EndDict()
				.Build();
		}
		return answer;
	}

	json::Node JsonReader::BuildMapRequest(const json::Dict& dict, const transport_catalogue::TransportCatalogue& catalogue, const map_renderer::MapRenderer& map_renderer) const {
		json::Node answer;
		int request_id = dict.at("id").AsInt();
		request_handler::RequestHandler request_handler{ catalogue, map_renderer };
		svg::Document map = request_handler.RenderMap();
		std::ostringstream oss;
		map.Render(oss);
		answer =
			json::Builder{}
			.StartDict()
			.Key("request_id").Value(request_id)
			.Key("map").Value(oss.str())
			.EndDict()
			.Build();
		return answer;
	}

	json::Node JsonReader::BuildRouteRequest(const json::Dict& dict, const transport_router::TransportRouter& transport_router) const {
		json::Node answer;
		int request_id = dict.at("id").AsInt();
		std::string stop_from = dict.at("from").AsString();
		std::string stop_to = dict.at("to").AsString();
		auto optimal_route = transport_router.CalculateOptimalRoute(stop_from, stop_to);
		if (!optimal_route.has_value()) {
			answer =
				json::Builder{}
				.StartDict()
				.Key("request_id").Value(request_id)
				.Key("error_message").Value("not found")
				.EndDict()
				.Build();
		}
		else {
			json::Array items;
			const auto edges = transport_router.GetEdgesVector(optimal_route.value());
			double total_time = 0.0;
			for (auto& edge : edges) {
				total_time += edge.weight;
				if (edge.items_type == graph::ItemsType::WAIT) {
					items.emplace_back(json::Node(json::Builder{}
						.StartDict()
						.Key("stop_name").Value(edge.name)
						.Key("time").Value(edge.weight)
						.Key("type").Value("Wait")
						.EndDict()
						.Build()));
				}
				if (edge.items_type == graph::ItemsType::BUS) {
					items.emplace_back(json::Node(json::Builder{}
						.StartDict()
						.Key("bus").Value(edge.name)
						.Key("span_count").Value(edge.span_count)
						.Key("time").Value(edge.weight)
						.Key("type").Value("Bus")
						.EndDict()
						.Build()));
				}
			}
			answer = 
				json::Builder{}
				.StartDict()
				.Key("request_id").Value(request_id)
				.Key("total_time").Value(total_time)
				.Key("items").Value(items)
				.EndDict()
				.Build();
		}
		return answer;
	}

	void JsonReader::PrintStat(const transport_catalogue::TransportCatalogue& catalogue) const {

		transport_router::RoutingSettings routing_settings{ GetRoutingSettings().AsMap().at("bus_wait_time").AsInt(), GetRoutingSettings().AsMap().at("bus_velocity").AsDouble() };
		transport_router::TransportRouter transport_router(routing_settings, catalogue);

		json::Array stat_to_print;
		json::Array stat_requests = GetStatRequests().AsArray();
		for (auto& request : stat_requests) {
			if (request.AsMap().at("type").AsString() == "Stop") {
				stat_to_print.push_back(BuildStopRequest(request.AsMap(), catalogue).AsMap());
			}
			if (request.AsMap().at("type").AsString() == "Bus") {
				stat_to_print.push_back(BuildBusRequest(request.AsMap(), catalogue).AsMap());
			}
			if (request.AsMap().at("type").AsString() == "Map") {
				json::Dict render_settings = GetRenderSettings().AsMap();
				map_renderer::MapRenderer map_renderer = GetMapRenderer(render_settings);
				stat_to_print.push_back(BuildMapRequest(request.AsMap(), catalogue, map_renderer).AsMap());
			}
			if (request.AsMap().at("type").AsString() == "Route") {
				stat_to_print.push_back(BuildRouteRequest(request.AsMap(), transport_router).AsMap());
			}
		}
		json::Print(json::Document{ stat_to_print }, std::cout);
	}

} // namespace json_reader