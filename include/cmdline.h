/*
    cpp-cmdline - Code covered by the MIT License
    Author: mutouyun (http://orzz.org)
*/

#pragma once

#include <deque>
#include <string>
#include <functional>
#include <utility>
#include <iostream>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <algorithm>

namespace cmdline {

class str_view {

    char const * str_  = nullptr;
    std::size_t  size_ = 0;

public:
    str_view() noexcept = default;

    str_view(std::string const & s) noexcept
        : str_ (s.c_str())
        , size_(s.size ()) {
    }

    str_view(char const * str, std::size_t size) noexcept
        : str_ (str )
        , size_((str == nullptr) ? 0 : size) {
    }

    str_view(char const * str)
        : str_ (str)
        , size_((str == nullptr) ? 0 : std::string::npos) {
    }

    template <std::size_t N>
    str_view(char const (& str)[N]) noexcept
        : str_ (str )
        , size_(N - 1) {
    }

    void swap(str_view & rhs) noexcept {
        std::swap(str_ , rhs.str_ );
        std::swap(size_, rhs.size_);
    }

    char const * data() const noexcept {
        return str_;
    }

    std::size_t size() const noexcept {
        return size_;
    }

    std::size_t length() const {
        return (size() == std::string::npos) ? (assert(str_ != nullptr), std::strlen(str_)) : size();
    }

    bool empty() const noexcept {
        return (data() == nullptr) || (size() == 0) || (data()[0] == '\0');
    }

    std::size_t find_first_of(char const c) const noexcept {
        if (empty()) return std::string::npos;
        for (std::size_t i = 0; i < size(); ++i) {
            if (data()[i] == c) {
                return i;
            }
            else if (data()[i] == '\0') {
                return std::string::npos;
            }
            else continue;
        }
        return std::string::npos;
    }

    std::size_t find_last_of(char const c) const {
        if (empty()) return std::string::npos;
        std::size_t sz = length();
        for (std::size_t i = sz - 1; i > 0; --i) {
            if (data()[i] == c) {
                return i;
            }
            else continue;
        }
        return (data()[0] == c) ? 0 : std::string::npos;
    }

    str_view substr(std::size_t off, std::size_t count = std::string::npos) const noexcept {
        if (empty()) return {};
        if (count == 0) return {};
        std::size_t sz = length();
        if (off >= sz) return {};
        return { data() + off, std::min<std::size_t>(count, sz - off) };
    }

    char & operator[](std::size_t i) noexcept {
        return const_cast<char *>(data())[i];
    }

    char const operator[](std::size_t i) const noexcept {
        return (*const_cast<str_view *>(this))[i];
    }

    friend bool operator==(str_view const & lhs, str_view const & rhs) noexcept {
        if ((lhs.data() == rhs.data()) && (lhs.size() == rhs.size())) {
            return true;
        }
        if (lhs.empty() && rhs.empty()) return true;
        if (lhs.empty() || rhs.empty()) return false;
        if (lhs.size() == rhs.size()) {
            return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
        }
        std::size_t e = std::min<std::size_t>(lhs.size(), rhs.size());
        if (std::memcmp(lhs.data(), rhs.data(), e) != 0) {
            return false;
        }
        return ((lhs.size() < rhs.size()) ? rhs : lhs)[e] == '\0';
    }

    friend bool operator!=(str_view const & lhs, str_view const & rhs) noexcept {
        return !(lhs == rhs);
    }

    friend std::ostream & operator<<(std::ostream & o, str_view const & sv) {
        for (std::size_t i = 0; (i < sv.size()) && (sv[i] != '\0'); ++i) {
            o << sv[i];
        }
        return o;
    }
};

class parser;
using handle_t = void(parser &, str_view const &);

struct option {
    char const * sname_;
    char const * lname_;
    char const * description_;
    bool         necessary_;
    std::string  default_;
    std::function<handle_t> handle_;
};

using options_t = std::deque<option>;

class parser {

