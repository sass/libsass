using System.Collections.Generic;

namespace NSass.Tool.Models
{
	public class FileModel
	{
		public string Path { get; set; }

		public string FullPath { get; set; }

		public List<string> Dependencies { get; private set; }

		public Project Project { get; private set; }

		public uint LastChecksum { get; set; }

		public FileModel(Project project)
		{
			Dependencies = new List<string>();
			Project = project;
			LastChecksum = 0;
		}

		public void AddDependency(string fullPath)
		{
			fullPath = System.IO.Path.GetFullPath(fullPath);

			if (fullPath.StartsWith(Project.SourceFolderPath))
			{
				string relativePath = fullPath.Substring(Project.SourceFolderPath.Length);
				if (relativePath != Path)
				{
					Dependencies.Add(relativePath);
				}
			}
		}

		public bool VerifyChecksum()
		{
			uint previousChecksum = LastChecksum;
			LastChecksum = FileChecksum.Calculate(FullPath);
			return LastChecksum != previousChecksum;
		}
	}
}