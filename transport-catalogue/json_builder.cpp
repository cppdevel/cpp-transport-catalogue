#include "json_builder.h"

namespace json {

    KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsNull()) {
            throw std::logic_error("Unable to call Key(): creating Key outside of Dict");
        }
        if (key_.has_value()) {
            throw std::logic_error("Unable to call Key(): creating Key after Key");
        }
        if (nodes_stack_.back()->IsMap() && !key_) {
            nodes_stack_.back()->AsMapNonConstant()[key] = {};
            key_ = std::move(key);
        }
        return { *this };
    }

    Builder& Builder::Value(Node::Value value) {
        Node node_value = CreateNode(std::move(value));
        if (!nodes_stack_.empty()) {
            if (nodes_stack_.back()->IsMap()) {
                if (!key_.has_value()) {
                    throw std::logic_error("Unable to call Value(): can't add Value without Key");
                }
                nodes_stack_.back()->AsMapNonConstant()[key_.value()] = std::move(node_value);
                key_ = std::nullopt;
            }
            if (nodes_stack_.back()->IsArray()) {
                nodes_stack_.back()->AsArrayNonConstant().push_back(std::move(node_value));
            }
        }
        else if (nodes_stack_.empty()) {
            if (!root_.IsNull()) {
                throw std::logic_error("Unable to call Value(): root has been added");
            }
            else {
                root_ = std::move(node_value);
            }
        }
        return *this;
    }

    DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty()) {
            root_ = CreateNode(Dict{});
            nodes_stack_.emplace_back(&root_);
        }
        else if (nodes_stack_.back()->IsMap()) {
            if (!key_) {
                throw std::logic_error("Unable to call Value(): can't add Value without Key");
            }
            nodes_stack_.back()->AsMapNonConstant()[key_.value()] = CreateNode(Dict{});
            nodes_stack_.push_back(&nodes_stack_.back()->AsMapNonConstant()[key_.value()]);
            key_ = std::nullopt;
        }
        else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArrayNonConstant().push_back(CreateNode(Dict{}));
            nodes_stack_.push_back(&nodes_stack_.back()->AsArrayNonConstant().back());
        }
        return { *this };
    }

    ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty()) {
            root_ = CreateNode(Array{});
            nodes_stack_.emplace_back(&root_);
        }
        else if (nodes_stack_.back()->IsMap()) {
            if (!key_) {
                throw std::logic_error("Unable to call Value(): can't add Value without Key");
            }
            nodes_stack_.back()->AsMapNonConstant()[key_.value()] = CreateNode(Array{});
            nodes_stack_.push_back(&nodes_stack_.back()->AsMapNonConstant()[key_.value()]);
            key_ = std::nullopt;
        }
        else if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.back()->AsArrayNonConstant().push_back(CreateNode(Array{}));
            nodes_stack_.push_back(&nodes_stack_.back()->AsArrayNonConstant().back());
        }
        return { *this };
    }

    Builder& Builder::EndDict() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Unable to call EndDict(): stack is empty");
        }
        else if (!nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Unable to call EndDict(): the object is not Dictionary");
        }
        else if (nodes_stack_.size() == 1) {
            root_ = *nodes_stack_.back();
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Unable to call EndArray(): stack is empty");
        }
        if (!nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Unable to call EndArray(): the object is not Array");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (root_.IsNull()) {
            throw std::logic_error("Unable to call Build(): calling the function after constructor");
        }
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Unable to call Build(): unfinished Arrays and Dictionaries");
        }
        return root_;
    }

    Node Builder::CreateNode(Node::Value value) {
        if (std::holds_alternative<int>(value)) {
            return Node(std::get<int>(value));
        }
        if (std::holds_alternative<double>(value)) {
            return Node(std::get<double>(value));
        }
        if (std::holds_alternative<bool>(value)) {
            return Node(std::get<bool>(value));
        }
        if (std::holds_alternative<std::nullptr_t>(value)) {
            return Node(std::get<std::nullptr_t>(value));
        }
        if (std::holds_alternative<std::string>(value)) {
            return Node(std::get<std::string>(value));
        }
        if (std::holds_alternative<Dict>(value)) {
            return Node(std::get<Dict>(value));
        }
        if (std::holds_alternative<Array>(value)) {
            return Node(std::get<Array>(value));
        }
        return Node{};
    }

    KeyItemContext ItemContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    DictItemContext ItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext ItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& ItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder& ItemContext::EndArray() {
        return builder_.EndArray();
    }

    DictItemContext KeyItemContext::Value(Node::Value value) {
        return { builder_.Value(std::move(value)) };
    }

    ArrayItemContext ArrayItemContext::Value(Node::Value value) {
        return { builder_.Value(std::move(value)) };
    }

} // namespace json
