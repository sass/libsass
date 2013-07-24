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

			AppDomain.CurrentDomain.AssemblyResolve += (sender, args) =>
            {
				if (args.Name.StartsWith(String.Format("{0}.proxy,", AssemblyName), StringComparison.OrdinalIgnoreCase))
                {
                    if (IsProxyPresent(assemblyDir))
                    {
                        throw new InvalidOperationException(String.Format("Found {0}.proxy.dll which cannot exist. Must instead have {0}.x86.dll and {0}.x64.dll. Check your build settings." + assemblyDir, AssemblyName));
                    }

                    string x86FullPath = Path.Combine(assemblyDir, String.Format("{0}.x86.dll", AssemblyName));
                    string x64FullPath = Path.Combine(assemblyDir, String.Format("{0}.x64.dll", AssemblyName));

                    string fileName = Environment.Is64BitProcess ? x64FullPath : x86FullPath;
                    if (!File.Exists(fileName))
                    {
                        throw new InvalidOperationException(string.Format("Could not find the wrapper assembly. It may not have been copied to the output directory. Path='{0}'",fileName));
                    }

					return Assembly.LoadFile(fileName);
				}

				return null;
			};
		}

        private static bool IsProxyPresent(string assemblyDir)
        {
            return File.Exists(Path.Combine(assemblyDir, AssemblyName + ".proxy.dll"));
        }
	}
}