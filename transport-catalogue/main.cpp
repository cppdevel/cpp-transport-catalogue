#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    using namespace transport_catalogue;

    TransportCatalogue catalogue;

    input::InputRequest(catalogue);
    output::OutputRequest(catalogue);
}