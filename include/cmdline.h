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

namespace cmdline {

class parser;
using handle_t = void(parser const &, std::string const &);

struct option {
    char const * sname_;
    char const * lname_;
    char const * description_;
    bool         necessary_;
    std::string  default_;
    std::function<handle_t> handle_;
};

using options = std::deque<option>;

class parser {

    options necessary_, optional_;
    std::function<handle_t> usage_;
    std::string path_;

public:
    options &       necessary(void)       { return necessary_; }
    options const & necessary(void) const { return necessary_; }

    options &       optional(void)       { return optional_; }
    options const & optional(void) const { return optional_; }

    std::function<handle_t> &       usage(void)       { return usage_; }
    std::function<handle_t> const & usage(void) const { return usage_; }

    template <typename T>
    void print_usage(T && out) const {
        if (path_.empty()) {
            out << "Must has at least one argument (the path of current program)." << std::endl;
            return;
        }
        size_t slash = path_.find_last_of('\\');
        if (slash == std::string::npos) slash = path_.find_last_of('/');
        std::string name = path_.substr(slash + 1);
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
                out << "  " << o.sname_ << ", " << o.lname_ << "\t" << o.description_;
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

    void push(options && opts) {
        for (auto && o : opts) {
            options * list = &(o.necessary_ ? necessary_ : optional_);
            list->emplace_back(std::move(o));
        }
    }

    void clear(void) {
        necessary_.clear();
        optional_ .clear();
    }

    template <typename T>
    void exec(T && out, int argc, char const * const argv[]) {
        if (argc >= 1) path_ = argv[0];
        if (argc <= 1) this->print_usage(std::forward<T>(out));
        else {
            struct ST_opt {
                std::function<handle_t> const & hd_;
                std::string cm_;
            };
            std::deque<ST_opt> exec_list;
            size_t c_nec = 0;
            for (int i = 1; i < argc; ++i) {
                std::string a = argv[i];
                size_t c = a.find_first_of('=');
                std::string o = a.substr(0, c);
                auto foreach = [&](auto const & cc, auto && fr) {
                    for (auto it = cc.begin(); it != cc.end(); ++it) {
                        if (o == it->sname_ || o == it->lname_) {
                            exec_list.emplace_back(ST_opt{
                                it->handle_,
                                c == std::string::npos ? it->default_ : a.substr(c + 1)
                            });
                            fr();
                        }
                    }
                };
                foreach(necessary_, [&c_nec] { ++c_nec; });
                foreach(optional_ , [] {});
            }
            if (c_nec != necessary_.size() || exec_list.empty()) this->print_usage(std::forward<T>(out));
            else for (auto & e : exec_list) {
                e.hd_(*this, e.cm_);
            }
        }
    }

    void exec(int argc, char const * const argv[]) {
        this->exec(std::cout, argc, argv);
    }
};

} // namespace cmdline