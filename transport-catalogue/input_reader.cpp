#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_set>

#include "input_reader.h"

namespace transport_catalogue::input {

    void InputRequest(TransportCatalogue& catalogue) {
        int base_request_count;
        std::cin >> base_request_count >> std::ws;
        {
            InputReader reader;
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                std::getline(std::cin, line);
                reader.ParseLine(line);
            }
            reader.ApplyCommands(catalogue);
        }
    }

std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    bool IsBusCircular(std::string_view route) {
        if (route.find('>') != route.npos) {
            return true;
        }
        return false;
    }

    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return { std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1)) };
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    std::vector<std::string_view> ParseDescription(std::string_view description) {
        auto parsed_description = Split(description, ',');
        return parsed_description;
    }

    geo::Coordinates GetCoordinatesFromDescription(const std::vector<std::string_view>& parsed_description) {
        double lat = std::stod(std::string(parsed_description[0]));
        double lng = std::stod(std::string(parsed_description[1]));
        return { lat, lng };
    }

    std::vector<Distance> GetDistancesFromDescription(const TransportCatalogue& catalogue, std::string_view stop_from, const std::vector<std::string_view>& parsed_description) {
        if (parsed_description.size() > 2) {
            std::vector<std::string_view> distances;
            for (auto i = 2; i < parsed_description.size(); ++i) {
                distances.push_back(parsed_description[i]);
            }
            std::vector<Distance> distances_from_request(distances.size());
            for (auto i = 0; i < distances.size(); ++i) {
                auto str = distances[i];
                uint64_t distance = std::stoi(std::string(str.substr(0, str.find('m'))));
                auto stop_to = str.substr(str.find('m') + 5);
                distances_from_request[i].stop_from = catalogue.FindStop(stop_from);
                distances_from_request[i].stop_to = catalogue.FindStop(stop_to);
                distances_from_request[i].distance = distance;
            }
            return distances_from_request;
        }
        return {};
    }

    void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
        for (auto& comm : commands_) {
            if (comm.command == "Stop") {
                catalogue.AddStop(comm.id, GetCoordinatesFromDescription(ParseDescription(comm.description)));
            }
        }
        for (auto& comm : commands_) {
            if (comm.command == "Stop") {
                catalogue.SetDistance(GetDistancesFromDescription(catalogue, comm.id, ParseDescription(comm.description)));
            }
        }
        for (auto& comm : commands_) {
            if (comm.command == "Bus") {
                IsBusCircular(comm.description) ? catalogue.AddBus(comm.id, BusType::CIRCULAR) : catalogue.AddBus(comm.id, BusType::ORDINARY);
                auto stops_from_request = ParseRoute(comm.description);
                catalogue.AddStopToBus(stops_from_request, comm.id);
                catalogue.AddBusToStop(stops_from_request, comm.id);
            }
        }
    }

}
