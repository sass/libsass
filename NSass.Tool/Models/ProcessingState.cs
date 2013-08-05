using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;

namespace NSass.Tool.Models
{
	public class ProcessingState
	{
		public Project Project { get; private set; }

		public bool IsProcessingInProgress { get; set; }

		public StringBuilder Log { get; private set; }

		public FileSystemWatcher FileSystemWatcher { get; private set; }

		public List<FileModel> Files { get; private set; }

		public ProcessingState(Project project)
		{
			Project = project;
			IsProcessingInProgress = false;
			Log = new StringBuilder();
			FileSystemWatcher = new FileSystemWatcher
			{
				Filter = "*.scss"
			};
			UpdateWatcher();
			Files = new List<FileModel>();
		}

		public void UpdateWatcher()
		{
			FileSystemWatcher.Path = Project.SourceFolderPath;
			FileSystemWatcher.IncludeSubdirectories = Project.IncludeSubdirectories;
		}

		public FileModel GetOrCreateFile(string fullPath)
		{
			if (!fullPath.StartsWith(Project.SourceFolderPath))
			{
				throw new ArgumentException("Full path should be subdirectory of a source one.", "fullPath");
			}

			string relativePath = fullPath.Substring(Project.SourceFolderPath.Length);

			FileModel file = Files.SingleOrDefault(x => x.Path == relativePath);
			if (file == null)
			{
				file = new FileModel(Project)
				{
					Path = fullPath.Substring(Project.SourceFolderPath.Length),
					FullPath = fullPath
				};
				Files.Add(file);
			}
			return file;
		}

		public IEnumerable<string> GetDependantFiles(string fullPath)
		{
			return Files.Where(x => x.Dependencies.Contains(fullPath.Substring(Project.SourceFolderPath.Length))).Select(x => x.FullPath);
		}
	}
}