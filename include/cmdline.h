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

        str_view const * longer;
        std::size_t e;
        if (lhs.size() < rhs.size()) {
            longer = &rhs;
            e = lhs.size();
        }
        else if (lhs.size() > rhs.size()) {
            longer = &lhs;
            e = rhs.size();
        }
        // lhs.size() == rhs.size()
        else return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;

        for (std::size_t i = 0; i < e; ++i) {
            if (lhs[i] != rhs[i]) return false;
        }
        assert(longer != nullptr);
        return (*longer)[e] == '\0';
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
using handle_t = void(parser const &, str_view const &);

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
    str_view path_;

public:
    options_t &       necessary(void)       { return necessary_; }
    options_t const & necessary(void) const { return necessary_; }

    options_t &       optional(void)       { return optional_; }
    options_t const & optional(void) const { return optional_; }

    std::function<handle_t> &       usage(void)       { return usage_; }
    std::function<handle_t> const & usage(void) const { return usage_; }

    template <typename T>
    void print_usage(T && out) const {
        if (path_.empty()) {
            out << "Must has at least one argument (the path of current program)." << std::endl;
            return;
        }
        std::size_t slash = path_.find_last_of('\\');
        if (slash == std::string::npos) slash = path_.find_last_of('/');
        str_view name = path_.substr(slash + 1);
        if (usage_) {
            usage_(*this, name);
        }
        else {
            out << "Usage: " << name << " ";
            if (!necessary_.empty()) for (auto & o : necessary_) {
                out << o.lname_;
                if (!o.default_.empty()) out << "=" << o.default_;
                out << " ";
            }
            out << "[OPTIONS]..." << std::endl;
            out << "Options: " << std::endl;
            auto print_opt = [&](auto & o) {
                out << " ";
                if (o.sname_ != nullptr) {
                    out << " " << o.sname_ << ",";
                }
                out << " " << o.lname_ << " \t" << o.description_;
                if (!o.default_.empty()) out << "[=" << o.default_ << "]";
                out << std::endl;
            };
            for (auto & o : necessary_) { print_opt(o); }
            for (auto & o : optional_ ) { print_opt(o); }
        }
    }

    void print_usage(void) const {
        this->print_usage(std::cout);
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

    template <typename T>
    int exec(T && out, int argc, char const * const argv[]) {
        if (argc >= 1) path_ = argv[0];
        if (argc <= 1) {
            this->print_usage(std::forward<T>(out));
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
                this->print_usage(std::forward<T>(out));
            }
            else for (ST_opt & e : exec_list) {
                e.hd_(*this, e.cm_);
            }
        }
        return 0;
    }

    int exec(int argc, char const * const argv[]) {
        return this->exec(std::cout, argc, argv);
    }
};

} // namespace cmdline