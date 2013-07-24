namespace NSass
{
	typedef void (__stdcall *FileAccessDelegate)(const char* path);

	class CallbackManager
	{
		private:
			FileAccessDelegate _fileAccessDelegate;

			CallbackManager();

			CallbackManager(CallbackManager const&);
			void operator=(CallbackManager const&);

		public:
			static CallbackManager& getInstance();

			void set_file_access_callback(FileAccessDelegate callBack);
			void unset_file_access_callback();
			void trigger_file_access_callback(const char* path);
	};
}