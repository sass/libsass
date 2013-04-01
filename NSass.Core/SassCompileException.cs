using System;

namespace NSass
{
	public class SassCompileException : Exception
	{
		public SassCompileException(string message) : base(message)
		{
		}		 
	}
}