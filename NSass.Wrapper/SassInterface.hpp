#include "ISassInterface.hpp"

namespace NSass
{
	public ref class SassInterface : ISassInterface
	{
		public:
			virtual int Compile(SassContext^ sassContext);
			virtual int Compile(SassFileContext^ sassFileContext);
			// Folder context isn't implemented in core libsass library now
			/*virtual int Compile(SassFolderContext^ sassFolderContext);*/
	};
}