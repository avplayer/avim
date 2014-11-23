#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <fstream>

namespace avim {

	class ini {

		bool m_modified;
		short m_err_code;
		std::string m_ini_file;
		boost::property_tree::ptree m_pt;

	public:
		ini(const std::string& ini_file);
		~ini();

		template<typename value_type>
		value_type get(const std::string& path);

		template<typename value_type>
		void put(const std::string& path, const value_type& value);
	};

	template<typename value_type>
	value_type ini::get(const std::string& path)
	{
		if (m_err_code) {
			return value_type();
		}
    // path不存在时，异常处理
    try {
      return m_pt.get<value_type>(path);
    } catch (boost::property_tree::ptree_error error) {
      // FIXME: 这里异常返回啥比较好？
      return error.what();
    }
	}

	template<typename value_type>
	void ini::put(const std::string& path, const value_type& value)
	{
		if (m_err_code) {
			std::ofstream of(m_ini_file);
			if (!of) {
				return;
			}
			of.close();
			m_err_code = 0;
		}
		m_modified = true;
		m_pt.put(path, value);
	}
} // namespace avim
