#include "SassOptions.hpp"

namespace NSass
{
	public interface class ISassInterface
	{
		int Compile(SassContext^ sassContext);
		int Compile(SassFileContext^ sassFileContext);
		// Folder context isn't implemented in core libsass library now
		/*int Compile(SassFolderContext^ sassFolderContext);*/
	};
}