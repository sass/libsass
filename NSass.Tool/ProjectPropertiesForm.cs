using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
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
			if (!ValidateChildren())
			{
				DialogResult = DialogResult.None;
				return;
			}

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

		private void ValidateNotEmpty(object sender, CancelEventArgs e)
		{
			TextBox textBox = (TextBox)sender;


			if (String.IsNullOrEmpty(textBox.Text))
			{
				string textBoxName = textBox.Name.Substring(0, textBox.Name.IndexOf("TextBox"));
				Regex regex = new Regex("(?!^)([A-Z])");
				textBoxName = regex.Replace(textBoxName, x =>  " " + x.Value.ToLower());
				textBoxName = Char.ToUpper(textBoxName[0]) + textBoxName.Substring(1);

				mainErrorProvider.SetError(textBox, String.Format("{0} should be specified", textBoxName));
				e.Cancel = true;
			}
			else
			{
				mainErrorProvider.SetError(textBox, null);
				e.Cancel = false;
			}
		}
	}
}