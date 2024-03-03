#pragma once

#include "json.h"

#include <optional>

namespace json {

    class ItemContext;
    class KeyItemContext;
    class DictItemContext;
    class ArrayItemContext;

    class Builder {
    public:
        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();
    private:
        Node root_{ nullptr };
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> key_ { std::nullopt };

        Node CreateNode(Node::Value value);
    };

    class ItemContext {
    public:
        ItemContext(Builder& builder)
            : builder_(builder)
        {
        }
        KeyItemContext Key(std::string key);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
    protected:
        Builder& builder_;
    };

    class KeyItemContext : public ItemContext {
    public:
        DictItemContext Value(Node::Value value);
    private:
        KeyItemContext Key(std::string key) = delete;
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
    };

    class DictItemContext : public ItemContext {
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        DictItemContext StartDict() = delete;
    };

    class ArrayItemContext : public ItemContext {
    public:
        ArrayItemContext Value(Node::Value value);
    private:
        KeyItemContext Key(std::string key) = delete;
        Builder& EndDict() = delete;
    };

} // namespace json
