using System;
using System.IO;
using FluentAssertions;
using NUnit.Framework;

namespace NSass.Tests
{
	[TestFixture]
	public class SassCompilerStringTests : SassCompilerTestsBase
	{
		private readonly string _basicContent = File.ReadAllText(BasicPath);
		private readonly string _importingContent = File.ReadAllText(ImportingPath);
		private readonly string _importingAdditionalContent = File.ReadAllText(ImportingAdditionalPath);

		[Test]
		public void ShouldCompileString()
		{
			// Arrange
			const string expectedResult = "/* line 3, source string */\nbody .class {\n  color: red; }\n";

			// Act
			string compiled = SassCompiler.Compile(_basicContent);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileStringWithCompressedOutputStyle()
		{
			// Arrange
			const string expectedResult = "body .class {color:red;}";

			// Act
			string compiled = SassCompiler.Compile(_basicContent, OutputStyle.Compressed);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		// Uncomment and fix those test when libsass will support other output styles
		/*[Test]
		public void ShouldCompileStringWithCompactOutputStyle()
		{
			// Arrange
			const string expectedResult = "";

			// Act
			string compiled = _sassCompiler.Compile(_basicContent, OutputStyle.Compact);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileStringWithEchoOutputStyle()
		{
			// Arrange
			const string expectedResult = "body .class {\n  color: red; }\n";

			// Act
			string compiled = _sassCompiler.Compile(_basicContent, OutputStyle.Echo);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileStringWithExpandedOutputStyle()
		{
			// Arrange
			const string expectedResult = "body .class {\n  color: red; }\n";

			// Act
			string compiled = _sassCompiler.Compile(_basicContent, OutputStyle.Expanded);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}*/

		[Test]
		public void ShouldNotCompileStringWithCompactOutputStyle()
		{
			// Arrange

			// Act
			Action compile = () => SassCompiler.Compile(_basicContent, OutputStyle.Compact);

			// Assert
			compile.ShouldThrow<ArgumentException>();
		}

		[Test]
		public void ShouldNotCompileStringWithEchoOutputStyle()
		{
			// Arrange

			// Act
			Action compile = () => SassCompiler.Compile(_basicContent, OutputStyle.Echo);

			// Assert
			compile.ShouldThrow<ArgumentException>();
		}

		[Test]
		public void ShouldNotCompileStringWithExpandedOutputStyle()
		{
			// Arrange

			// Act
			Action compile = () => SassCompiler.Compile(_basicContent, OutputStyle.Expanded);

			// Assert
			compile.ShouldThrow<ArgumentException>();
		}

		[Test]
		public void ShouldCompileStringWithImport()
		{
			// Arrange
			string expectedResult = String.Format("/* line 1, {0} */\nbody {{\n  border-color: green; }}\n\n/* line 5, source string */\nbody .class {{\n  color: red; }}\n", ReplaceLastSlash(ImportPath));

			// Act
			string compiled = SassCompiler.Compile(_importingContent, includePaths: new[] { Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "TestFiles") });

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileStringWithAdditionalImport()
		{
			// Arrange
			string expectedResult = String.Format("/* line 1, {0} */\nbody {{\n  border-color: green; }}\n\n/* line 3, {1} */\nbody > #id {{\n  font-size: 14px; }}\n\n/* line 6, source string */\nbody .class {{\n  color: red; }}\n",
				ReplaceLastSlash(ImportPath),
				ReplaceLastSlash(AdditionalImportPath));

			// Act
			string compiled = SassCompiler.Compile(_importingAdditionalContent, includePaths: new[]
			{
				TestFilesDirectory,
				AdditionalImportTestFilesDirectory
			});

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileStringWithoutComments()
		{
			// Arrange
			const string expectedResult = "body .class {\n  color: red; }\n";

			// Act
			string compiled = SassCompiler.Compile(_basicContent, sourceComments: false);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}
	}
}