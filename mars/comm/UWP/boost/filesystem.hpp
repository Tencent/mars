#pragma once
#include <boost/detail/bitmask.hpp>
#include <comm/UWP/boost/path.hpp>

#if defined(_MSC_VER) // || (defined(__GLIBCXX__) && __GLIBCXX__ >= 20110325)
#  define BOOST_FILESYSTEM_STRICMP _stricmp
#else
#  define BOOST_FILESYSTEM_STRICMP strcmp
#endif

namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
	namespace filesystem
	{
		enum file_type
		{
			status_error,
#   ifndef BOOST_FILESYSTEM_NO_DEPRECATED
			status_unknown = status_error,
#   endif
			file_not_found,
			regular_file,
			directory_file,
			// the following may not apply to some operating systems or file systems
			symlink_file,
			block_file,
			character_file,
			fifo_file,
			socket_file,
			reparse_file,  // Windows: FILE_ATTRIBUTE_REPARSE_POINT that is not a symlink
			type_unknown,  // file does exist, but isn't one of the above types or
						   // we don't have strong enough permission to find its type

			_detail_directory_symlink  // internal use only; never exposed to users
		};

		enum perms
		{
			no_perms = 0,       // file_not_found is no_perms rather than perms_not_known

									// POSIX equivalent macros given in comments.
									// Values are from POSIX and are given in octal per the POSIX standard.
			
			// permission bits
			
			owner_read = 0400,  // S_IRUSR, Read permission, owner
			owner_write = 0200, // S_IWUSR, Write permission, owner
			owner_exe = 0100,   // S_IXUSR, Execute/search permission, owner
			owner_all = 0700,   // S_IRWXU, Read, write, execute/search by owner
			
			group_read = 040,   // S_IRGRP, Read permission, group
			group_write = 020,  // S_IWGRP, Write permission, group
			group_exe = 010,    // S_IXGRP, Execute/search permission, group
			group_all = 070,    // S_IRWXG, Read, write, execute/search by group
			
			others_read = 04,   // S_IROTH, Read permission, others
			others_write = 02,  // S_IWOTH, Write permission, others
			others_exe = 01,    // S_IXOTH, Execute/search permission, others
			others_all = 07,    // S_IRWXO, Read, write, execute/search by others

			all_all = 0777,     // owner_all|group_all|others_all

			// other POSIX bits

			set_uid_on_exe = 04000, // S_ISUID, Set-user-ID on execution
			set_gid_on_exe = 02000, // S_ISGID, Set-group-ID on execution
			sticky_bit = 01000, // S_ISVTX,
					// (POSIX XSI) On directories, restricted deletion flag 
					// (V7) 'sticky bit': save swapped text even after use 
					// (SunOS) On non-directories: don't cache this file
					// (SVID-v4.2) On directories: restricted deletion flag
					// Also see http://en.wikipedia.org/wiki/Sticky_bit

			perms_mask = 07777,     // all_all|set_uid_on_exe|set_gid_on_exe|sticky_bit
							
			perms_not_known = 0xFFFF, // present when directory_entry cache not loaded

																  // options for permissions() function
										
			add_perms = 0x1000,     // adds the given permission bits to the current bits
			remove_perms = 0x2000,  // removes the given permission bits from the current bits;
													// choose add_perms or remove_perms, not both; if neither add_perms
													// nor remove_perms is given, replace the current bits with
													// the given bits.

			symlink_perms = 0x4000  // on POSIX, don't resolve symlinks; implied on Windows
		};

		BOOST_BITMASK(perms)

		class file_status
		{
		public:
			file_status() BOOST_NOEXCEPT
				: m_value(status_error), m_perms(perms_not_known) {}
			explicit file_status(file_type v) BOOST_NOEXCEPT
				: m_value(v), m_perms(perms_not_known) {}
			file_status(file_type v, perms prms) BOOST_NOEXCEPT
				: m_value(v), m_perms(prms) {}

			//  As of October 2015 the interaction between noexcept and =default is so troublesome
			//  for VC++, GCC, and probably other compilers, that =default is not used with noexcept
			//  functions. GCC is not even consistent for the same release on different platforms.

			file_status(const file_status& rhs) BOOST_NOEXCEPT
				: m_value(rhs.m_value), m_perms(rhs.m_perms) {}
			file_status& operator=(const file_status& rhs) BOOST_NOEXCEPT
			{
				m_value = rhs.m_value;
				m_perms = rhs.m_perms;
				return *this;
			}

			// observers
			file_type  type() const BOOST_NOEXCEPT { return m_value; }
			perms      permissions() const BOOST_NOEXCEPT { return m_perms; }

			// modifiers
			void       type(file_type v) BOOST_NOEXCEPT { m_value = v; }
			void       permissions(perms prms) BOOST_NOEXCEPT { m_perms = prms; }

			bool operator==(const file_status& rhs) const BOOST_NOEXCEPT
			{
				return type() == rhs.type() &&
					permissions() == rhs.permissions();
			}
			bool operator!=(const file_status& rhs) const BOOST_NOEXCEPT
			{
				return !(*this == rhs);
			}

		private:
			file_type   m_value;
			enum perms  m_perms;
		};

		struct handle_wrapper
		{
			HANDLE handle;
			handle_wrapper(HANDLE h)
				: handle(h) {}
			~handle_wrapper()
			{
				if (handle != INVALID_HANDLE_VALUE)
					CloseHandle(handle);
			}
		};
		
		inline
			perms make_permissions(const path& p, DWORD attr)
		{
			perms prms = perms::owner_read | perms::group_read | perms::others_read;
			if ((attr & FILE_ATTRIBUTE_READONLY) == 0)
				prms |= perms::owner_write | perms::group_write | perms::others_write;
			if (BOOST_FILESYSTEM_STRICMP(p.extension().string().c_str(), ".exe") == 0
				|| BOOST_FILESYSTEM_STRICMP(p.extension().string().c_str(), ".com") == 0
				|| BOOST_FILESYSTEM_STRICMP(p.extension().string().c_str(), ".bat") == 0
				|| BOOST_FILESYSTEM_STRICMP(p.extension().string().c_str(), ".cmd") == 0)
				prms |= perms::owner_exe | perms::group_exe | perms::others_exe;
			return prms;
		}

		inline
			file_status status(const path& p)
		{
			std::wstring ws = p.wstring();
			WIN32_FILE_ATTRIBUTE_DATA fad;
			BOOL ret = GetFileAttributesExW(ws.c_str(), GetFileExInfoStandard, &fad);
			if (ret != TRUE)
			{
				return file_status(file_not_found, no_perms);
			}

			DWORD attr = fad.dwFileAttributes;
			if (attr == 0xFFFFFFFF)
			{
				return file_status(file_not_found, no_perms);
			}

			return (attr & FILE_ATTRIBUTE_DIRECTORY)
				? file_status(directory_file, make_permissions(p, attr))
				: file_status(regular_file, make_permissions(p, attr));
		}
		
		inline
			bool is_directory(const path& p)
		{
			file_status f = status(p);
			return f.type() == directory_file;
		}

		inline
			bool is_directory(file_status f)
		{
			return f.type() == directory_file;
		}

		inline
			bool is_regular_file(file_status f)
		{
			return f.type() == regular_file;
		}

		inline
			bool exists(const path& p)
		{
			file_status f = status(p);
			return f.type() != status_error && f.type() != file_not_found;
		}

		inline
			mars_boost::uintmax_t file_size(const path& p)
		{
			std::wstring ws = p.wstring();
			WIN32_FILE_ATTRIBUTE_DATA fad;
			BOOL ret = GetFileAttributesExW(ws.c_str(), GetFileExInfoStandard, &fad);
			if (ret != TRUE)
			{
				return static_cast<mars_boost::uintmax_t>(-1);
			}

			if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				return static_cast<mars_boost::uintmax_t>(-1);
			}

			return (static_cast<mars_boost::uintmax_t>(fad.nFileSizeHigh)
				<< (sizeof(fad.nFileSizeLow) * 8)) + fad.nFileSizeLow;
		}

		inline
			std::time_t to_time_t(const FILETIME & ft)
		{
			__int64 t = (static_cast<__int64>(ft.dwHighDateTime) << 32)
				+ ft.dwLowDateTime;
#   if !defined(BOOST_MSVC) || BOOST_MSVC > 1300 // > VC++ 7.0
			t -= 116444736000000000LL;
#   else
			t -= 116444736000000000;
#   endif
			t /= 10000000;
			return static_cast<std::time_t>(t);
		}

		inline
			std::time_t last_write_time(const path& p) 
		{
			CREATEFILE2_EXTENDED_PARAMETERS para;
			para.dwFileAttributes = NULL;
			para.dwFileFlags = FILE_FLAG_BACKUP_SEMANTICS;
			para.dwSecurityQosFlags = SECURITY_ANONYMOUS; // for windows store app
			para.lpSecurityAttributes = NULL;
			para.hTemplateFile = NULL;

			std::wstring ws = p.wstring();

			handle_wrapper hw(CreateFile2(ws.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, &para));
			
			if (hw.handle == INVALID_HANDLE_VALUE)
			{
				return std::time_t(-1);
			}

			FILETIME lwt;
			BOOL ret = GetFileTime(hw.handle, 0, 0, &lwt);
			return ret ==TRUE ?  to_time_t(lwt) : std::time_t(-1);
		}

		inline
			bool remove(const path& p)
		{
			bool  isSucc = false;
			file_status f = status(p);
			if (f.type() == status_error || f.type() == file_not_found)
			{
				return isSucc;
			}

			std::wstring ws = p.wstring();
			if (f.type() == directory_file || f.type() == _detail_directory_symlink)
			{
				if (RemoveDirectoryW(ws.c_str()) == TRUE)
					isSucc = true;
			}
			else
			{
				if (DeleteFileW(ws.c_str()) == TRUE) 
					isSucc = true;
			}

			return isSucc;
		}

		inline
			bool create_directory(const path& p)
		{
			std::wstring ws = p.wstring();
			bool ret = CreateDirectoryW(ws.c_str(), NULL);
			return ret == TRUE ? true : false;
		}

		inline
			bool create_directories(const path& p)
		{
			path filename(p.filename());
			if ((filename.native().size() == 1 && filename.native()[0] == '.')
				|| (filename.native().size() == 2
					&& filename.native()[0] == '.' && filename.native()[1] == '.'))
				return create_directories(p.parent_path());

			file_status p_status = status(p);
			if (p_status.type() == directory_file)
			{
				return false;
			}

			path parent = p.parent_path();
			if (!parent.empty())
			{
				file_status parent_status = status(parent);
				if (parent_status.type() == file_not_found)
				{
					if (!create_directories(parent))
						return false;
				}
			}

			return create_directory(p);
		}

		inline
			std::string wstring_to_string(const std::wstring & ws)	 
		{
			if (ws.empty())
				return std::string("");

			int size = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.length(), NULL, 0, NULL, NULL);
			if (size <= 0) 
				return std::string("");

			char* pszDst = new char[size + 1];
			if (pszDst == NULL) 
				return std::string("");

			memset(pszDst, 0, (size + 1) * sizeof(char));
			WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.length(), pszDst, size, NULL, NULL);

			std::string str(pszDst);
			delete[] pszDst;

			return str;
		}

		inline
			void dir_itr_close(void *& handle)
		{
			if (handle != 0)
			{
				FindClose(handle);
				handle = 0;
			}
		}

		class directory_entry
		{
		public:

			directory_entry() {}

			explicit directory_entry(const mars_boost::filesystem::path& p)
				: m_path(p), m_status(file_status()), m_symlink_status(file_status()) {}

			directory_entry(const mars_boost::filesystem::path& p,
				file_status st, file_status symlink_st = file_status())
				: m_path(p), m_status(st), m_symlink_status(symlink_st) {}

			directory_entry(const directory_entry& rhs)
				: m_path(rhs.m_path), m_status(rhs.m_status), m_symlink_status(rhs.m_symlink_status) {}

			directory_entry& operator=(const directory_entry& rhs)
			{
				m_path = rhs.m_path;
				m_status = rhs.m_status;
				m_symlink_status = rhs.m_symlink_status;
				return *this;
			}

			void assign(const mars_boost::filesystem::path& p,
				file_status st = file_status(), file_status symlink_st = file_status())
			{
				m_path = p; m_status = st; m_symlink_status = symlink_st;
			}

			void replace_filename(const mars_boost::filesystem::path& p,
				file_status st = file_status(), file_status symlink_st = file_status())
			{
				m_path.remove_filename();
				m_path /= p;
				m_status = st;
				m_symlink_status = symlink_st;
			}

			const mars_boost::filesystem::path&  path() const { return m_path; }
			operator const mars_boost::filesystem::path&() const { return m_path; }
			file_status   status() const { m_status = mars_boost::filesystem::status(m_path); return m_status; }
			file_status   symlink_status() const { return m_symlink_status; }

			bool operator==(const directory_entry& rhs) const { return m_path.string().compare(rhs.path().string()) == 0; }
			bool operator!=(const directory_entry& rhs) const { return m_path.string().compare(rhs.path().string()) != 0; }

		private:
			mars_boost::filesystem::path   m_path;
			mutable file_status       m_status; 
			mutable file_status       m_symlink_status;
		};

		struct dir_itr_imp
		{
			directory_entry  dir_entry;
			void*            handle;

			dir_itr_imp() : handle(0){}

			~dir_itr_imp()
			{
				dir_itr_close(handle);
			}
		};

		class directory_iterator
		{
		private:
			mars_boost::shared_ptr< dir_itr_imp >  m_imp;
			
			bool dir_itr_first(void *& handle, const path& dir, std::string & target, file_status & sf, file_status & symlink_sf)
			{
				std::wstring dirpath(dir.wstring());
				dirpath += (dirpath.empty()
					|| (dirpath[dirpath.size() - 1] != L'\\'
						&& dirpath[dirpath.size() - 1] != L'/'
						&& dirpath[dirpath.size() - 1] != L':')) ? L"\\*" : L"*";

				WIN32_FIND_DATAW data;
				if ((handle = FindFirstFileExW(dirpath.c_str(), FindExInfoStandard, &data, FindExSearchNameMatch, NULL, 0)) == INVALID_HANDLE_VALUE)
				{
					handle = 0;  // signal eof
					return false;
				}
				target = wstring_to_string(data.cFileName);
				if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
				{
					sf.type(status_error);
					symlink_sf.type(status_error);
					return false;
				}
				else
				{
					if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						sf.type(directory_file);
						symlink_sf.type(directory_file);
					}
					else
					{
						sf.type(regular_file);
						symlink_sf.type(regular_file);
					}
					sf.permissions(make_permissions(target, data.dwFileAttributes));
					symlink_sf.permissions(sf.permissions());
				}
				return true;
			}

			bool dir_itr_increment(void *& handle, std::string & target, file_status & sf, file_status & symlink_sf)
			{
				WIN32_FIND_DATAW data;
				if (FindNextFileW(handle, &data) == 0)// fails
				{
					dir_itr_close(handle);
					return false;
				}
				target = wstring_to_string(data.cFileName);
				if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
				{
					sf.type(status_error);
					symlink_sf.type(status_error);
					return false;
				}
				else
				{
					if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						sf.type(directory_file);
						symlink_sf.type(directory_file);
					}
					else
					{
						sf.type(regular_file);
						symlink_sf.type(regular_file);
					}
					sf.permissions(make_permissions(target, data.dwFileAttributes));
					symlink_sf.permissions(sf.permissions());
				}
				return true;
			}

			void directory_iterator_construct(directory_iterator& it, const path& p)
			{
				if (p.empty())
					return;

				string_type filename;
				file_status file_stat, symlink_file_stat;
				bool result = dir_itr_first(it.m_imp->handle, p, filename, file_stat, symlink_file_stat);

				if (!result)
				{
					it.m_imp.reset();
					return;
				}

				if (it.m_imp->handle == 0)
					it.m_imp.reset(); // eof, so make end iterator
				else // not eof
				{
					mars_boost::filesystem::path np(p);
					np /= filename;
					it.m_imp->dir_entry.assign(np, file_stat, symlink_file_stat);
					if (filename[0] == '.' // dot or dot-dot
						&& (filename.size() == 1
							|| (filename[1] == '.'
								&& filename.size() == 2)))
					{
						directory_iterator_increment(it);
					}
				}
			}

			void directory_iterator_increment(directory_iterator& it)
			{
				if (!it.m_imp.get())
					return;

				if (it.m_imp->handle == 0)
				{
					m_imp.reset();
					return;
				}

				string_type filename;
				file_status file_stat, symlink_file_stat;
				bool result;

				for (;;)
				{
					result = dir_itr_increment(it.m_imp->handle, filename, file_stat, symlink_file_stat);

					if (!result)  // happens if filesystem is corrupt, such as on a damaged optical disc
					{
						mars_boost::filesystem::path error_path(it.m_imp->dir_entry.path().parent_path());  // fix ticket #5900
						it.m_imp.reset();
						return;
					}

					if (it.m_imp->handle == 0)  // eof, make end
					{
						it.m_imp.reset();
						return;
					}

					if (!(filename[0] == '.' // !(dot or dot-dot)
						&& (filename.size() == 1
							|| (filename[1] == '.'
								&& filename.size() == 2))))
					{
						it.m_imp->dir_entry.replace_filename(
							filename, file_stat, symlink_file_stat);
						return;
					}
				}
			}

		public:
			directory_iterator() {}

			explicit directory_iterator(const path& p) : m_imp(new dir_itr_imp)
			{
				directory_iterator_construct(*this, p);
			}

			~directory_iterator() 
			{
				if (m_imp.get())
					m_imp.reset();
			}

			directory_iterator* operator->()
			{
				return this;
			}

			directory_iterator& operator++ ()
			{
				directory_iterator_increment(*this);
				return *this;
			}

			bool operator==(const directory_iterator &other)const { 
				if (m_imp.get() && other.m_imp.get())
				{
					return m_imp->dir_entry == other.m_imp->dir_entry;
				}
				else if (!m_imp.get() && !other.m_imp.get())
				{
					return true;
				}
				else
				{
					return false;
				}
			}

			bool operator!=(const directory_iterator &other)const { 
				if (m_imp.get() && other.m_imp.get())
				{
					return m_imp->dir_entry != other.m_imp->dir_entry;
				}
				else if (!m_imp.get() && !other.m_imp.get())
				{
					return false;
				}
				else
				{
					return true;
				}
			}

			path path()
			{
				return m_imp.get() ? m_imp->dir_entry.path() : mars_boost::filesystem::path();
			}

			file_status status()
			{
				return m_imp.get() ? m_imp->dir_entry.status() : file_status();
			}
		};
	}
}
