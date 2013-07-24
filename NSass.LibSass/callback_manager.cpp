#include <stdio.h>
#include "callback_manager.hpp"

namespace NSass
{
	CallbackManager& CallbackManager::getInstance()
	{
		static CallbackManager instance;

		return instance;
	}

	CallbackManager::CallbackManager()
	{
		_fileAccessDelegate = NULL;
	}

	void CallbackManager::set_file_access_callback(FileAccessDelegate callBack)
	{
		_fileAccessDelegate = callBack;
	}

	void CallbackManager::unset_file_access_callback()
	{
		_fileAccessDelegate = NULL;
	}

	void CallbackManager::trigger_file_access_callback(const char* path)
	{
		if (_fileAccessDelegate != NULL)
		{
			_fileAccessDelegate(path);
		}
	}
}