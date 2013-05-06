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

namespace NSass.Tool
{
	internal partial class MainForm : Form
	{
		public MainForm()
		{
			InitializeComponent();
			Program.Data.Projects.CollectionChanged += ProjectsChanged;
		}

		private void NewProject(object sender, EventArgs e)
		{
			ProjectPropertiesForm newProjectForm = new ProjectPropertiesForm();

			if (newProjectForm.ShowDialog(this) == DialogResult.OK)
			{
				Program.Data.Projects.Add(newProjectForm.Project);
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

		private void Exit(object sender, EventArgs e)
		{
			Application.Exit();
		}

		private void ProjectsChanged(object sender, NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action)
			{
				case NotifyCollectionChangedAction.Add:
					foreach (Project project in e.NewItems.Cast<Project>())
					{
						projectsListView.Items.Add(project.Id.ToString(), project.Name, 0);
					}
					break;
				case NotifyCollectionChangedAction.Remove:
					foreach (Project project in e.NewItems.Cast<Project>())
					{
						projectsListView.Items.RemoveByKey(project.Id.ToString());
					}
					break;
			}
		}

		private ListViewItem GetSelectedItem()
		{
			return projectsListView.SelectedItems.Cast<ListViewItem>().Single();
		}

		private Project GetSelectedProject(ListViewItem selectedItem = null)
		{
			selectedItem = selectedItem ?? GetSelectedItem();
			string key = selectedItem.Name;
			return Program.Data.Projects.Single(x => x.Id.ToString() == key);
		}
	}
}