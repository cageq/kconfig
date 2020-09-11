#pragma once 

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <string.h>
#include <vector>
#include <iterator>
#include <regex>
namespace kconfig
{
	enum ValueType {
		VAL_NORMAL = 0,
		VAL_ARRAY,
	};
	struct Value {
		Value() :type(0) { }
		Value(int t, const std::string & v) :type(t), value(v) { }
		int type;
		std::string value;
	};
	template <typename T>
		struct TypeTrait {
			T operator () (const std::string & value) {
				return static_cast<T> (value.c_str());
			}
		};


	template <>
		struct TypeTrait<uint32_t>
		{
			int operator () (const std::string & value) {
				return atoi(value.c_str());
			}
		};


	template <>
		struct TypeTrait<int32_t>
		{
			int operator () (const std::string & value) {
				return atoi(value.c_str());
			}
		};

	template <>
		struct TypeTrait<float>
		{
			float operator () (const std::string & value) {
				return atof(value.c_str());
			}
		};


	template <>
		struct TypeTrait<bool>
		{
			bool operator () (const std::string & value) {
				std::vector<std::string> trueVars = { "yes","1","true" };
				std::string v = value;
				std::transform(value.begin(), value.end(), v.begin(), ::tolower);
				for (auto &val : trueVars) {
					if(v.compare(val) == 0) return true;
				}
				return false;
			}
		};


	class KConfig
	{

		const char * default_segment = "all";

		public:

		KConfig(bool needSeg = true, const std::string & separator = " ,") :m_need_segment(needSeg), m_separator(separator) {
		}

		typedef std::function<bool(int)> FilterFunc;
		static bool is_space(int ch)
		{
			return  (ch == ' ' || ch == 160 || ch == '\n' || ch == '\r' || ch == '\t');
		}

