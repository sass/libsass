using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using NSass.Tool.Models;

namespace NSass.Tool
{
	internal partial class ProjectPropertiesForm : Form
	{
		public ProjectPropertiesForm()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			if (Project != null)
			{
				nameTextBox.Text = Project.Name;
				sourceFolderTextBox.Text = Project.SourceFolderPath;
				destinationFolderTextBox.Text = Project.DestinationFolderPath;
				includeSubdirectoriesCheckBox.Checked = Project.IncludeSubdirectories;
			}
		}

		public Project Project { get; set; }

		private void Submit(object sender, EventArgs e)
		{
			Project = Project ?? new Project { Id = new Guid() };

			Project.Name = nameTextBox.Text;
			Project.SourceFolderPath = sourceFolderTextBox.Text;
			Project.DestinationFolderPath = destinationFolderTextBox.Text;
			Project.IncludeSubdirectories = includeSubdirectoriesCheckBox.Checked;
		}

		private void ChooseSourceFolder(object sender, EventArgs e)
		{
			mainFolderBrowserDialog.ShowNewFolderButton = false;

			if (mainFolderBrowserDialog.ShowDialog(this) == DialogResult.OK)
			{
				sourceFolderTextBox.Text = mainFolderBrowserDialog.SelectedPath;

				if (String.IsNullOrEmpty(destinationFolderTextBox.Text))
				{
					destinationFolderTextBox.Text = mainFolderBrowserDialog.SelectedPath;
				}
			}
		}

		private void ChooseDestinationFolder(object sender, EventArgs e)
		{
			mainFolderBrowserDialog.ShowNewFolderButton = true;

			if (mainFolderBrowserDialog.ShowDialog(this) == DialogResult.OK)
			{
				destinationFolderTextBox.Text = mainFolderBrowserDialog.SelectedPath;
			}
		}
	}
}