#pragma once
#include <string>

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
	namespace filesystem
	{
		typedef char value_type;
		typedef std::basic_string<value_type>  string_type;
		typedef string_type::size_type  size_type;

		class path
		{
		private:
			string_type m_pathname;

			const value_type* const separators = "/\\";
			const value_type separator = '/';
			const value_type* separator_string = "/";
			const value_type preferred_separator = '\\';
			const value_type* preferred_separator_string = "\\";
			const value_type colon = ':';
			const value_type dot = '.';
			const value_type questionmark = '?';

		public:

			path() {}
			path(const path& p) : m_pathname(p.m_pathname) {}
			path(const string_type& s) : m_pathname(s) {}
			path(string_type& s) : m_pathname(s) {}
			path(const value_type* s) { m_pathname.assign(s); }
			path(value_type* s) { m_pathname.assign(s);}

			path& operator=(const path& p){ m_pathname = p.m_pathname; return *this; }
			path& operator=(const value_type* ptr) { m_pathname = ptr; return *this; }
			path& operator=(value_type* ptr) { m_pathname = ptr; return *this; }
			path& operator=(const string_type& s) { m_pathname = s; return *this; }
			path& operator=(string_type& s) { m_pathname = s; return *this; }

			path& operator+=(const path& p) { m_pathname += p.m_pathname; return *this; }
			path& operator+=(const value_type* ptr) { m_pathname += ptr; return *this; }
			path& operator+=(value_type* ptr) { m_pathname += ptr; return *this; }
			path& operator+=(const string_type& s) { m_pathname += s; return *this; }
			path& operator+=(string_type& s) { m_pathname += s; return *this; }
			path& operator+=(value_type c) { m_pathname += c; return *this; }

			path& operator/=(const value_type* ptr);
			path& operator/=(value_type* ptr)
			{
				return this->operator/=(const_cast<const value_type*>(ptr));
			}
			path& operator/=(const string_type& s) { return this->operator/=(path(s)); }
			path& operator/=(string_type& s) { return this->operator/=(path(s)); }
			path& operator/=(const path& p)
			{
				if (p.empty())
					return *this;
				if (this == &p)  // self-append
				{
					path rhs(p);
					if (!is_separator(rhs.m_pathname[0]))
						m_append_separator_if_needed();
					m_pathname += rhs.m_pathname;
				}
				else
				{
					if (!is_separator(*p.m_pathname.begin()))
						m_append_separator_if_needed();
					m_pathname += p.m_pathname;
				}
				return *this;
			}

			bool empty() const
			{
				return m_pathname.empty();
			}

			const string_type& native() const
			{ 
				return m_pathname; 
			}

			const string_type& string() const
			{
				return m_pathname;
			}

			std::wstring wstring() const
			{
				if (m_pathname.empty())
					return std::wstring(L"");

				int size = MultiByteToWideChar(CP_ACP, 0, (char*)m_pathname.c_str(), m_pathname.length(), NULL, 0);
				if (size <= 0)
					return std::wstring(L"");

				wchar_t* wc = new wchar_t[size + 1];
				if (wc == NULL)
					return std::wstring(L"");

				memset(wc, 0, (size + 1) * sizeof(wchar_t));
				MultiByteToWideChar(CP_ACP, 0, (char*)m_pathname.c_str(), m_pathname.length(), wc, size);

				std::wstring wcharString(wc);
				delete[] wc;

				return wcharString;
			}


			path path::filename() const
			{
				size_type pos(filename_pos(m_pathname, m_pathname.size()));
				return (m_pathname.size()
					&& pos
					&& is_separator(m_pathname[pos])
					&& !is_root_separator(m_pathname, pos))
					? path(".")
					: path(m_pathname.c_str() + pos);
			}

			path path::extension() const
			{
				path name(filename());
				if (strcmp(name.string().c_str(), ".") == 0 || strcmp(name.string().c_str(), "..") == 0) return path();
				size_type pos(name.m_pathname.rfind(dot));
				return pos == string_type::npos
					? path()
					: path(name.m_pathname.c_str() + pos);
			}

			path path::parent_path() const
			{
				size_type end_pos(m_parent_path_end());
				return end_pos == string_type::npos
					? path()
					: path(m_pathname.substr(0, end_pos));
			}

			path& path::remove_filename()
			{
				m_pathname.erase(m_parent_path_end());
				return *this;
			}

		private:

			bool is_letter(value_type c) const
			{
				return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
			}

			bool is_separator(value_type c) const
			{
				return c == separator || c == preferred_separator;
			}

			size_type filename_pos(const string_type & str,
				size_type end_pos) const // end_pos is past-the-end position
								   // return 0 if str itself is filename (or empty)
			{
				// case: "//"
				if (end_pos == 2
					&& is_separator(str[0])
					&& is_separator(str[1])) return 0;

				// case: ends in "/"
				if (end_pos && is_separator(str[end_pos - 1]))
					return end_pos - 1;

				// set pos to start of last element
				size_type pos(str.find_last_of(separators, end_pos - 1));

				if (pos == string_type::npos && end_pos > 1)
					pos = str.find_last_of(colon, end_pos - 2);

				return (pos == string_type::npos // path itself must be a filename (or empty)
					|| (pos == 1 && is_separator(str[0]))) // or net
					? 0 // so filename is entire string
					: pos + 1; // or starts after delimiter
			}

			bool is_root_separator(const string_type & str, size_type pos) const
				// pos is position of the separator
			{
				// subsequent logic expects pos to be for leftmost slash of a set
				while (pos > 0 && is_separator(str[pos - 1]))
					--pos;

				//  "/" [...]
				if (pos == 0)
					return true;

				//  "c:/" [...]
				if (pos == 2 && is_letter(str[0]) && str[1] == colon)
					return true;

				//  "//" name "/"
				if (pos < 3 || !is_separator(str[0]) || !is_separator(str[1]))
					return false;

				return str.find_first_of(separators, 2) == pos;
			}

			string_type::size_type path::m_parent_path_end() const
			{
				size_type end_pos(filename_pos(m_pathname, m_pathname.size()));

				bool filename_was_separator(m_pathname.size()
					&& is_separator(m_pathname[end_pos]));

				// skip separators unless root directory
				size_type root_dir_pos(root_directory_start(m_pathname, end_pos));
				for (;
					end_pos > 0
					&& (end_pos - 1) != root_dir_pos
					&& is_separator(m_pathname[end_pos - 1])
					;
					--end_pos) {
				}

				return (end_pos == 1 && root_dir_pos == 0 && filename_was_separator)
					? string_type::npos
					: end_pos;
			}

			size_type root_directory_start(const string_type & path, size_type size) const
				// return npos if no root_directory found
			{
				// case "c:/"
				if (size > 2
					&& path[1] == colon
					&& is_separator(path[2])) return 2;

				// case "//"
				if (size == 2
					&& is_separator(path[0])
					&& is_separator(path[1])) return string_type::npos;

				// case "\\?\"
				if (size > 4
					&& is_separator(path[0])
					&& is_separator(path[1])
					&& path[2] == questionmark
					&& is_separator(path[3]))
				{
					string_type::size_type pos(path.find_first_of(separators, 4));
					return pos < size ? pos : string_type::npos;
				}

				// case "//net {/}"
				if (size > 3
					&& is_separator(path[0])
					&& is_separator(path[1])
					&& !is_separator(path[2]))
				{
					string_type::size_type pos(path.find_first_of(separators, 2));
					return pos < size ? pos : string_type::npos;
				}

				// case "/"
				if (size > 0 && is_separator(path[0])) return 0;

				return string_type::npos;
			}

			string_type::size_type path::m_append_separator_if_needed()
			{
				if (!m_pathname.empty() && *(m_pathname.end() - 1) != colon &&
					!is_separator(*(m_pathname.end() - 1)))
				{
					string_type::size_type tmp(m_pathname.size());
					m_pathname += preferred_separator;
					return tmp;
				}
				return 0;
			}

		};
	}

}