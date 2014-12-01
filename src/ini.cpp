#include "ini.h"
#include <boost/property_tree/ini_parser.hpp>

namespace avim
{

	namespace fs = boost::filesystem;

	ini::ini(const boost::filesystem::path& ini_file)
		: m_modified(false)
		, m_ini_file(ini_file)
	{
		fs::path p(ini_file);
		if (fs::exists(ini_file))
		{
			m_err_code = 0;
			boost::property_tree::ini_parser::read_ini(m_ini_file.string(), m_pt);
		}
		else
		{
			m_err_code = 1;
		}
	}

	ini::~ini()
	{
		write_to_file();
	}

	void ini::write_to_file()
	{
		if (m_modified)
		{
			boost::property_tree::ini_parser::write_ini(m_ini_file.string(), m_pt);
			m_modified = false;
		}
	}

}
