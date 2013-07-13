using System;
using System.IO;

namespace NSass.Tests
{
	public class SassCompilerTestsBase
	{
		protected readonly ISassCompiler SassCompiler = new SassCompiler();

		protected static readonly string TestFilesDirectory = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "TestFiles");
		protected static readonly string AdditionalImportTestFilesDirectory = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "TestFiles", "AdditionalImport");

		protected static readonly string BasicPath = Path.Combine(TestFilesDirectory, "Basic.scss");
		protected static readonly string ImportPath = Path.Combine(TestFilesDirectory, "Import.scss");
		protected static readonly string ImportingPath = Path.Combine(TestFilesDirectory, "Importing.scss");
		protected static readonly string ImportingAdditionalPath = Path.Combine(TestFilesDirectory, "ImportingAdditional.scss");
		protected static readonly string AdditionalImportPath = Path.Combine(AdditionalImportTestFilesDirectory, "AdditionalImport.scss");

		protected static string ReplaceLastSlash(string path)
		{
			int index = path.LastIndexOf('\\');
			return path.Substring(0, index) + '/' + path.Substring(index + 1);
		}
	}
}