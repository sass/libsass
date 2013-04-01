namespace NSass
{
	public interface ISassCompiler
	{
		string Compile(string source, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true);
		string CompileFile(string inputPath, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true);
	}
}