using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using NSass.Tool.Models;
using NSass.Tool.Properties;

namespace NSass.Tool
{
	internal partial class MainForm : Form
	{
		private readonly List<ToolStripItem> _enabledForSelectedProjectOnlyControls = new List<ToolStripItem>();
		private readonly Data _data = new Data();
		private readonly DataStorage _dataStorage = new DataStorage();
		private readonly ISassCompiler _compiler = new SassCompiler();
		private readonly Object _syncRoot = new Object();

		public MainForm()
		{
			InitializeComponent();

			_data = new Data();
			_data.Projects.CollectionChanged += ProjectsChanged;

			_enabledForSelectedProjectOnlyControls.AddRange(new ToolStripItem[]
			{
				editToolStripButton, editToolStripMenuItem,
				deleteToolStripButton, deleteToolStripMenuItem,
				startStopToolStripButton, startStopToolStripMenuItem
			});
			SelectedProjectChanged(null, null);

			foreach (Project project in _dataStorage.GetAll())
			{
				_data.Projects.Add(project);
			}
		}

		#region Actions

		private void NewProject(object sender, EventArgs e)
		{
			ProjectPropertiesForm newProjectForm = new ProjectPropertiesForm();

			if (newProjectForm.ShowDialog(this) == DialogResult.OK)
			{
				_data.Projects.Add(newProjectForm.Project);
			}
		}

		private void EditProject(object sender, EventArgs e)
		{
			ListViewItem selectedItem = GetSelectedItem();

			ProjectPropertiesForm editProjectForm = new ProjectPropertiesForm
			{
				Project = GetSelectedProject(selectedItem)
			};

			if (editProjectForm.ShowDialog() == DialogResult.OK)
			{
				selectedItem.Text = editProjectForm.Project.Name;
				_dataStorage.Update(editProjectForm.Project);
			}
		}

		private void DeleteProject(object sender, EventArgs e)
		{
			if (MessageBox.Show(this, "Are you sure want to delete selected project?", "Confirm", MessageBoxButtons.YesNo,
				MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button2) == DialogResult.Yes)
			{
				ListViewItem selectedItem = GetSelectedItem();
				projectsListView.Items.Remove(selectedItem);
				_data.Projects.Remove(GetSelectedProject(selectedItem));
			}
		}

		private void StartStopPreprocessing(object sender, EventArgs e)
		{
			Project project = GetSelectedProject();
			project.ProcessingState.IsProcessingInProgress = !project.ProcessingState.IsProcessingInProgress;

			string logMessage = project.ProcessingState.IsProcessingInProgress ? "Preprocessing started" : "Preprocessing stopped";
			Log(project, logMessage);

			UpdateStartStopButtons(project);

			if (project.ProcessingState.IsProcessingInProgress)
			{
				SearchOption searchOption = project.IncludeSubdirectories ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly;
				IEnumerable<string> filePaths = Directory.EnumerateFiles(project.SourceFolderPath, "*.scss", searchOption);
				foreach (string filePath in filePaths)
				{
					FileModel file = project.ProcessingState.GetOrCreateFile(filePath);

					if (file.VerifyChecksum())
					{
						CompileFile(file, project);
					}
				}

				project.ProcessingState.UpdateWatcher();
				FileSystemEventHandler handler = (o, args) =>
				{
					lock (_syncRoot)
					{

						try
						{
							FileModel file = project.ProcessingState.GetOrCreateFile(args.FullPath);

							if (file.VerifyChecksum())
							{
								CompileFile(file, project);
								IEnumerable<string> dependantFilePaths = project.ProcessingState.GetDependantFiles(file.FullPath);
								foreach (string dependantFilePath in dependantFilePaths)
								{
									FileModel dependantFile = project.ProcessingState.GetOrCreateFile(dependantFilePath);
									CompileFile(dependantFile, project);
								}
							}
						}
						catch (Exception ex)
						{
							Log(project, "Unhandled exception: " + ex.Message);
						}

					}
				};
				project.ProcessingState.FileSystemWatcher.Changed += handler;
				/*selectedProject.ProcessingState.FileSystemWatcher.Renamed += (o, args) => handler(o, args);
				selectedProject.ProcessingState.FileSystemWatcher.Deleted += handler;*/
				project.ProcessingState.FileSystemWatcher.Created += handler;
				project.ProcessingState.FileSystemWatcher.EnableRaisingEvents = true;

			}
			else
			{
				project.ProcessingState.FileSystemWatcher.EnableRaisingEvents = false;
			}
		}

