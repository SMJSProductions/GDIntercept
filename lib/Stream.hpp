#pragma once

#include <string>
#include <vector>
#include <concepts>

template<typename T>
class Stream;

template<typename T>
concept is_stream = requires(T* stream) {
    []<typename U>(Stream<U>*) {}(stream);
};

template<typename F, typename Returns, typename ...Args>
concept returning_lambda = std::invocable<F, Args...> && std::is_convertible_v<std::invoke_result_t<F, Args...>, Returns>;

template<typename F, typename Returns, typename ...PrefixArgs>
concept basic_lambda = returning_lambda<F, Returns, PrefixArgs...>;

template<typename F, typename Returns, typename ...PrefixArgs>
concept indexed_lambda = returning_lambda<F, Returns, PrefixArgs..., size_t>;

template<typename F, typename Returns, typename Value, typename ...PrefixArgs>
concept full_lambda = returning_lambda<F, Returns, PrefixArgs..., size_t, Stream<Value>&>;

template<typename F, typename Returns, typename Value, typename ...PrefixArgs>
concept stream_lambda = basic_lambda<F, Returns, PrefixArgs...> || indexed_lambda<F, Returns, PrefixArgs...> || full_lambda<F, Returns, Value, PrefixArgs...>;

template<typename F, typename ...PrefixArgs>
concept inferred_basic_lambda = std::invocable<F, PrefixArgs...>;

template<typename F, typename ...PrefixArgs>
concept inferred_indexed_lambda = std::invocable<F, PrefixArgs..., size_t>;

template<typename F, typename Value, typename ...PrefixArgs>
concept inferred_full_lambda = std::invocable<F, PrefixArgs..., size_t, Stream<Value>&>;

template<typename F, typename Value, typename ...PrefixArgs>
concept inferred_stream_lambda = inferred_basic_lambda<F, PrefixArgs...> || inferred_indexed_lambda<F, PrefixArgs...> || inferred_full_lambda<F, Value, PrefixArgs...>;

template<typename F, typename Value, typename... PrefixArgs>
struct __stream_lambda_return_impl;

template<typename F, typename Value, typename... PrefixArgs> requires(inferred_basic_lambda<F, PrefixArgs...>)
struct __stream_lambda_return_impl<F, Value, PrefixArgs...> {
    using type = std::invoke_result_t<F, PrefixArgs...>;
};

template<typename F, typename Value, typename... PrefixArgs> requires (!inferred_basic_lambda<F, PrefixArgs...> && inferred_indexed_lambda<F, PrefixArgs...>)
struct __stream_lambda_return_impl<F, Value, PrefixArgs...> {
    using type = std::invoke_result_t<F, PrefixArgs..., size_t>;
};

template<typename F, typename Value, typename... PrefixArgs> requires (
    !inferred_basic_lambda<F, PrefixArgs...> &&
    !inferred_indexed_lambda<F, PrefixArgs...> &&
    inferred_stream_lambda<F, Value, PrefixArgs...>
)
struct __stream_lambda_return_impl<F, Value, PrefixArgs...> {
    using type = std::invoke_result_t<F, PrefixArgs..., size_t, Stream<Value>&>;
};

template<typename F, typename Value, typename ...PrefixArgs>
using stream_lambda_return = __stream_lambda_return_impl<F, Value, PrefixArgs...>::type;

// NOTE! Non constant versions will consume on each iteration
template<typename T>
class Stream : public std::vector<T> {
public:
    Stream& operator=(Stream&&) noexcept(std::is_nothrow_move_assignable_v<std::vector<T>>) = default;

    Stream() noexcept = default;
    Stream(const Stream& other) = default;
    Stream(Stream&& other) noexcept = default;
    Stream(std::vector<T>&& vector) noexcept : std::vector<T>(std::move(vector)) { }
    Stream(const std::vector<T>& vector) : std::vector<T>(vector) { }
    Stream(const std::initializer_list<T>& list) : std::vector<T>(list) { }
    template<typename I>
    Stream(const I& begin, const I& end) : std::vector<T>(begin, end) { }
    template<typename ...Args> requires(std::convertible_to<Args, T> && ...)
    Stream(Args&& ...args) : std::vector<T>{ std::forward<Args>(args)... } { }

    template<stream_lambda<bool, T, const T&> F>
    [[nodiscard]] Stream<T> filter(F&& predicate) && {
        Stream<T> stream;

        for (size_t i = 0; i < this->size(); i++) {
            T& element = (*this)[i];

            if constexpr (basic_lambda<F, bool, const T&>) {
                if (predicate(element)) stream.emplace_back(std::move(element));
            } else if constexpr (indexed_lambda<F, bool, const T&>) {
                if (predicate(element, i)) stream.emplace_back(std::move(element));
            } else {
                if (predicate(element, i, *this)) stream.emplace_back(std::move(element));
            }
        }

        return stream;
    }

