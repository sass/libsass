using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
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
			Project selectedProject = GetSelectedProject();
			selectedProject.IsProcessingInProgress = !selectedProject.IsProcessingInProgress;
			UpdateStartStopButtons(selectedProject);
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

			if (areEnabled)
			{
				Project selectedProject = GetSelectedProject(selectedItem);
				UpdateStartStopButtons(selectedProject); 
			}
		}

		#endregion

		#region Utility functions

		private ListViewItem GetSelectedItem()
		{
			return projectsListView.SelectedItems.Cast<ListViewItem>().SingleOrDefault();
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

			if (project.IsProcessingInProgress)
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

		#endregion
	}
}