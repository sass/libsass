using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NSass.Tool.Models
{
	public class ProcessingState
	{
		public Project Project { get; private set; }

		public bool IsProcessingInProgress { get; set; }

		public StringBuilder Log { get; private set; }

		public ProcessingState(Project project)
		{
			Project = project;
			IsProcessingInProgress = false;
			Log = new StringBuilder();
		}
	}
}
