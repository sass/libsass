using System.Text;
using System.Web.Optimization;

namespace NSass
{
	public class SassTransform : IBundleTransform
	{
		private readonly ISassCompiler _sassCompiler = new SassCompiler();

		public void Process(BundleContext context, BundleResponse response)
		{
			StringBuilder responseContent = new StringBuilder();
			string root = context.HttpContext.Server.MapPath("~");

			foreach (BundleFile bundleFile in response.Files)
			{
				string filename = context.HttpContext.Server.MapPath(bundleFile.IncludedVirtualPath);
				string output = _sassCompiler.CompileFile(filename, additionalIncludePaths: new[] { root });
				responseContent.Append(output);
			}

			response.Content = responseContent.ToString();
			response.ContentType = "text/css";
		}
	}
}