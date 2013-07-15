using System;
using System.Xml.Serialization;

namespace NSass.Tool.Models
{
	public class Project
	{
		public Guid Id { get; set; }

		public string Name { get; set; }

		public string SourceFolderPath { get; set; }

		public string DestinationFolderPath { get; set; }

		public bool IncludeSubdirectories { get; set; }

		[XmlIgnore]
		public bool IsProcessingInProgress { get; set; }
	}
}