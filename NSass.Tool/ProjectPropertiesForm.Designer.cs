namespace NSass.Tool
{
    internal partial class ProjectPropertiesForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.Windows.Forms.Label sourceFolderLabel;
            System.Windows.Forms.Label destinationFolderLabelabel;
            System.Windows.Forms.Label nameLabel;
            System.Windows.Forms.Button okButton;
            System.Windows.Forms.Button cancelButton;
            this.chooseSourceFolderButton = new System.Windows.Forms.Button();
            this.sourceFolderTextBox = new System.Windows.Forms.TextBox();
            this.destinationFolderTextBox = new System.Windows.Forms.TextBox();
            this.chooseDestinationFolderButton = new System.Windows.Forms.Button();
            this.nameTextBox = new System.Windows.Forms.TextBox();
            this.includeSubdirectoriesCheckBox = new System.Windows.Forms.CheckBox();
            this.mainFolderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            sourceFolderLabel = new System.Windows.Forms.Label();
            destinationFolderLabelabel = new System.Windows.Forms.Label();
            nameLabel = new System.Windows.Forms.Label();
            okButton = new System.Windows.Forms.Button();
            cancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // sourceFolderLabel
            // 
            sourceFolderLabel.AutoSize = true;
            sourceFolderLabel.Location = new System.Drawing.Point(12, 44);
            sourceFolderLabel.Name = "sourceFolderLabel";
            sourceFolderLabel.Size = new System.Drawing.Size(73, 13);
            sourceFolderLabel.TabIndex = 0;
            sourceFolderLabel.Text = "Source folder:";
            // 
            // destinationFolderLabelabel
            // 
            destinationFolderLabelabel.AutoSize = true;
            destinationFolderLabelabel.Location = new System.Drawing.Point(12, 75);
            destinationFolderLabelabel.Name = "destinationFolderLabelabel";
            destinationFolderLabelabel.Size = new System.Drawing.Size(92, 13);
            destinationFolderLabelabel.TabIndex = 0;
            destinationFolderLabelabel.Text = "Destination folder:";
            // 
            // nameLabel
            // 
            nameLabel.AutoSize = true;
            nameLabel.Location = new System.Drawing.Point(12, 15);
            nameLabel.Name = "nameLabel";
            nameLabel.Size = new System.Drawing.Size(38, 13);
            nameLabel.TabIndex = 0;
            nameLabel.Text = "Name:";
            // 
            // chooseSourceFolderButton
            // 
            this.chooseSourceFolderButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.chooseSourceFolderButton.Location = new System.Drawing.Point(347, 38);
            this.chooseSourceFolderButton.Name = "chooseSourceFolderButton";
            this.chooseSourceFolderButton.Size = new System.Drawing.Size(25, 25);
            this.chooseSourceFolderButton.TabIndex = 5;
            this.chooseSourceFolderButton.Text = "...";
            this.chooseSourceFolderButton.UseVisualStyleBackColor = true;
            this.chooseSourceFolderButton.Click += new System.EventHandler(this.ChooseSourceFolder);
            // 
            // sourceFolderTextBox
            // 
            this.sourceFolderTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.sourceFolderTextBox.Location = new System.Drawing.Point(110, 41);
            this.sourceFolderTextBox.Name = "sourceFolderTextBox";
            this.sourceFolderTextBox.Size = new System.Drawing.Size(231, 20);
            this.sourceFolderTextBox.TabIndex = 4;
            // 
            // destinationFolderTextBox
            // 
            this.destinationFolderTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.destinationFolderTextBox.Location = new System.Drawing.Point(110, 72);
            this.destinationFolderTextBox.Name = "destinationFolderTextBox";
            this.destinationFolderTextBox.Size = new System.Drawing.Size(231, 20);
            this.destinationFolderTextBox.TabIndex = 6;
            // 
            // chooseDestinationFolderButton
            // 
            this.chooseDestinationFolderButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.chooseDestinationFolderButton.Location = new System.Drawing.Point(347, 69);
            this.chooseDestinationFolderButton.Name = "chooseDestinationFolderButton";
            this.chooseDestinationFolderButton.Size = new System.Drawing.Size(25, 25);
            this.chooseDestinationFolderButton.TabIndex = 7;
            this.chooseDestinationFolderButton.Text = "...";
            this.chooseDestinationFolderButton.UseVisualStyleBackColor = true;
            this.chooseDestinationFolderButton.Click += new System.EventHandler(this.ChooseDestinationFolder);
            // 
            // nameTextBox
            // 
            this.nameTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.nameTextBox.Location = new System.Drawing.Point(110, 12);
            this.nameTextBox.Name = "nameTextBox";
            this.nameTextBox.Size = new System.Drawing.Size(262, 20);
            this.nameTextBox.TabIndex = 3;
            // 
            // includeSubdirectoriesCheckBox
            // 
            this.includeSubdirectoriesCheckBox.AutoSize = true;
            this.includeSubdirectoriesCheckBox.Checked = true;
            this.includeSubdirectoriesCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.includeSubdirectoriesCheckBox.Location = new System.Drawing.Point(15, 98);
            this.includeSubdirectoriesCheckBox.Name = "includeSubdirectoriesCheckBox";
            this.includeSubdirectoriesCheckBox.Size = new System.Drawing.Size(129, 17);
            this.includeSubdirectoriesCheckBox.TabIndex = 8;
            this.includeSubdirectoriesCheckBox.Text = "Include subdirectories";
            this.includeSubdirectoriesCheckBox.UseVisualStyleBackColor = true;
            // 
            // okButton
            // 
            okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            okButton.Location = new System.Drawing.Point(216, 122);
            okButton.Name = "okButton";
            okButton.Size = new System.Drawing.Size(75, 23);
            okButton.TabIndex = 1;
            okButton.Text = "OK";
            okButton.UseVisualStyleBackColor = true;
            okButton.Click += new System.EventHandler(this.Submit);
            // 
            // cancelButton
            // 
            cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            cancelButton.Location = new System.Drawing.Point(297, 122);
            cancelButton.Name = "cancelButton";
            cancelButton.Size = new System.Drawing.Size(75, 23);
            cancelButton.TabIndex = 2;
            cancelButton.Text = "Cancel";
            cancelButton.UseVisualStyleBackColor = true;
            // 
            // ProjectPropertiesForm
            // 
            this.AcceptButton = okButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = cancelButton;
            this.ClientSize = new System.Drawing.Size(384, 157);
            this.Controls.Add(cancelButton);
            this.Controls.Add(okButton);
            this.Controls.Add(this.includeSubdirectoriesCheckBox);
            this.Controls.Add(this.destinationFolderTextBox);
            this.Controls.Add(this.nameTextBox);
            this.Controls.Add(this.sourceFolderTextBox);
            this.Controls.Add(this.chooseDestinationFolderButton);
            this.Controls.Add(this.chooseSourceFolderButton);
            this.Controls.Add(destinationFolderLabelabel);
            this.Controls.Add(nameLabel);
            this.Controls.Add(sourceFolderLabel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ProjectPropertiesForm";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "New project";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button chooseSourceFolderButton;
        private System.Windows.Forms.TextBox sourceFolderTextBox;
        private System.Windows.Forms.TextBox destinationFolderTextBox;
        private System.Windows.Forms.Button chooseDestinationFolderButton;
        private System.Windows.Forms.TextBox nameTextBox;
        private System.Windows.Forms.CheckBox includeSubdirectoriesCheckBox;
        private System.Windows.Forms.FolderBrowserDialog mainFolderBrowserDialog;
    }
}