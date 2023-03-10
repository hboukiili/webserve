#include <iostream>
#include "parse.hpp"
#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>

std::pair<std::string, std::string> split(std::string s)
{
    std::string key, value;
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    key = s.substr(0, s.find("="));
    value = s.substr(s.find("=") + 1, s.length());
    return std::make_pair(key, value);
}

server &   server_parse(server & s, const std::pair<std::string, std::string> &m,
    std::map<std::string, std::string> &b)
{
    if (m.first == "port")
        s.set_port(m.second);
    else if (m.first == "host")
        s.set_host(m.second);
    else if (m.first == "server_name")
        s.set_server_name(m.second);
    else if (m.first == "error_page")
        s.set_error_page(m.second);
    else if (m.first == "body_size_limit")
        s.set_body_size(m.second);
    else if (m.first == "cgi")
    {
        b.insert(std::make_pair(m.second.substr(0, m.second.find("/")),
            m.second.substr(m.second.find("/"), m.second.length())));
        s.set_cgi(b);
    }
	return s;
}

location & location_parse(std::pair<std::string, std::string> &m, location &l)
{
    if (m.first == "method")
    {
		size_t                      pos;
    	std::vector<std::string>    h;

        pos = m.second.find("GET");
        if (pos != std::string::npos)
            h.push_back("GET");
        pos = m.second.find("POST");
        if (pos != std::string::npos)
            h.push_back("POST");
        pos = m.second.find("DELETE");
        if (pos != std::string::npos)
            h.push_back("DELETE");
        l.set_method(h);
    }
    else if (m.first == "root")
        l.set_root(m.second);
    else if (m.first == "autoindex")
        l.set_autoindex(m.second);
    else if (m.first == "default")
        l.set_default(m.second);
    else if (m.first == "upload")
        l.set_upload(m.second);
	else if (m.first == "redirect")
	{
		std::map<std::string, std::string> p;
		std::string first = m.second.substr(0, m.second.find("http"));
		std::string second = m.second.substr(m.second.find("http"), m.second.length());
		p.insert(std::make_pair(first, second));
		l.set_redirect(p);
	}
    return l;
}

int main(int ac, char **av)
{
    if (ac != 2)
        return 1;
    std::vector<server>                 _s;
    server                              s;
    std::ifstream                       ifs(av[1]);
    std::string                         line;
    std::pair<std::string, std::string> m;
    std::map<std::string, std::string>  b;
    std::map<std::string, location>     _l;
    location 							l;
    std::string 						temp;
    size_t 								pos;

	s.flg = false;
    while (std::getline(ifs, line))
    {
        pos = line.find("server");
        if (pos != std::string::npos && !s.flg)
        {
            s.flg = true;
            while(std::getline(ifs, line) && s.flg)
            {
                if (!line.empty())
                    m = split(line);
                if (m.first == "}")
                    s.flg = false;
                else if (m.first != "location")
                    s = server_parse(s, m, b);
                else if (m.first == "location")
                {
                    if (line.find("["))
                        l.flg = true;
                    temp = m.second.substr(0, m.second.find("["));
                    while (std::getline(ifs, line) && l.flg)
                    {
                        m = split(line);
                        l = location_parse(m, l);
                        if (m.first == "]")
						{
                            l.flg = false;
							break ;
						}
                    }
                    _l.insert(std::make_pair(temp, l));
                }
                // std::cout << line << std::endl;
                // if (m.first.find('{'))
                // {
                //     write(2, "error '{'\n", 11);
                //     exit(1);
                // }
            }
            if (!b.empty())
            {
                s.set_cgi(b);
                b.clear();
            }
            if (!_l.empty())
            {
                s.set_location(_l);
                _l.clear();
            }
            _s.push_back(s);
        }
        // else if (!line.find("server {") && s.flg)
        // {
        //     write(2, "error '{'\n", 11);
        //     exit(1);
        // }
    }
    // test parse
    size_t i = 0;
    size_t o = 0;
    while(i < _s.size())
    {   
        std::cout << _s[i].get_port() << std::endl;
        std::cout << _s[i].get_host() << std::endl;
        std::cout << _s[i].get_server_name() << std::endl;
        std::cout << _s[i].get_body_size() << std::endl;
        std::cout << _s[i].get_error_page() << std::endl;
        std::map<std::string, std::string>::iterator it;
        it = _s[i].get_cgi().begin();
        while (it != _s[i].get_cgi().end())
        {
            std::cout << it->first << "  " << it->second << std::endl;
            it++;
        }
        std::map<std::string, location>::iterator lit;
        lit = _s[i].get_location().begin();
        while(lit != _s[i].get_location().end())
        {
            std::cout << "location : "<< lit->first << std::endl;
            std::cout << "method = ";
            std::vector<std::string>::iterator mit;
            mit = lit->second.get_method().begin();
            while (mit != lit->second.get_method().end())
            {
                std::cout << *mit << " ";
                mit++;
            }
            std::cout << std::endl;
            std::cout << "root = " << lit->second.get_root() << std::endl;
            std::cout << "autoindex = " << lit->second.get_autoindex() << std::endl;
            std::cout << "_default = " << lit->second.get_default() << std::endl;
            std::cout << "upload = " << lit->second.get_upload() << std::endl;
			std::cout << "redirect = ";
			std::map<std::string, std::string>::iterator rit = lit->second.get_redirect().begin();
			std::cout << rit->first << "  " << rit->second << std::endl;
            std::cout << "-------------------------\n";
            lit++;
        }
        std::cout << "=-=-=-=-=-=-=-\n" ;
        i++;
    }
}