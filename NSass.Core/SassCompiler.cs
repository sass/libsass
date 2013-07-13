using System;
using System.Collections.Generic;
using System.IO;

namespace NSass
{
	public class SassCompiler : ISassCompiler
	{
		private readonly ISassInterface _sassInterface;

		static SassCompiler()
		{
			AssemblyResolver.Initialize();
		}

		public SassCompiler()
		{
			_sassInterface = new SassInterface();
		}

		public SassCompiler(ISassInterface sassInterface)
		{
			_sassInterface = sassInterface;
		}

		public string Compile(string source, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true, IEnumerable<string> includePaths = null)
		{
			if (outputStyle != OutputStyle.Nested && outputStyle != OutputStyle.Compressed)
			{
				throw new ArgumentException("Only nested and compressed output styles are currently supported by libsass.");
			}

			SassContext context = new SassContext
			{
				SourceString = source,
				Options = new SassOptions
				{
					OutputStyle = (int)outputStyle,
					SourceComments = sourceComments,
					IncludePaths = includePaths != null ? String.Join(";", includePaths) : String.Empty,
					ImagePath = String.Empty
				}
			};

			_sassInterface.Compile(context);

			if (context.ErrorStatus)
			{
				throw new SassCompileException(context.ErrorMessage);
			}

			return context.OutputString;
		}

		public string CompileFile(string inputPath, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true, IEnumerable<string> additionalIncludePaths = null)
		{
			if (outputStyle != OutputStyle.Nested && outputStyle != OutputStyle.Compressed)
			{
				throw new ArgumentException("Only nested and compressed output styles are currently supported by libsass.");
			}

			string directoryName = Path.GetDirectoryName(inputPath);
			List<string> includePaths = new List<string> { directoryName };
			if (additionalIncludePaths != null)
			{
				includePaths.AddRange(additionalIncludePaths);
			}

			SassFileContext context = new SassFileContext
			{
				InputPath = inputPath,
				Options = new SassOptions
				{
					OutputStyle = (int)outputStyle,
					SourceComments = sourceComments,
					IncludePaths = String.Join(";", includePaths),
					ImagePath = String.Empty
				}
			};

			_sassInterface.Compile(context);

			if (context.ErrorStatus)
			{
				throw new SassCompileException(context.ErrorMessage);
			}

			return context.OutputString;
		}
	}
}