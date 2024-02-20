#include "request_handler.h"

namespace request_handler {

	svg::Document RequestHandler::RenderMap() const {
		return renderer_.GetSvgDocument(db_.GetSortedBuses());
	}

} // namespace request_handler