		// trim from start
		static inline std::string &ltrim(std::string &s, FilterFunc filter = nullptr) {
			if (!filter)
			{
				filter = &KConfig::is_space;
			}
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(filter)));
			//s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), filter));
			return s;
		}

		// trim from end
		static inline std::string &rtrim(std::string &s, FilterFunc filter = nullptr) {
			if (!filter)
			{
				filter = &KConfig::is_space;
			}
			s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(filter)).base(), s.end());
			//s.erase(std::find_if(s.rbegin(), s.rend(),  filter).base(), s.end());
			return s;
		}

		static inline std::string tolower(std::string  s)
		{
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
			return s;
		}

		static void  split(const std::string & s, const std::string& delims, std::vector<std::string> & rst)
		{
			unsigned int beg = 0;
			for (unsigned int i = 0; i < s.length(); i++) {
				if (strchr(delims.c_str(), s[i]) != NULL)
				{
					if (beg < i)
					{
						rst.push_back(std::string(s.c_str(), beg, i - beg));
					}
					beg = i + 1;
				}
			}
			if (beg < s.length())
			{
				rst.push_back(std::string(s.c_str(), beg, s.length() - beg));
			}
		}

		static std::vector<std::string>  split(const std::string & s, const std::string& delims)
		{
			std::vector<std::string> rst;
			split(s, delims, rst);
			return rst;
		}

		static void split(const std::string & s, const std::string& delims,
				std::function<void(const std::string& token)> func)
		{
			unsigned int beg = 0;
			for (unsigned int i = 0; i < s.length(); i++) {
				if (strchr(delims.c_str(), s[i]) != NULL)
				{
					if (beg < i)
					{
						func(std::string(std::string(s.c_str(), beg, i - beg)));
					}
					beg = i + 1;
				}
			}
			if (beg < s.length())
			{
				func(std::string(std::string(s.c_str(), beg, s.length() - beg)));
			}
		}


		// trim from both ends
		static inline std::string &trim(std::string &s, FilterFunc filter = nullptr) {
			return ltrim(rtrim(s, filter), filter);
		}

		bool read(const std::string &config) {

			std::ifstream infile{ config };
			if(!infile.good()){ 
				std::cerr<<"config file not exist!" << config<<std::endl;
				return false; 
			}
			std::string seg = default_segment;
			int lineNum = 0;
			std::string line;
			while (std::getline(infile, line))
			{
				lineNum++;
				m_text.push_back(line);
				line = trim(line);
				//std::cout << line << std::endl; 

				std::smatch lineMatch;
				if (line[0] == '#') {
					//is comment 
					//    std::cout << "comment :" << line << std::endl; 
				}
				else if (std::regex_match(line, lineMatch, std::regex("\\[(.*)\\]")))
				{
					std::string curSeg = trim(line, [](int ch) { return (ch == ' ' || ch == 160 || ch == '\t' || ch == '[' || ch == ']');  });
					if (!curSeg.empty()) {
						seg = curSeg;
					}
					std::cout << "segment is " << seg << std::endl;
				}
				else {
					std::string::size_type pos = line.find('=');
					if (pos != std::string::npos) {
						std::string key = line.substr(0, pos);
						if (!key.empty())
						{
							//std::cout <<  " seg is " << seg << "  key is " << key  << std::endl; 
							std::string segKey = m_need_segment ? (seg + "." + trim(key)) : trim(key);
							std::string value = line.substr(pos + 1);
							//std::cout << "seg is :"<< seg << " key is :" << segKey<<  " value is:" << value << std::endl; 
							if (!m_need_segment && m_configs.find(segKey) != m_configs.end()) {
								//std::cerr<<"[WARNING] duplicate key :"<<trim(key)<< ", using segment mode " << std::endl; 
							}
							if (m_configs.find(segKey) != m_configs.end()) {
								//m_configs[segKey] ; 
								Value val = m_configs[segKey];
								val.type = VAL_ARRAY;
								if (val.value.empty()) {
									val.value = trim(value);
								}
								else {
									val.value += m_separator + trim(value);
								}
								m_configs[segKey] = val;
							}
							else {
								std::smatch sm;
								if (std::regex_match(trim(value), sm, std::regex("\\[(.*)\\]"))) {
									//std::cout << "found array " << value  << " match size " << sm.size() << std::endl; 
									//for (unsigned i=0; i<sm.size(); ++i) {  
									//        std::cout   << sm[i] << "\n ";  
									//}  
									m_configs[segKey] = Value(VAL_ARRAY, sm[1]);
								}
								else
								{
									m_configs[segKey] = Value(VAL_NORMAL, trim(value));
								}
							}

						}
						else {
							std::cerr << "[WARNING] illegal line," << lineNum << " :" << line << std::endl;
						}
					}

				};

			}
			return true;
		};


		typedef std::function<void(const std::string &) > StrResultFunc;
		std::string get_string(const char * key, StrResultFunc resultFunc, const std::string & defVal = "") {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end()) {
				resultFunc(rst->second.value);
				return rst->second.value;
			}
			return defVal;
		}

		typedef std::function<void(int) > IntResultFunc;
		int  get_int(const char * key, IntResultFunc resultFunc, int  defVal = 0) {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end())
			{
				int result = atoi(rst->second.value.c_str());
				resultFunc(result);
				return result;
			}
			return defVal;
		}
		template <class T> 
			T as(){ 

				return T(); 
			}

		typedef std::function<void(float) > FloatResultFunc;
		float get_float(const char * key, FloatResultFunc resultFunc, float defVal = 0.0) {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end())
			{
				int result = atof(rst->second.value.c_str());
				resultFunc(result);
				return result;
			}
			return defVal;
		}

		typedef std::function<void(bool) > BoolResultFunc;
		bool get_bool(const char * key, BoolResultFunc resultFunc, float defVal = false) {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end()) {
				std::vector<std::string> trueVars = { "yes","1","true" };
				for (auto &val : trueVars)
				{
					int ret = tolower(rst->second.value).compare(val);
					if (ret == 0)
					{
						resultFunc(true);
						return true;
					}
				}
				resultFunc(false);
			}
			return defVal;
		}

		std::string get_string(const char * key, const std::string & defVal = "") {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end())
			{
				return rst->second.value;
			}
			return defVal;
		}


		template <class T>
			T get(const char * key, T defVal = T()) {
				auto rst = m_configs.find(key);
				if (rst != m_configs.end()) {
					return TypeTrait<T>()(rst->second.value);
				}
				return defVal;
			}

		template <class T>
			void get(const char * key, std::function<void(const T &)> proc) {
				auto rst = m_configs.find(key);
				if (rst != m_configs.end())
				{
					proc(TypeTrait<T>()(rst->second.value));
				}
			}

		template <class T>
			void  get_array(const char * key, std::function<void(const T &)> proc) {
				auto rst = m_configs.find(key);
				if (rst != m_configs.end() && rst->second.type == VAL_ARRAY) {
					std::vector<std::string> items;
					split(rst->second.value, m_separator, items);
					for (auto &item : items)
					{
						proc(TypeTrait<T>()(item));
					}
				}

			}


		template <class T>
			void  get_array(const char * key, std::vector<T> &result) {
				auto rst = m_configs.find(key);
				if (rst != m_configs.end() && rst->second.type == VAL_ARRAY) {
					std::vector<std::string> items;
					split(rst->second.value, m_separator, items);
					for (auto &item : items) {
						result.push_back(TypeTrait<T>()(item));
					}
				}
			}

		template <class T>
			std::vector<T> get_array(const char * key)
			{
				auto rst = m_configs.find(key);
				if (rst != m_configs.end() && rst->second.type == VAL_ARRAY) {
					std::vector<T> result;
					std::vector<std::string> items;
					split(rst->second.value, m_separator, items);
					for (auto &item : items) {
						result.push_back(TypeTrait<T>()(item));
					}
					return result;
				}
				return std::vector<T>();
			}

		void get_str_array(const char * key, std::vector<std::string> & items) {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end() && rst->second.type == VAL_ARRAY) {
				split(rst->second.value, m_separator, items);
			}
		}


		std::vector<std::string> get_str_array(const char * key) {
			auto rst = m_configs.find(key);
			if (rst->second.type == VAL_ARRAY) {
				return split(rst->second.value, m_separator);
			}
			return std::vector<std::string>();
		}

		bool get_bool(const char * key, bool defVal = false) {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end()) {
				std::vector<std::string> trueVars = { "yes","1","true" };
				for (auto &val : trueVars) {
					int ret = tolower(rst->second.value).compare(val);
					if (ret == 0) {
						return true;
					}
				}
			}
			return defVal;
		}

		int get_int(const char * key, int defVal = 0) {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end()) {
				return atoi(rst->second.value.c_str());
			}
			return defVal;
		}

		float get_float(const char * key, float defVal = 0.0) {
			auto rst = m_configs.find(key);
			if (rst != m_configs.end()) {
				return atof(rst->second.value.c_str());
			}
			return defVal;
		}

		private:
		std::vector<std::string> m_text;
		typedef std::unordered_map<std::string, Value >  HashMap;
		bool m_need_segment;
		std::string m_separator;
		HashMap  m_configs;
	};
}

