using System;
using System.IO;
using FluentAssertions;
using NUnit.Framework;

namespace NSass.Tests
{
	[TestFixture]
	public class SassCompilerFileTests : SassCompilerTestsBase
	{
		[Test]
		public void ShouldCompileFile()
		{
			// Arrange
			string expectedResult = String.Format("/* line 3, {0} */\nbody .class {{\n  color: red; }}\n", BasicPath);

			// Act
			string compiled = SassCompiler.CompileFile(BasicPath);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileFileWithCompressedOutputStyle()
		{
			// Arrange
			const string expectedResult = "body .class {color:red;}";

			// Act
			string compiled = SassCompiler.CompileFile(BasicPath, OutputStyle.Compressed);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		// Uncomment and fix those test when libsass will support other output styles
		/*[Test]
		public void ShouldCompileFileWithCompactOutputStyle()
		{
			// Arrange
			const string expectedResult = "";

			// Act
			string compiled = _sassCompiler.CompileFile(BasicPath, OutputStyle.Compact);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileFileWithEchoOutputStyle()
		{
			// Arrange
			const string expectedResult = "body .class {\n  color: red; }\n";

			// Act
			string compiled = _sassCompiler.CompileFile(BasicPath, OutputStyle.Echo);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileFileWithExpandedOutputStyle()
		{
			// Arrange
			const string expectedResult = "body .class {\n  color: red; }\n";

			// Act
			string compiled = _sassCompiler.CompileFile(BasicPath, OutputStyle.Expanded);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}*/

		[Test]
		public void ShouldNotCompileFileWithCompactOutputStyle()
		{
			// Arrange

			// Act
			Action compile = () => SassCompiler.CompileFile(BasicPath, OutputStyle.Compact);

			// Assert
			compile.ShouldThrow<ArgumentException>();
		}

		[Test]
		public void ShouldNotCompileFileWithEchoOutputStyle()
		{
			// Arrange

			// Act
			Action compile = () => SassCompiler.CompileFile(BasicPath, OutputStyle.Echo);

			// Assert
			compile.ShouldThrow<ArgumentException>();
		}

		[Test]
		public void ShouldNotCompileFileWithExpandedOutputStyle()
		{
			// Arrange

			// Act
			Action compile = () => SassCompiler.CompileFile(BasicPath, OutputStyle.Expanded);

			// Assert
			compile.ShouldThrow<ArgumentException>();
		}

		[Test]
		public void ShouldCompileFileWithImport()
		{
			// Arrange
			string expectedResult = String.Format("/* line 1, {0} */\nbody {{\n  border-color: green; }}\n\n/* line 5, {1} */\nbody .class {{\n  color: red; }}\n", ReplaceLastSlash(ImportPath), ImportingPath);

			// Act
			string compiled = SassCompiler.CompileFile(ImportingPath);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileFileWithAdditionalImport()
		{
			// Arrange
			string expectedResult = String.Format("/* line 1, {0} */\nbody {{\n  border-color: green; }}\n\n/* line 3, {1} */\nbody > #id {{\n  font-size: 14px; }}\n\n/* line 6, {2} */\nbody .class {{\n  color: red; }}\n",
				ReplaceLastSlash(ImportPath),
				ReplaceLastSlash(AdditionalImportPath),
				ImportingAdditionalPath);

			// Act
			string compiled = SassCompiler.CompileFile(ImportingAdditionalPath, additionalIncludePaths: new[]
			{
				AdditionalImportTestFilesDirectory
			});

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}

		[Test]
		public void ShouldCompileFileWithoutComments()
		{
			// Arrange
			const string expectedResult = "body .class {\n  color: red; }\n";

			// Act
			string compiled = SassCompiler.CompileFile(BasicPath, sourceComments: false);

			// Assert
			compiled.Should().BeEquivalentTo(expectedResult);
		}
	}
}