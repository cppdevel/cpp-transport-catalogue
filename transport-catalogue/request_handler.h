#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

#include <optional>

namespace request_handler {

    struct StopStat {
        std::string_view name;
        geo::Coordinates coordinates;
        std::map<std::string_view, int> road_distances;
    };

    struct BusStat {
        std::string_view name;
        std::vector<std::string_view> stops;
        bool is_roundtrip;
    };

    class RequestHandler {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue& db, const map_renderer::MapRenderer& renderer)
            : db_(db)
            , renderer_(renderer)
        {
        }

        svg::Document RenderMap() const;

    private:
        const transport_catalogue::TransportCatalogue& db_;
        const map_renderer::MapRenderer& renderer_;
    };

} // namespace request_handler