    template<stream_lambda<bool, T, const T&> F>
    Stream<T> filter(F&& predicate) const& {
        Stream<T> stream;

        for (size_t i = 0; i < this->size(); i++) {
            const T& element = (*this)[i];

            if constexpr (basic_lambda<F, bool, const T&>) {
                if (predicate(element)) stream.emplace_back(element);
            } else if constexpr (indexed_lambda<F, bool, const T&>) {
                if (predicate(element, i)) stream.emplace_back(element);
            } else {
                if (predicate(element, i, *this)) stream.emplace_back(element);
            }
        }

        return stream;
    }

    template<stream_lambda<bool, T, T&&> F>
    [[nodiscard]] bool every(F&& predicate) && {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, bool, T&&>) {
                if (!predicate(std::move((*this)[i]))) return false;
            } else if constexpr (indexed_lambda<F, bool, T&&>) {
                if (!predicate(std::move((*this)[i]), i)) return false;
            } else {
                if (!predicate(std::move((*this)[i]), i, *this)) return false;
            }
        }

        return true;
    }

    template<stream_lambda<bool, T, const T&> F>
    bool every(F&& predicate) const& {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, bool, const T&>) {
                if (!predicate((*this)[i])) return false;
            } else if constexpr (indexed_lambda<F, bool, const T&>)  {
                if (!predicate((*this)[i], i)) return false;
            } else {
                if (!predicate((*this)[i], i, *this)) return false;
            }
        }

        return true;
    }

    template<stream_lambda<bool, T, T&&> F>
    [[nodiscard]] bool some(F&& predicate) && {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, bool, T&&>) {
                if (predicate(std::move((*this)[i]))) return true;
            } else if constexpr (indexed_lambda<F, bool, T&&>) {
                if (predicate(std::move((*this)[i]), i)) return true;
            } else {
                if (predicate(std::move((*this)[i]), i, *this)) return true;
            }
        }

        return false;
    }

    template<stream_lambda<bool, T, const T&> F>
    bool some(F&& predicate) const& {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, bool, const T&>) {
                if (predicate((*this)[i])) return true;
            } else if constexpr (indexed_lambda<F, bool, const T&>) {
                if (predicate((*this)[i], i)) return true;
            } else {
                if (predicate((*this)[i], i, *this)) return true;
            }
        }

        return false;
    }

    template<inferred_stream_lambda<T, T&&> F>
    [[nodiscard]] Stream<stream_lambda_return<F, T, T&&>> map(F&& mapper) && {
        Stream<stream_lambda_return<F, T, T&&>> stream;

        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (inferred_basic_lambda<F, T&&>) {
                stream.emplace_back(mapper(std::move((*this)[i])));
            } else if constexpr (inferred_indexed_lambda<F, T&&>) {
                stream.emplace_back(mapper(std::move((*this)[i]), i));
            } else {
                stream.emplace_back(mapper(std::move((*this)[i]), i, *this));
            }
        }

        return stream;
    }

    template<inferred_stream_lambda<T, const T&> F>
    Stream<stream_lambda_return<F, T, const T&>> map(F&& mapper) const& {
        Stream<stream_lambda_return<F, T, const T&>> stream;

        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (inferred_basic_lambda<F, const T&>) {
                stream.emplace_back(mapper((*this)[i]));
            } else if constexpr (inferred_indexed_lambda<F, const T&>) {
                stream.emplace_back(mapper((*this)[i], i));
            } else {
                stream.emplace_back(mapper((*this)[i], i, *this));
            }
        }

        return stream;
    }

    template<typename R, stream_lambda<R, T, R&&, T&&> F>
    [[nodiscard]] R reduce(F&& reducer, R initial) && {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, R, R&&, T&&>) {
                initial = reducer(std::move(initial), std::move((*this)[i]));
            } else if constexpr (indexed_lambda<F, R, R&&, T&&>) {
                initial = reducer(std::move(initial), std::move((*this)[i]), i);
            } else {
                initial = reducer(std::move(initial), std::move((*this)[i]), i, *this);
            }
        }

        return initial;
    }

    template<typename R, stream_lambda<R, T, R&&, const T&> F>
    R reduce(F&& reducer, R initial) const& {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, R, R&&, const T&>) {
                initial = reducer(std::move(initial), (*this)[i]);
            } else if constexpr (indexed_lambda<F, R, R&&, const T&>) {
                initial = reducer(std::move(initial), (*this)[i], i);
            } else {
                initial = reducer(std::move(initial), (*this)[i], i, *this);
            }
        }

        return initial;
    }

    template<stream_lambda<void, T, T&&> F>
    void forEach(F&& consumer) && {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, void, T&&>) {
                consumer(std::move((*this)[i]));
            } else if constexpr (indexed_lambda<F, void, T&&>) {
                consumer(std::move((*this)[i]), i);
            } else {
                consumer(std::move((*this)[i]), i, *this);
            }
        }
    }

    template<stream_lambda<void, T, const T&> F>
    void forEach(F&& consumer) const& {
        for (size_t i = 0; i < this->size(); i++) {
            if constexpr (basic_lambda<F, void, const T&>) {
                consumer((*this)[i]);
            } else if constexpr (indexed_lambda<F, void, const T&>) {
                consumer((*this)[i], i);
            } else {
                consumer((*this)[i], i, *this);
            }
        }
    }

    template<basic_lambda<T, T, const T&, const T&> F>
    [[nodiscard]] Stream<T> sort(F&& comparator = [](const T& a, const T& b) { return a < b; }) && {
        std::sort(this->begin(), this->end(), comparator);

        return std::move(*this);
    }

    template<basic_lambda<T, T, const T&, const T&> F>
    Stream<T> sort(F&& comparator = [](const T& a, const T& b) { return a < b; }) const& {
        Stream<T> stream = *this;

        std::sort(stream.begin(), stream.end(), comparator);

        return stream;
    }

    [[nodiscard]] Stream<T> unique() && {
        Stream<T> stream;

        for (T& element : *this) {
            if (!this->includes(element)) {
                stream.emplace_back(std::move(element));
            }
        }

        return stream;
    }

    Stream<T> unique() const& {
        Stream<T> stream;

        for (const T& element : *this) {
            if (!this->includes(element)) {
                stream.emplace_back(element);
            }
        }

        return stream;
    }

    [[nodiscard]] auto flat() && requires(is_stream<T>) {
        Stream<typename T::value_type> stream;

        for (T& element : *this) {
            stream.concatInPlace(std::move(element));
        }

        return stream;
    }

    auto flat() const& requires(is_stream<T>) {
        Stream<typename T::value_type> stream;

        for (const T& element : *this) {
            stream.concatInPlace(element);
        }

        return stream;
    }

    [[nodiscard]] Stream<T> reverse() && {
        std::reverse(this->begin(), this->end());

        return std::move(*this);
    }

    Stream<T> reverse() const& {
        Stream<T> stream = *this;

        std::reverse(stream.begin(), stream.end());

        return stream;
    }

    [[nodiscard]] Stream<T> concat(Stream<T> stream) && {
        for (T& element : stream) {
            this->emplace_back(std::move(element));
        }

        return std::move(*this);
    }

    Stream<T> concat(Stream<T> stream) const& {
        Stream<T> result = *this;

        for (T& element : stream) {
            result.emplace_back(std::move(element));
        }

        return result;
    }

    void concatInPlace(Stream<T>&& stream) {
        this->reserve(this->size() + stream.size());
        this->insert(this->end(), std::make_move_iterator(stream.begin()), std::make_move_iterator(stream.end()));
    }

    [[nodiscard]] Stream<T> concat(std::vector<T> vector) && {
        return this->concat(std::move(Stream<T>(vector)));
    }

    Stream<T> concat(std::vector<T> vector) const& {
        return this->concat(std::move(Stream<T>(vector)));
    }

    [[nodiscard]] Stream<T> concat(std::initializer_list<T> list) && {
        return this->concat(std::move(list));
    }

    Stream<T> concat(std::initializer_list<T> list) const& {
        return this->concat(std::move(list));
    }

    [[nodiscard]] Stream<T> slice(const size_t start, const size_t end) && {
        const size_t cappedEnd = std::min(end, this->size());

        return Stream<T>(
            std::make_move_iterator(this->begin() + (start > cappedEnd ? 0 : start)),
            std::make_move_iterator(this->begin() + cappedEnd)
        );
    }

    [[nodiscard]] Stream<T> slice(const size_t start) && {
        return this->slice(start, this->size());
    }

    Stream<T> slice(const size_t start, const size_t end) const& {
        const size_t cappedEnd = std::min(end, this->size());

        return Stream<T>(
            this->begin() + (start > cappedEnd ? 0 : start),
            this->begin() + cappedEnd
        );
    }

    Stream<T> slice(const size_t start) const& {
        return this->slice(start, this->size());
    }

    [[nodiscard]] Stream<T> splice(const size_t start, const size_t count, Stream<T>&& replacement = {}) && {
        const size_t cappedStart = start >= this->size() ? 0 : start;
        const size_t cappedCount = std::min(cappedStart + count, this->size());

        this->reserve(cappedStart + replacement.size() + this->size() - cappedCount);
        this->erase(this->begin() + cappedStart, this->begin() + cappedCount);

        if (replacement.size()) {
            this->insert(this->begin() + cappedStart, std::make_move_iterator(replacement.begin()), std::make_move_iterator(replacement.end()));
        }

        return std::move(*this);
    }

    Stream<T> splice(const size_t start, const size_t count, const Stream<T>& replacement = {}) const& {
        const size_t cappedStart = start >= this->size() ? 0 : start;
        const size_t cappedCount = std::min(cappedStart + count, this->size());
        Stream<T> stream = *this;

        stream.reserve(cappedStart + replacement.size() + this->size() - cappedCount);
        stream.erase(stream.begin() + cappedStart, stream.begin() + cappedCount);

        if (replacement.size()) {
            stream.insert(stream.begin() + cappedStart, replacement.begin(), replacement.end());
        }

        return stream;
    }

    template<typename R>
    [[nodiscard]] Stream<R> cast() && {
        Stream<R> stream;

        for (auto&& value : std::move(*this)) {
            stream.emplace_back(std::move(value));
        }

        return stream;
    }

    template<typename R>
    Stream<R> cast() const& {
        Stream<R> stream;

        for (auto& value : *this) {
            stream.emplace_back(value);
        }

        return stream;
    }

    template<std::invocable F>
    [[nodiscard]] Stream<T> orExecute(F&& execution) && {
        if (this->empty()) {
            execution();
        }

        return std::move(*this);
    }

    template<std::invocable F>
    Stream<T> orExecute(F&& execution) const& {
        if (this->empty()) {
            execution();
        }

        return *this;
    }

    bool includes(const T& value) const {
        return std::find(this->begin(), this->end(), value) != this->end();
    }
};

