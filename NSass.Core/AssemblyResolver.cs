using System;
using System.IO;
using System.Reflection;

namespace NSass
{
	public static class AssemblyResolver
	{
		private const string AssemblyName = "NSass.Wrapper";
		private const string AssembliesDir = "NSass.Wrapper";

		public static void Initialize()
		{
			string assemblyDir = AppDomain.CurrentDomain.SetupInformation.PrivateBinPath;
			// PrivateBinPath is empty in test scenarios; use BaseDirectory instead
			if (String.IsNullOrEmpty(assemblyDir))
			{
				assemblyDir = AppDomain.CurrentDomain.BaseDirectory;
			}
			assemblyDir = Path.Combine(assemblyDir, AssembliesDir);
			string proxyFullPath = Path.Combine(assemblyDir, String.Format("{0}.proxy.dll", AssemblyName));
			string x86FullPath = Path.Combine(assemblyDir, String.Format("{0}.x86.dll", AssemblyName));
			string x64FullPath = Path.Combine(assemblyDir, String.Format("{0}.x64.dll", AssemblyName));
			if (File.Exists(proxyFullPath) || !File.Exists(x86FullPath) || !File.Exists(x64FullPath))
			{
				throw new InvalidOperationException(String.Format("Found {0}.proxy.dll which cannot exist. Must instead have {0}.x86.dll and {0}.x64.dll. Check your build settings." + assemblyDir, AssemblyName));
			}

			AppDomain.CurrentDomain.AssemblyResolve += (sender, args) =>
			{
				if (args.Name.StartsWith(String.Format("{0}.proxy,", AssemblyName), StringComparison.OrdinalIgnoreCase))
				{
					string fileName = Path.Combine(assemblyDir, String.Format("{0}.{1}.dll", AssemblyName, Environment.Is64BitProcess ? "x64" : "x86"));
					return Assembly.LoadFile(fileName);
				}
				return null;
			};
		}
	}
}