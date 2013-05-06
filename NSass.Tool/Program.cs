using System;
using System.Windows.Forms;
using NSass.Tool.Models;

namespace NSass.Tool
{
	internal static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		private static void Main()
		{
			Data = new Data();

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new MainForm());
		}

		public static Data Data { get; private set; }
	}
}