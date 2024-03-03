#include "map_renderer.h"

namespace map_renderer {

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	std::vector<svg::Polyline> MapRenderer::DrawRoute(const std::map<std::string_view, domain::Bus*>& buses, SphereProjector& sphere_projector) const {
		std::vector<svg::Polyline> polylines;
		uint64_t color_palette_num = 0;
		for (auto& [bus_name, bus_ptr] : buses) {
			if (bus_ptr->stops.size() == 0) {
				continue;
			}
			svg::Polyline polyline;
			for (auto& stop : bus_ptr->stops) {
				polyline.AddPoint(sphere_projector(stop->coordinates));
			}
			polyline.SetFillColor("none");
			polyline.SetStrokeColor(render_settings_.color_palette[color_palette_num]);
			if (color_palette_num >= render_settings_.color_palette.size() - 1) {
				color_palette_num = 0;
			}
			else {
				++color_palette_num;
			}
			polyline.SetStrokeWidth(render_settings_.line_width);
			polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			polylines.push_back(std::move(polyline));
		}
		return polylines;
	}

	std::vector<svg::Text> MapRenderer::DrawBusName(const std::map<std::string_view, domain::Bus*>& buses, SphereProjector& sphere_projector) const {
		std::vector<svg::Text> texts;
		uint64_t color_palette_num = 0;
		for (auto& [bus_name, bus_ptr] : buses) {
			if (bus_ptr->stops.size() == 0) {
				continue;
			}
			svg::Text substrate;
			substrate.SetPosition(sphere_projector(bus_ptr->stops[0]->coordinates));
			substrate.SetOffset(render_settings_.bus_label_offset);
			substrate.SetFontSize(render_settings_.bus_label_font_size);
			substrate.SetFontFamily("Verdana");
			substrate.SetFontWeight("bold");
			substrate.SetData(bus_ptr->name);
			substrate.SetFillColor(render_settings_.underlayer_color);
			substrate.SetStrokeColor(render_settings_.underlayer_color);
			substrate.SetStrokeWidth(render_settings_.underlayer_width);
			substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			texts.push_back(substrate);
			svg::Text text;
			text.SetPosition(sphere_projector(bus_ptr->stops[0]->coordinates));
			text.SetOffset(render_settings_.bus_label_offset);
			text.SetFontSize(render_settings_.bus_label_font_size);
			text.SetFontFamily("Verdana");
			text.SetFontWeight("bold");
			text.SetData(bus_ptr->name);
			text.SetFillColor(render_settings_.color_palette[color_palette_num]);
			if (color_palette_num >= render_settings_.color_palette.size() - 1) {
				color_palette_num = 0;
			}
			else {
				++color_palette_num;
			}
			texts.push_back(text);
			if (bus_ptr->bus_type == domain::BusType::ORDINARY && (bus_ptr->stops[0] != bus_ptr->stops[bus_ptr->stops.size() / 2])) {
				svg::Text second_substrate = substrate;
				second_substrate.SetPosition(sphere_projector(bus_ptr->stops[bus_ptr->stops.size() / 2]->coordinates));
				texts.push_back(second_substrate);
				svg::Text second_text = text;
				second_text.SetPosition(sphere_projector(bus_ptr->stops[bus_ptr->stops.size() / 2]->coordinates));
				texts.push_back(second_text);
			}
		}
		return texts;
	}

	std::vector<svg::Circle> MapRenderer::DrawStopCircle(const std::map<std::string_view, domain::Stop*>& stops, SphereProjector& sphere_projector) const {
		std::vector<svg::Circle> circles;
		for (auto& [stop_name, stop_ptr] : stops) {
			svg::Circle circle;
			circle.SetCenter(sphere_projector(stop_ptr->coordinates));
			circle.SetRadius(render_settings_.stop_radius);
			circle.SetFillColor("white");
			circles.push_back(std::move(circle));
		}
		return circles;
	}

	std::vector<svg::Text> MapRenderer::DrawStopName(const std::map<std::string_view, domain::Stop*>& stops, SphereProjector& sphere_projector) const {
		std::vector<svg::Text> texts;
		for (auto& [stop_name, stop_ptr] : stops) {
			svg::Text substrate;
			substrate.SetPosition(sphere_projector(stop_ptr->coordinates));
			substrate.SetOffset(render_settings_.stop_label_offset);
			substrate.SetFontSize(render_settings_.stop_label_font_size);
			substrate.SetFontFamily("Verdana");
			substrate.SetData(stop_ptr->name);
			substrate.SetFillColor(render_settings_.underlayer_color);
			substrate.SetStrokeColor(render_settings_.underlayer_color);
			substrate.SetStrokeWidth(render_settings_.underlayer_width);
			substrate.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			substrate.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			texts.push_back(std::move(substrate));
			svg::Text text;
			text.SetPosition(sphere_projector(stop_ptr->coordinates));
			text.SetOffset(render_settings_.stop_label_offset);
			text.SetFontSize(render_settings_.stop_label_font_size);;
			text.SetFontFamily("Verdana");
			text.SetData(stop_ptr->name);
			text.SetFillColor("black");
			texts.push_back(std::move(text));
		}
		return texts;
	}

	svg::Document MapRenderer::GetSvgDocument(const std::map<std::string_view, domain::Bus*>& buses) const {
		svg::Document document;
		std::vector<geo::Coordinates> coordinates;
		std::map<std::string_view, domain::Stop*> stops;
		for (auto& [bus_name, bus_ptr] : buses) {
			if (bus_ptr->stops.size() == 0) {
				continue;
			}
			for (auto& stop : bus_ptr->stops) {
				coordinates.push_back(stop->coordinates);
				stops[stop->name] = stop;
			}
		}
		SphereProjector sphere_projector(coordinates.begin(), coordinates.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
		for (auto& bus_line : DrawRoute(buses, sphere_projector)) {
			document.Add(bus_line);
		}
		for (auto& bus_name : DrawBusName(buses, sphere_projector)) {
			document.Add(bus_name);
		}
		for (auto& stop_circle : DrawStopCircle(stops, sphere_projector)) {
			document.Add(stop_circle);
		}
		for (auto& stop_name : DrawStopName(stops, sphere_projector)) {
			document.Add(stop_name);
		}
		return document;
	}

} // namespace map_renderer