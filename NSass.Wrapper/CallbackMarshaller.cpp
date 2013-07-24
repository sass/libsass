#include "callback_manager.hpp"
#include "CallbackMarshaller.hpp"

namespace NSass
{
	void CallbackMarshaller::SetFileAccessCallBack(IntPtr^ callBack)
	{
		FileAccessDelegate pointer = (FileAccessDelegate)callBack->ToPointer();
		NSass::CallbackManager::getInstance().set_file_access_callback(pointer);
	}

	void CallbackMarshaller::UnsetFileAccessCallBack()
	{
		NSass::CallbackManager::getInstance().unset_file_access_callback();
	}
}