template<typename T = long long> requires(std::is_arithmetic_v<T>)
class IntStream : public Stream<T> {
    using Stream<T>::Stream;
public:
    [[nodiscard]] static IntStream<T> range(const T exclusiveEnd) {
        return IntStream::range(0, exclusiveEnd);
    }

    [[nodiscard]] static IntStream<T> range(const T includeStart, const T exclusiveEnd) {
        IntStream<T> stream;

        for (T i = includeStart; i < exclusiveEnd; i++) {
            stream.emplace_back(i);
        }

        return stream;
    }
};

class StringUtils : public Stream<std::string> {
    using Stream::Stream;
public:
    static constexpr auto TRIM_CHARS = "\t\n\r\f\v ";

    [[nodiscard]] static StringUtils split(const std::string_view input, const char delimiter) {
        StringUtils stream;
        size_t start = 0;

        for (size_t end; (end = input.find(delimiter, start)) != std::string_view::npos; start = end + 1) {
            stream.emplace_back(input.substr(start, end - start));
        }

        stream.emplace_back(input.substr(start));

        return stream;
    }

    [[nodiscard]] static StringUtils split(const std::string_view input, const std::string_view delimiter) {
        StringUtils stream;
        size_t start = 0;

        if (delimiter.empty()) {
            stream.emplace_back(input);

            return stream;
        }

        for (size_t end; (end = input.find(delimiter, start)) != std::string_view::npos; start = end + delimiter.size()) {
            stream.emplace_back(input.substr(start, end - start));
        }

        stream.emplace_back(input.substr(start));

        return stream;
    }

