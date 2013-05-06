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

		public Project Project { get; private set; }

		private void Submit(object sender, EventArgs e)
		{
			Project = new Project
			{
				Id = Guid.NewGuid(),
				Name = nameTextBox.Text,
				SourceFolderPath = sourceFolderTextBox.Text,
				DestinationFolderPath = destinationFolderTextBox.Text,
				IncludeSubdirectories = includeSubdirectoriesCheckBox.Checked
			};
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