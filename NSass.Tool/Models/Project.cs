using System;

namespace NSass.Tool.Models
{
	internal class Project
	{
		public Guid Id { get; set; }

		public string Name { get; set; }

		public string SourceFolderPath { get; set; }

		public string DestinationFolderPath { get; set; }

		public bool IncludeSubdirectories { get; set; }
	}
}