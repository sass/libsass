using System.Collections.Generic;

namespace NSass
{
	public interface ISassCompiler
	{
		string Compile(string source, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true, IEnumerable<string> includePaths = null);
		string CompileFile(string inputPath, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true, IEnumerable<string> additionalIncludePaths = null);
	}
}