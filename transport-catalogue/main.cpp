#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    json_reader::JsonReader requests(std::cin);
    requests.FillTransportCatalogue(catalogue);
    requests.PrintStat(catalogue);
}