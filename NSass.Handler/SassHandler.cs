using System;
using System.Web;

namespace NSass
{
	public class SassHandler : IHttpHandler
	{
		private readonly ISassCompiler _sassCompiler = new SassCompiler();

		public void ProcessRequest(HttpContext context)
		{
			string filename = context.Server.MapPath(String.Format("~{0}", context.Request.Path));
			string root = context.Server.MapPath("~");

			string output = _sassCompiler.CompileFile(filename, additionalIncludePaths: new[] { root });

			context.Response.Write(output);
			context.Response.ContentType = "text/css";
		}

		public bool IsReusable
		{
			get
			{
				return true;
			}
		}
	}
}