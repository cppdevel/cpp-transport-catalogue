#pragma once

#include <iosfwd>
#include <iomanip>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue::output {

    void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
        std::ostream& output);

}