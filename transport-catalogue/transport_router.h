#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport_router {

    struct RoutingSettings {
        int bus_wait_time = 0;
        double bus_velocity = 0.0;
    };

    class TransportRouter {
    public:
        TransportRouter(RoutingSettings routing_settings, const transport_catalogue::TransportCatalogue& catalogue)
            : routing_settings_(routing_settings)
            , catalogue_(catalogue)
        {
            CreateGraph();
        }

        void CreateGraph();
        std::optional<graph::Router<double>::RouteInfo> CalculateOptimalRoute(std::string_view stop_from, std::string_view stop_to) const;
        std::vector<graph::Edge<double>> GetEdgesVector(graph::Router<double>::RouteInfo route_info) const;

    private:
        RoutingSettings routing_settings_;
        const transport_catalogue::TransportCatalogue& catalogue_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_;
        std::unordered_map<std::string_view, graph::VertexId> stops_id_;
    };

} // namespace transport_router