/*
    cpp-cmdline - Code covered by the MIT License
    Author: mutouyun (http://darkc.at)
*/

#pragma once

#include <list>
#include <string>
#include <functional>
#include <utility>
#include <iostream>

namespace cmdline {

class parser;
using handle_t = void(const parser&, const std::string&);

struct option
{
    const char* sname_;
    const char* lname_;
    const char* description_;
    bool        necessary_;
    std::string default_;
    std::function<handle_t> handle_;
};

using options = std::list<option>;

class parser
{
    options necessary_, optional_;
    std::function<handle_t> usage_;
    std::string path_;

public:
    options&       necessary(void)       { return necessary_; }
    const options& necessary(void) const { return necessary_; }

    options&       optional(void)       { return optional_; }
    const options& optional(void) const { return optional_; }

    std::function<handle_t>&       usage(void)       { return usage_; }
    const std::function<handle_t>& usage(void) const { return usage_; }

    template <typename T>
    void print_usage(T&& out) const
    {
        size_t slash = path_.find_last_of('\\');
        if (slash == std::string::npos) slash = path_.find_last_of('/');
        std::string name = path_.substr(slash + 1);
        if (usage_)
        {
            usage_(*this, name);
        }
        else
        {
            out << "Usage: " << name << " ";
            if (!necessary_.empty()) for (auto& o: necessary_)
            {
                out << o.lname_;
                if (!o.default_.empty()) out << "=" << o.default_;
                out << " ";
            }
            out  << "[OPTIONS]..." << std::endl;
            out << "Options: " << std::endl;
            auto print_opt = [&](auto& o)
            {
                out << "  " << o.sname_ << ", " << o.lname_ << "\t" << o.description_;
                if (!o.default_.empty()) out << "[=" << o.default_ << "]";
            };
            for (auto& o: necessary_) { print_opt(o); out << std::endl; }
            for (auto& o: optional_ ) { print_opt(o); out << std::endl; }
        }
    }

    void print_usage(void) const
    {
        this->print_usage(std::cout);
    }

    void push(options&& opts)
    {
        for (auto&& o: opts)
        {
            options* list = &(o.necessary_ ? necessary_ : optional_);
            list->push_back(std::move(o));
        }
    }

    void exec(int argc, const char * const argv[])
    {
        path_ = argv[0];
        if (argc == 1) this->print_usage();
        else
        {
            struct ST_opt
            {
                const std::function<handle_t>& hd_;
                std::string cm_;
            };
            std::list<ST_opt> exec_list;
            size_t c_nec = 0;
            for (int i = 1; i < argc; ++i)
            {
                std::string a = argv[i];
                size_t c = a.find_first_of('=');
                std::string o = a.substr(0, c);
                auto foreach = [&](const auto& cc, auto fr)
                {
                    for (auto it = cc.begin(); it != cc.end(); ++it)
                    {
                        if (o == it->sname_ || o == it->lname_)
                        {
                            exec_list.push_back(ST_opt
                            {
                                it->handle_, 
                                c == std::string::npos ? it->default_ : a.substr(c + 1)
                            });
                            fr();
                        }
                    }
                };
                foreach(necessary_, [&c_nec]{ ++c_nec; });
                foreach(optional_ , []{});
            }
            if (c_nec != necessary_.size() || exec_list.empty()) this->print_usage();
            else for (auto& e: exec_list)
            {
                e.hd_(*this, e.cm_);
            }
        }
    }
};

} // namespace cmdline