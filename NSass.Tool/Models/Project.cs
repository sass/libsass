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
		public ProcessingState ProcessingState { get; private set; }

		public Project()
		{
			ProcessingState = new ProcessingState(this);
		}
	}
}