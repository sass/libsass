using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text;

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

		public string Compile(string source, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true)
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
					SourceComments = sourceComments
				}
			};

			_sassInterface.Compile(context);

			if (context.ErrorStatus)
			{
				throw new SassCompileException(context.ErrorMessage);
			}

			return context.OutputString;
		}

		public string CompileFile(string inputPath, OutputStyle outputStyle = OutputStyle.Nested, bool sourceComments = true)
		{
			if (outputStyle != OutputStyle.Nested && outputStyle != OutputStyle.Compressed)
			{
				throw new ArgumentException("Only nested and compressed output styles are currently supported by libsass.");
			}

			SassFileContext context = new SassFileContext
			{
				InputPath = inputPath,
				Options = new SassOptions
				{
					OutputStyle = (int)outputStyle,
					SourceComments = sourceComments
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