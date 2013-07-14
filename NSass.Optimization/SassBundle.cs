using System.Web.Optimization;

namespace NSass
{
	public class SassBundle : Bundle
	{
		public SassBundle(string virtualPath) : base(virtualPath, new SassTransform(), new CssMinify()) { }

		public SassBundle(string virtualPath, string cdnPath) : base(virtualPath, cdnPath, new SassTransform(), new CssMinify()) { }
	}
}