    options_t necessary_, optional_;
    std::function<handle_t> usage_;
    std::function<void(str_view const &)> printer_;
    str_view path_;

    void print_impl() const noexcept {}

    template <typename T, typename ... A>
    void print_impl(T && t, A && ... args) const {
        printer_(std::forward<T>(t));
        print_impl(std::forward<A>(args)...);
    }

    template <typename ... A>
    void print(A && ... args) const {
        if (printer_) {
            print_impl(std::forward<A>(args)...);
        }
    }

public:
    parser() {
        set_printer(std::cout);
    }

    options_t &       necessary()       noexcept { return necessary_; }
    options_t const & necessary() const noexcept { return necessary_; }

    options_t &       optional()       noexcept { return optional_; }
    options_t const & optional() const noexcept { return optional_; }

    std::function<handle_t> &       usage()       noexcept { return usage_; }
    std::function<handle_t> const & usage() const noexcept { return usage_; }

    template <typename F>
    void set_usage(F && u) {
        usage_ = std::forward<F>(u);
    }

    template <typename T, typename = decltype(std::declval<T>() << std::declval<str_view>())>
    void set_printer(T && o) {
        printer_ = [this, &o](str_view const & sv) {
            o << sv;
        };
    }

    void print_usage() {
        if (path_.empty()) {
            print("Must has at least one argument (the path of current program).\n");
            return;
        }
        std::size_t slash = path_.find_last_of('\\');
        if (slash == std::string::npos) slash = path_.find_last_of('/');
        str_view name = path_.substr(slash + 1);
        if (usage_) {
            usage_(*this, name);
        }
        else {
            print("Usage: ", name, " ");
            if (!necessary_.empty()) for (auto & o : necessary_) {
                print(o.lname_);
                if (!o.default_.empty()) print("=", o.default_);
                print(" ");
            }
            print("[OPTIONS]...\n");
            print("Options: \n");
            auto print_opt = [&](auto & o) {
                print(" ");
                if (o.sname_ != nullptr) {
                    print(" ", o.sname_, ",");
                }
                print(" ", o.lname_, " \t", o.description_);
                if (!o.default_.empty()) print("[=", o.default_, "]");
                print("\n");
            };
            for (auto & o : necessary_) { print_opt(o); }
            for (auto & o : optional_ ) { print_opt(o); }
        }
    }

    void push(options_t && opts) {
        for (auto && o : opts) {
            if (o.lname_ == nullptr) {
                continue;
            }
            options_t * list = &(o.necessary_ ? necessary_ : optional_);
            list->emplace_back(std::move(o));
        }
    }

    void clear(void) {
        necessary_.clear();
        optional_ .clear();
    }

    int exec(int argc, char const * const argv[]) {
        if (argc >= 1) path_ = argv[0];
        if (argc <= 1) {
            print_usage();
        }
        else {
            struct ST_opt {
                std::function<handle_t> const & hd_;
                str_view cm_;
            };
            std::deque<ST_opt> exec_list;
            std::size_t c_nec = 0;
            for (int i = 1; i < argc; ++i) {
                str_view    a = argv[i];
                std::size_t c = a.find_first_of('=');
                str_view    o = a.substr(0, c);
                auto foreach = [&](options_t const & cc, auto && fr) {
                    for (option const & it : cc) {
                        if (((it.sname_ != nullptr) && (o == it.sname_)) || (o == it.lname_)) {
                            exec_list.emplace_back(ST_opt {
                                it.handle_,
                                (c == std::string::npos) ? it.default_ : a.substr(c + 1)
                            });
                            fr();
                        }
                    }
                };
                foreach(necessary_, [&c_nec] { ++c_nec; });
                foreach(optional_ , [] {});
            }
            if ((c_nec != necessary_.size()) || exec_list.empty()) {
                print_usage();
            }
            else for (ST_opt & e : exec_list) {
                e.hd_(*this, e.cm_);
            }
        }
        return 0;
    }
};

} // namespace cmdline