    static std::string join(const std::vector<std::string>& input, const std::string_view delimiter) {
        return StringUtils(input).reduce([delimiter](std::string&& acc, std::string&& part, const size_t i) {
            if (i) acc += delimiter;

            acc += part;

            return std::move(acc);
        }, std::string());
    }

    static std::string replace(const std::string_view input, const std::string_view delimiter, const std::string_view replacement) {
        return StringUtils::join(StringUtils::split(input, delimiter), replacement);
    }

    static std::string_view trim(std::string_view input) {
        const size_t first = input.find_first_not_of(StringUtils::TRIM_CHARS);

        if (first == std::string_view::npos) {
            return {};
        } else {
            input.remove_suffix(input.size() - input.find_last_not_of(StringUtils::TRIM_CHARS) - 1);
            input.remove_prefix(first);

            return input;
        }
    }

    static std::string trim(std::string&& input) {
        const size_t first = input.find_first_not_of(StringUtils::TRIM_CHARS);

        if (first == std::string_view::npos) {
            return {};
        } else {
            input.erase(input.find_last_not_of(StringUtils::TRIM_CHARS) + 1);
            input.erase(0, first);

            return std::move(input);
        }
    }

    bool includes(const std::string_view value) const {
        return std::find(this->begin(), this->end(), value) != this->end();
    }
};

template<typename I>
Stream(const I&, const I&) -> Stream<typename std::iterator_traits<I>::value_type>;