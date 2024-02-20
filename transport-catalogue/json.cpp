#include "json.h"

using namespace std;

namespace json {

    namespace {

        std::string LoadLiteral(std::istream& input) {
            std::string str;

            while (std::isalpha(input.peek())) {
                str.push_back(static_cast<char>(input.get()));
            }
            return str;
        }

        Node LoadNode(istream& input);

        Node LoadArray(std::istream& input) {
            std::vector<Node> array;

            for (char ch; input >> ch && ch != ']';) {
                if (ch != ',') {
                    input.putback(ch);
                }

                array.push_back(LoadNode(input));
            }

            if (!input) {
                throw ParsingError("Failed to read array from stream"s);
            }

            return Node(array);
        }

        Node LoadNull(std::istream& input) {
            if (auto value = LoadLiteral(input); value == "null"sv) {
                return Node(nullptr);
            }
            else {
                throw ParsingError("Failed to convert "s + value + " to null"s);
            }
        }

        Node LoadBool(std::istream& input) {
            const auto value = LoadLiteral(input);

            if (value == "true"sv) {
                return Node(true);
            }
            else if (value == "false"sv) {
                return Node(false);
            }
            else {
                throw ParsingError("Failed to convert "s + value + " to bool"s);
            }
        }

        Node LoadNumber(std::istream& input) {
            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }

            if (input.peek() == '0') {
                read_char();
            }
            else {
                read_digits();
            }

            bool is_int = true;
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadDict(std::istream& input) {
            Dict result;

            for (char ch; input >> ch && ch != '}';) {
                if (ch == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> ch && ch == ':') {
                        if (result.find(key) != result.end()) {
                            throw ParsingError("duplicate key '"s + key + "'found");
                        }
                        result.emplace(std::move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError(": expected. but '"s + ch + "' found"s);
                    }
                }
                else if (ch != ',') {
                    throw ParsingError("',' expected. but '"s + ch + "' found"s);
                }
            }

            if (!input) {
                throw ParsingError("Failed to read Dict from stream"s);
            }
            else {
                return Node(result);
            }

        }

        Node LoadNode(std::istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return IsPureDouble() || IsInt();
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        else {
            throw std::logic_error("Node doesn`t contain int");
        }
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(*this);
        }
        else {
            throw std::logic_error("Node doesn`t contain bool");
        }
    }

    double Node::AsDouble() const {
        if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        else if (IsInt()) {
            return static_cast<double>(AsInt());
        }
        else {
            throw std::logic_error("Node doesn`t contain double");
        }
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(*this);
        }
        else {
            throw std::logic_error("Node doesn`t contain string");
        }
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(*this);
        }
        else {
            throw std::logic_error("Node doesn`t contain Array");
        }
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(*this);
        }
        else {
            throw std::logic_error("Node doesn`t contain Dict");
        }
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        [[nodiscard]] PrintContext Indented() const {
            return { out,
                    indent_step,
                    indent + indent_step };
        }
    };

    void PrintNode(const Node& node, const PrintContext& context);

    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');

        for (const char c : value) {
            switch (c) {
            case '\t':
                out << "\\t"sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }

        out.put('"');
    }

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& context) {
        context.out << value;
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& context) {
        context.out << "null"sv;
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& context) {
        PrintString(value, context.out);
    }

    void PrintValue(bool value, const PrintContext& context) {
        context.out << std::boolalpha << value;
    }

    void PrintValue(Array arr, const PrintContext& context) {
        std::ostream& out = context.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_context = context.Indented();

        for (const Node& node : arr) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }

            inner_context.PrintIndent();
            PrintNode(node, inner_context);
        }

        out.put('\n');
        context.PrintIndent();
        out.put(']');
    }

    void PrintValue(Dict dict, const PrintContext& context) {
        std::ostream& out = context.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_context = context.Indented();

        for (const auto& [key, node] : dict) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }

            inner_context.PrintIndent();
            PrintString(key, context.out);
            out << ": "sv;
            PrintNode(node, inner_context);
        }

        out.put('\n');
        context.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& context) {
        std::visit([&context](const auto& value) {
            PrintValue(value, context);
            }, node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), PrintContext{ output });
    }

}  // namespace json