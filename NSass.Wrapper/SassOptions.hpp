#using <System.dll>

using namespace System;

namespace NSass
{
	public enum struct OutputStyle
	{
	  Nested,
	  Expanded,
	  Compact,
	  Compressed,
	  Echo
	};

	public ref class SassOptions
	{
		public:
			property OutputStyle OutputStyle;
			property bool SourceComments;
			property String^ IncludePaths;
			property String^ ImagePath;
	};

	public ref class SassContext
	{
		public:
			property String^ SourceString;
			property String^ OutputString;
			property SassOptions^ Options;
			property bool ErrorStatus;
			property String^ ErrorMessage;
	};

	public ref class SassFileContext
	{
		public:
			property String^ InputPath;
			property String^ OutputString;
			property SassOptions^ Options;
			property int ErrorStatus;
			property String^ ErrorMessage;
	};

	public ref class SassFolderContext
	{
		public:
			property String^ SearchPath;
			property String^ OutputPath;
			property SassOptions^ Options;
			property int ErrorStatus;
			property String^ ErrorMessage;
	};
}