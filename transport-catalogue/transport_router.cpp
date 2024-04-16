#include "transport_router.h"

namespace transport_router {

    void TransportRouter::FillGraphByStops() {
        const auto& stops = catalogue_.GetSortedStops();
        graph_ = graph::DirectedWeightedGraph<double>(stops.size() * 2);
        graph::VertexId vertex_id = 0;
        for (const auto& [stop_name, stop_ptr] : stops) {
            stops_id_[stop_ptr->name] = vertex_id;
            graph_.AddEdge({ vertex_id,
                ++vertex_id,
                static_cast<double>(routing_settings_.bus_wait_time),
                stop_ptr->name,
                0,
                graph::ItemsType::WAIT });
            ++vertex_id;
        }
    }

    void TransportRouter::FillGraphByBuses() {
        const double сoeff = 1000.0 / 60.0; // multiplication coefficient for converting the division result in minutes
        const auto& buses = catalogue_.GetSortedBuses();
        for (auto& [bus_name, bus_ptr] : buses) {
            size_t stops_count = bus_ptr->stops.size();
            for (int i = 0; i < stops_count; ++i) {
                double distance = 0.0;
                for (int j = i + 1; j < stops_count; ++j) {
                    const domain::Stop* stop_from = bus_ptr->stops[i];
                    const domain::Stop* stop_to = bus_ptr->stops[j];
                    distance += catalogue_.GetDistance(bus_ptr->stops[j - 1], stop_to);
                    graph_.AddEdge({ stops_id_.at(stop_from->name) + 1,
                        stops_id_.at(stop_to->name),
                        static_cast<double>(distance) / (routing_settings_.bus_velocity * сoeff),
                        bus_ptr->name,
                        j - i,
                        graph::ItemsType::BUS });
                }
            }
        }
    }

    void TransportRouter::CreateGraph() {
        FillGraphByStops();
        FillGraphByBuses();
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
        return graph_;
    }

    std::optional<graph::Router<double>::RouteInfo> TransportRouter::CalculateOptimalRoute(std::string_view stop_from, std::string_view stop_to) const {
        return router_->BuildRoute(stops_id_.at(stop_from), stops_id_.at(stop_to));
    }

} // namespace transport_router