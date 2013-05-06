using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace NSass.Tool.Models
{
	internal class Data
	{
		public Data()
		{
			Projects = new ObservableCollection<Project>();
		}

		public ObservableCollection<Project> Projects { get; private set; }
	}
}