#include "stat_reader.h"

#include <iostream>
#include <fstream>

namespace transport_catalogue::output {

    void OutputRequest(const TransportCatalogue& catalogue) {
        using namespace std::string_literals;
        int stat_request_count;
        std::cin >> stat_request_count >> std::ws;
        for (int i = 0; i < stat_request_count; ++i) {
            std::string line;
            std::getline(std::cin, line);
            transport_catalogue::output::ParseAndPrintStat(catalogue, line, std::cout);
        }
    }

    void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output) {
        using namespace std::string_literals;

        if (request.substr(0, 4) == "Stop") {
            std::string_view stop_name = request.substr(5, request.size() - 1);
            Stop* stop = tansport_catalogue.FindStop(stop_name);
            output << request << ": "s;
            if (stop == nullptr) {
                output << "not found"s << std::endl;
            }
            else if (stop->buses.empty()) {
                output << "no buses"s << std::endl;
            }
            else {
                auto sorted_unique_buses_on_stop = detail::GetSortedUniqueBuses(stop);
                output << "buses "s;
                for (auto bus : sorted_unique_buses_on_stop) {
                    output << bus << " "s;
                }
                output << std::endl;
            }
        }

        if (request.substr(0, 3) == "Bus") {
            std::string_view bus_name = request.substr(4, request.size() - 1);
            Bus* bus = tansport_catalogue.FindBus(bus_name);
            output << request << ": "s;
            if (bus == nullptr) {
                output << "not found"s << std::endl;
            }
            else {
                output << detail::CalculateStops(bus) << " stops on route, "s << detail::CalculateUniqueStops(bus) << " unique stops, "s << detail::CalculateRouteLength(bus) << " route length"s << std::endl;
            }
        }
    }

}