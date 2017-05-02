#pragma once
namespace mars_boost {} namespace boost = mars_boost; namespace mars_boost
{
	namespace filesystem
	{
		class file_status
		{
		public:

		};

		class path
		{
		private:
			std::string mStrPath;
		public:
			path()
			{

			}

			path(const std::string & strPath)
			{
				mStrPath = strPath;
			}

			path(const char * buffer)
			{
				mStrPath.assign(buffer);
			}

			const std::string & string()
			{
				return mStrPath;
			}

			path filename()
			{
				return *this;
			}


		};

		class directory_iterator
		{
		public:
			directory_iterator(){}

			directory_iterator(const path & _path)
			{

			}

			directory_iterator operator++ ()
			{
				return *this;
			}

			directory_iterator operator++(int)
			{}

			path path()
			{
				boost::filesystem::path p;
				return p;
			}

			bool operator==(const directory_iterator &other)const {
				return false;
			}

			bool operator!=(const directory_iterator &other)const {
				return false;
			}

			directory_iterator* operator->()
			{
				return this;
			}

			file_status status()
			{
				return file_status();
			}
		};

		

		inline
			bool is_directory(const path& p) {
			return false;
		}

		inline
			bool is_directory(file_status & status) {
			return false;
		}

		inline bool is_regular_file(file_status f) {
			return false;
		}

		inline
			bool remove(const path& p) {
			return false;
		}

		inline
			bool exists(const path& p) {
			return false;
		}


		inline
			std::time_t last_write_time(const path& p)
		{
			std::time_t t = 0;
			return t;
		}

		inline
			mars_boost::uintmax_t file_size(const path& p)
		{
			return 0;
		}

		
		inline
			bool create_directory(const path& p) {
			return false;
		}

		inline
			bool create_directories(const path& p) {
			return false;
		}

	}

}