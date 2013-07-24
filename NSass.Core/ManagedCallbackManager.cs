using System.Runtime.InteropServices;

namespace NSass
{
	public static class ManagedCallbackManager
	{
		[UnmanagedFunctionPointer(CallingConvention.StdCall)]
		public delegate void FileAccessDelegate([MarshalAs(UnmanagedType.LPStr)]string path);

		public static void SetFileAccessCallBack(FileAccessDelegate callback)
		{
			CallbackMarshaller.SetFileAccessCallBack(Marshal.GetFunctionPointerForDelegate(callback));
		}
	}
}