#include "transport_router.h"

namespace transport_router {

    void TransportRouter::CreateGraph() {
        const auto& stops = catalogue_.GetSortedStops();
        graph::DirectedWeightedGraph<double> graph(stops.size() * 2);
        graph::VertexId vertex_id = 0;
        for (const auto& [stop_name, stop_ptr] : stops) {
            stops_id_[stop_ptr->name] = vertex_id;
            graph.AddEdge({ vertex_id,
                ++vertex_id,
                static_cast<double>(routing_settings_.bus_wait_time),
                stop_ptr->name,
                0,
                graph::ItemsType::WAIT });
            ++vertex_id;
        }
        const auto& buses = catalogue_.GetSortedBuses();
        for (auto& [bus_name, bus_ptr] : buses) {
            size_t stops_count = bus_ptr->stops.size();
            for (int i = 0; i < stops_count; ++i) {
                double distance = 0.0;
                for (int j = i + 1; j < stops_count; ++j) {
                    const domain::Stop* stop_from = bus_ptr->stops[i];
                    const domain::Stop* stop_to = bus_ptr->stops[j];
                    distance += catalogue_.GetDistance(bus_ptr->stops[j - 1], stop_to);
                    graph.AddEdge({ stops_id_.at(stop_from->name) + 1,
                        stops_id_.at(stop_to->name),
                        static_cast<double>(distance) / (routing_settings_.bus_velocity * (100.0 / 6.0)),
                        bus_ptr->name,
                        j - i,
                        graph::ItemsType::BUS });
                }
            }
        }
        graph_ = std::move(graph);
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    std::optional<graph::Router<double>::RouteInfo> TransportRouter::CalculateOptimalRoute(std::string_view stop_from, std::string_view stop_to) const {
        return router_->BuildRoute(stops_id_.at(stop_from), stops_id_.at(stop_to));
    }

    std::vector<graph::Edge<double>> TransportRouter::GetEdgesVector(graph::Router<double>::RouteInfo route_info) const {
        std::vector<graph::Edge<double>> edges;
        for (auto& edge_id : route_info.edges) {
            graph::Edge<double> edge = graph_.GetEdge(edge_id);
            edges.emplace_back(edge);
        }
        return edges;
    }

} // namespace transport_router