		private void CompileFile(FileModel file, Project project)
		{
			file.Dependencies.Clear();
			ManagedCallbackManager.SetFileAccessCallBack(file.AddDependency);

			try
			{
				Stopwatch stopwatch = new Stopwatch();
				stopwatch.Start();

				string output = _compiler.CompileFile(file.FullPath);

				// Save file into physical one if it is not mixin file
				if (!Path.GetFileName(file.FullPath).StartsWith("_"))
				{
					// TODO: Save file here
				}

				stopwatch.Stop();

				Log(project, String.Format("Successfully compiled file at {0}, compilation took {1} ms", file.Path, stopwatch.ElapsedMilliseconds));
			}
			catch (SassCompileException ex)
			{
				Log(project, String.Format("Error while preprocessing file at {0}:{1}{2}", file.Path, Environment.NewLine, ex.Message));
			}

			ManagedCallbackManager.UnsetFileAccessCallBack();
		}

		private void Exit(object sender, EventArgs e)
		{
			Application.Exit();
		}

		#endregion

		#region Event handlers

		private void ProjectsChanged(object sender, NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action)
			{
				case NotifyCollectionChangedAction.Add:
					foreach (Project project in e.NewItems.Cast<Project>())
					{
						projectsListView.Items.Add(project.Id.ToString(), project.Name, 0);
						_dataStorage.Add(project);
					}
					break;
				case NotifyCollectionChangedAction.Remove:
					foreach (Project project in e.OldItems.Cast<Project>())
					{
						projectsListView.Items.RemoveByKey(project.Id.ToString());
						_dataStorage.Remove(project);
					}
					break;
			}
		}

		private void SelectedProjectChanged(object sender, EventArgs e)
		{
			ListViewItem selectedItem = GetSelectedItem();
			bool areEnabled = selectedItem != null;
			_enabledForSelectedProjectOnlyControls.ForEach(x => x.Enabled = areEnabled);

			Project selectedProject = GetSelectedProject(selectedItem);
			if (selectedProject != null)
			{
				UpdateStartStopButtons(selectedProject);
			}
			UpdateLog();
		}

		#endregion

		#region Utility functions

		private delegate ListViewItem GetSelectedItemCallback();

		private ListViewItem GetSelectedItem()
		{
			if (projectsListView.InvokeRequired)
			{
				return (ListViewItem)projectsListView.Invoke(new GetSelectedItemCallback(GetSelectedItem));
			}
			else
			{
				return projectsListView.SelectedItems.Cast<ListViewItem>().SingleOrDefault();
			}
		}

		private Project GetSelectedProject(ListViewItem selectedItem = null)
		{
			selectedItem = selectedItem ?? GetSelectedItem();
			if (selectedItem == null)
			{
				return null;
			}

			string key = selectedItem.Name;
			return _data.Projects.Single(x => x.Id.ToString() == key);
		}

		private void UpdateStartStopButtons(Project project)
		{
			Bitmap image;
			string menuItemText, toolbarButtonTooltip;

			if (project.ProcessingState.IsProcessingInProgress)
			{
				image = Resources.Stop;
				menuItemText = "Stop";
				toolbarButtonTooltip = "Stop preprocessing";
			}
			else
			{
				image = Resources.Start;
				menuItemText = "Start";
				toolbarButtonTooltip = "Start preprocessing";
			}

			startStopToolStripButton.Image = startStopToolStripMenuItem.Image = image;
			startStopToolStripMenuItem.Text = menuItemText;
			startStopToolStripButton.ToolTipText = toolbarButtonTooltip;
		}

		private delegate void SetTextCallback(string text);

		private void SetLogText(string text)
		{
			if (logTextBox.InvokeRequired)
			{
				logTextBox.Invoke(new SetTextCallback(SetLogText), new object[] { text });
			}
			else
			{
				logTextBox.Text = text;
			}
		}

		private void UpdateLog()
		{
			Project selectedProject = GetSelectedProject();
			SetLogText(selectedProject != null ? selectedProject.ProcessingState.Log.ToString() : String.Empty);
		}

		private void Log(Project project, string message)
		{
			project.ProcessingState.Log.AppendFormat("[{0}] {1}{2}", DateTime.Now.ToLongTimeString(), message, Environment.NewLine);
			UpdateLog();
		}

		#endregion
	}
}