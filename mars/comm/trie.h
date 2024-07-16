//
// Created by torenchen on 24-7-9.
//

#ifndef MARS_TRIE_H
#define MARS_TRIE_H

#include <map>
#include <memory>

namespace mars {
namespace comm {
/**
 * Trie
 * @tparam StringType std::string or std::vector<std::string>
 * @tparam Property should be a shared pointer
 */
template <typename StringType, typename Property>
class Trie : public std::enable_shared_from_this<Trie<StringType, Property>> {
 public:
    using TrieType = Trie<StringType, Property>;
    using Ptr = std::shared_ptr<TrieType>;

    static Trie::Ptr Create() {
        return Trie::Ptr(new TrieType());
    }

    ~Trie<StringType, Property>() = default;
    Trie<StringType, Property>(TrieType const& other) = default;
    TrieType& operator=(TrieType const& other) = default;

    Trie<StringType, Property>(TrieType&& other) = default;
    TrieType& operator=(TrieType&& other) = default;

    bool Insert(StringType segments, Property prop) {
        TrieType::Ptr node = this->shared_from_this();
        if (node == nullptr) {
            return false;
        }
        for (auto const& seg : segments) {
            auto iter = node->children.find(seg);
            if (iter == node->children.end()) {
                auto insert_result = node->children.insert(std::make_pair(seg, TrieType::Create()));
                if (!insert_result.second) {
                    // insert fail
                    return false;
                }
                iter = insert_result.first;
            }
            node = iter->second;
        }
        node->is_valid = true;
        node->prop = prop;
        return true;
    }

    TrieType::Ptr Search(StringType const& segments, std::size_t length) {
        TrieType::Ptr node = this->shared_from_this();
        if (node == nullptr) {
            return nullptr;
        }
        for (std::size_t i = 0; i < length; ++i) {
            auto iter = node->children.find(segments[i]);
            if (iter == node->children.end()) {
                return nullptr;
            }
            node = iter->second;
        }
        return node;
    }

    TrieType::Ptr Search(StringType const& segments) {
        return Search(segments, segments.size());
    }

    TrieType::Ptr SearchPrefix(StringType const& segments) {
        return Search(segments, segments.size() - 1U);
    }

    void Reset() {
        prop.reset();
        children.clear();
        is_valid = false;
    }

    Property GetProperty() {
        return prop;
    }

    Property const GetProperty() const {
        return prop;
    }

    std::map<typename StringType::value_type, TrieType::Ptr> GetChildren() {
        return children;
    }

    std::map<typename StringType::value_type, TrieType::Ptr> const GetChildren() const {
        return children;
    }

    void SetProperty(Property const& other_prop) {
        prop = other_prop;
    }

    bool IsValid() {
        return is_valid;
    }

 protected:
    Trie<Property>() = default;

 private:
    bool is_valid{false};
    Property prop;
    std::map<typename StringType::value_type, TrieType::Ptr> children{};
};

}  // namespace comm
}  // namespace mars
#endif  // MARS_TRIE_H
