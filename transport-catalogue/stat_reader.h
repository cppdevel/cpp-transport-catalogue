#pragma once

#include <iostream>
#include <iosfwd>
#include <iomanip>
#include <string_view>

#include "transport_catalogue.h"

namespace transport_catalogue::output {

    void OutputRequest(const TransportCatalogue& catalogue);

    void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
        std::ostream& output);

}