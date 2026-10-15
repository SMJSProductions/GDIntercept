#pragma once

#include <vector>
#include <string>

template<typename K, typename V>
class LookupTable {
public:
    using Entry = std::pair<K, V>;

    V& operator[](const K& key) {
        return this->at(key);
    }

    const V& operator[](const K& key) const {
        return this->at(key);
    }

    Entry& operator[](const size_t index) {
        return this->at(index);
    }

    const Entry& operator[](const size_t index) const {
        return this->at(index);
    }

    LookupTable() { }
    LookupTable(const std::initializer_list<Entry>& table) : m_table(table) { }

    std::vector<Entry>::iterator begin() {
        return m_table.begin();
    }

    std::vector<Entry>::const_iterator begin() const {
        return m_table.begin();
    }

    std::vector<Entry>::iterator end() {
        return m_table.end();
    }

    std::vector<Entry>::const_iterator end() const {
        return m_table.end();
    }

    size_t size() const {
        return m_table.size();
    }

    V& at(const K& key) {
        for (auto& [first, second] : m_table) {
            if (first == key) {
                return second;
            }
        }

        throw std::out_of_range("Key not found in LookupTable");
    }

    const V& at(const K& key) const {
        for (const auto& [first, second] : m_table) {
            if (first == key) {
                return second;
            }
        }

        throw std::out_of_range("Key not found in LookupTable");
    }

    Entry& at(const size_t index) {
        return m_table.at(index);
    }

    const Entry& at(const size_t index) const {
        return m_table.at(index);
    }

    std::vector<K> keys() const {
        std::vector<K> keys;

        for (const auto& [first, _] : m_table) {
            keys.push_back(first);
        }

        return keys;
    }

    std::vector<V> values() const {
        std::vector<V> values;

        for (const auto& [_, second] : m_table) {
            values.push_back(second);
        }

        return values;
    }

    std::vector<Entry> entries() const {
        return m_table;
    }

    bool contains(const K& key) const {
        for (const auto& [first, _] : m_table) {
            if (first == key) {
                return true;
            }
        }
        return false;
    }

    void insert(Entry entry) {
        this->erase(entry.first);

        m_table.push_back(std::move(entry));
    }

    void insert(K key, V value) {
        this->insert(std::make_pair(std::move(key), std::move(value)));
    }

    void insert(LookupTable<K, V> table) {
        for (Entry& entry : table) {
            this->insert(std::move(entry));
        }
    }

    void emplace(const K& before, const Entry& entry) {
        this->erase(entry.first);

        for (size_t i = 0; i < m_table.size(); i++) {
            if (m_table.at(i).first == before) {
                m_table.emplace(m_table.begin() + i, entry);
                return;
            }
        }

        m_table.push_back(entry);
    }

    void emplace(const K& before, const K& key, const V& value) {
        this->emplace(before, std::make_pair(key, value));
    }

    void emplace(const K& before, const LookupTable<K, V> table) {
        K last = before;

        for (const Entry& entry : table) {
            this->emplace(last, entry);

            last = entry.first;
        }
    }

    void erase(const K& key) {
        for (size_t i = 0; i < m_table.size(); i++) {
            if (m_table.at(i).first == key) {
                m_table.erase(m_table.begin() + i);
                return;
            }
        }
    }

    void clear() {
        m_table.clear();
    }

    template<typename T>
    T reduce(const std::function<T(const T& accumulator, const Entry& entry)> reducer, const T& initialValue) const {
        T accumulator = initialValue;

        for (const Entry& entry : m_table) {
            accumulator = reducer(accumulator, entry);
        }

        return accumulator;
    }
private:
    std::vector<Entry> m_table;
};