using namespace System;

namespace NSass
{
	public ref class CallbackMarshaller
	{
		public:
			static void SetFileAccessCallBack(IntPtr^ callBack);
			static void UnsetFileAccessCallBack();
	};
}