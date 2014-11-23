#include "ini.h"
#include <boost/property_tree/ini_parser.hpp>

namespace avim {

namespace fs = boost::filesystem;

ini::ini(const std::string& ini_file)
    : m_modified(false)
    , m_ini_file(ini_file)
{
    fs::path p(ini_file);
    if (fs::exists(p))
	{
        m_err_code = 0;
        boost::property_tree::ini_parser::read_ini(m_ini_file, m_pt);
    }
    else
	{
        m_err_code = 1;
    }
}

ini::~ini()
{
    if (m_modified)
	{
        boost::property_tree::ini_parser::write_ini(m_ini_file, m_pt);
        m_modified = false;
    }
}

}
