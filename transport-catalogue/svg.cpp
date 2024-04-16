#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, Color color) {
        if (std::holds_alternative<std::monostate>(color)) {
            out << "none"sv;
        }
        if (std::holds_alternative<std::string>(color)) {
            out << std::get<std::string>(color);
        }
        if (std::holds_alternative<Rgb>(color)) {
            out << "rgb("sv
                << int(std::get<Rgb>(color).red) << ","sv
                << int(std::get<Rgb>(color).green) << ","sv
                << int(std::get<Rgb>(color).blue) << ")"sv;
        }
        if (std::holds_alternative<Rgba>(color)) {
            out << "rgba("sv
                << int(std::get<Rgba>(color).red) << ","sv
                << int(std::get<Rgba>(color).green) << ","sv
                << int(std::get<Rgba>(color).blue) << ","sv
                << std::get<Rgba>(color).opacity << ")"sv;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap) {
        switch (stroke_line_cap)
        {
        case svg::StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case svg::StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case svg::StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join) {
        switch (stroke_line_join)
        {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case svg::StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        RenderContext context(out, 0, 2);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& object : objects_) {
            object->Render(context);
        }
        out << "</svg>"sv;
    }

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            out << points_[i].x << ","sv << points_[i].y;
            if (i + 1 != points_.size()) {
                out << " "sv;
            }
        }
        out << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        std::string data_for_svg;
        for (char c : data) {
            switch (c) {
            case '"':
                data_for_svg += "&quot;"sv;
                break;
            case '\'':
                data_for_svg += "&apos;"sv;
                break;
            case '`':
                data_for_svg += "&apos;"sv;
                break;
            case '<':
                data_for_svg += "&lt;"sv;
                break;
            case '>':
                data_for_svg += "&gt;"sv;
                break;
            case '&':
                data_for_svg += "&amp;"sv;
                break;
            default:
                data_for_svg += c;
                break;
            }
        }
        data_ = std::move(data_for_svg);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text "sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_;
        if (!font_family_.empty()) {
            out << "\" font-family=\""sv << font_family_;
        }
        if (!font_weight_.empty()) {
            out << "\" font-weight=\""sv << font_weight_;
        }
        out << "\">"sv << data_ << "</text>"sv;
    }

}  // namespace svg