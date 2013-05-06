﻿namespace NSass.Tool
{
    internal partial class MainForm
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
            System.Windows.Forms.MenuStrip mainMenuStrip;
            System.Windows.Forms.ToolStripMenuItem projectToolStripMenuItem;
            System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
            System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
            System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
            System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
            System.Windows.Forms.ToolStripMenuItem startStopToolStripMenuItem;
            System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
            System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
            System.Windows.Forms.ToolStrip mainToolStrip;
            System.Windows.Forms.ToolStripButton newToolStripButton;
            System.Windows.Forms.ToolStripButton editToolStripButton;
            System.Windows.Forms.ToolStripButton deleteToolStripButton;
            System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
            System.Windows.Forms.ToolStripButton startStopToolStripButton;
            System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
            System.Windows.Forms.ToolStripButton exitToolStripButton;
            this.projectsListView = new System.Windows.Forms.ListView();
            this.logTextBox = new System.Windows.Forms.TextBox();
            mainMenuStrip = new System.Windows.Forms.MenuStrip();
            projectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            startStopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            mainToolStrip = new System.Windows.Forms.ToolStrip();
            newToolStripButton = new System.Windows.Forms.ToolStripButton();
            editToolStripButton = new System.Windows.Forms.ToolStripButton();
            deleteToolStripButton = new System.Windows.Forms.ToolStripButton();
            toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            startStopToolStripButton = new System.Windows.Forms.ToolStripButton();
            toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            exitToolStripButton = new System.Windows.Forms.ToolStripButton();
            mainMenuStrip.SuspendLayout();
            mainToolStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenuStrip
            // 
            mainMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            projectToolStripMenuItem});
            mainMenuStrip.Location = new System.Drawing.Point(0, 0);
            mainMenuStrip.Name = "mainMenuStrip";
            mainMenuStrip.Size = new System.Drawing.Size(784, 24);
            mainMenuStrip.TabIndex = 0;
            // 
            // projectToolStripMenuItem
            // 
            projectToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            newToolStripMenuItem,
            editToolStripMenuItem,
            deleteToolStripMenuItem,
            toolStripSeparator1,
            startStopToolStripMenuItem,
            toolStripSeparator2,
            exitToolStripMenuItem});
            projectToolStripMenuItem.Name = "projectToolStripMenuItem";
            projectToolStripMenuItem.Size = new System.Drawing.Size(56, 20);
            projectToolStripMenuItem.Text = "&Project";
            // 
            // newToolStripMenuItem
            // 
            newToolStripMenuItem.Image = global::NSass.Tool.Properties.Resources.New;
            newToolStripMenuItem.Name = "newToolStripMenuItem";
            newToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            newToolStripMenuItem.Size = new System.Drawing.Size(158, 22);
            newToolStripMenuItem.Text = "&New...";
            newToolStripMenuItem.Click += new System.EventHandler(this.NewProject);
            // 
            // editToolStripMenuItem
            // 
            editToolStripMenuItem.Image = global::NSass.Tool.Properties.Resources.Edit;
            editToolStripMenuItem.Name = "editToolStripMenuItem";
            editToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.E)));
            editToolStripMenuItem.Size = new System.Drawing.Size(158, 22);
            editToolStripMenuItem.Text = "&Edit...";
            // 
            // deleteToolStripMenuItem
            // 
            deleteToolStripMenuItem.Image = global::NSass.Tool.Properties.Resources.Delete;
            deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            deleteToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.D)));
            deleteToolStripMenuItem.Size = new System.Drawing.Size(158, 22);
            deleteToolStripMenuItem.Text = "&Delete...";
            // 
            // toolStripSeparator1
            // 
            toolStripSeparator1.Name = "toolStripSeparator1";
            toolStripSeparator1.Size = new System.Drawing.Size(155, 6);
            // 
            // startStopToolStripMenuItem
            // 
            startStopToolStripMenuItem.Image = global::NSass.Tool.Properties.Resources.Start;
            startStopToolStripMenuItem.Name = "startStopToolStripMenuItem";
            startStopToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            startStopToolStripMenuItem.Size = new System.Drawing.Size(158, 22);
            startStopToolStripMenuItem.Text = "&Start";
            // 
            // toolStripSeparator2
            // 
            toolStripSeparator2.Name = "toolStripSeparator2";
            toolStripSeparator2.Size = new System.Drawing.Size(155, 6);
            // 
            // exitToolStripMenuItem
            // 
            exitToolStripMenuItem.Image = global::NSass.Tool.Properties.Resources.Exit;
            exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            exitToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.F4)));
            exitToolStripMenuItem.Size = new System.Drawing.Size(158, 22);
            exitToolStripMenuItem.Text = "&Exit";
            exitToolStripMenuItem.Click += new System.EventHandler(this.Exit);
            // 
            // mainToolStrip
            // 
            mainToolStrip.BackColor = System.Drawing.SystemColors.Control;
            mainToolStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            newToolStripButton,
            editToolStripButton,
            deleteToolStripButton,
            toolStripSeparator3,
            startStopToolStripButton,
            toolStripSeparator4,
            exitToolStripButton});
            mainToolStrip.Location = new System.Drawing.Point(0, 24);
            mainToolStrip.Name = "mainToolStrip";
            mainToolStrip.Size = new System.Drawing.Size(784, 25);
            mainToolStrip.TabIndex = 1;
            // 
            // newToolStripButton
            // 
            newToolStripButton.Image = global::NSass.Tool.Properties.Resources.New;
            newToolStripButton.Name = "newToolStripButton";
            newToolStripButton.Size = new System.Drawing.Size(23, 22);
            newToolStripButton.ToolTipText = "New project...";
            newToolStripButton.Click += new System.EventHandler(this.NewProject);
            // 
            // editToolStripButton
            // 
            editToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            editToolStripButton.Image = global::NSass.Tool.Properties.Resources.Edit;
            editToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            editToolStripButton.Name = "editToolStripButton";
            editToolStripButton.Size = new System.Drawing.Size(23, 22);
            editToolStripButton.Text = "toolStripButton1";
            editToolStripButton.ToolTipText = "Edit project...";
            // 
            // deleteToolStripButton
            // 
            deleteToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            deleteToolStripButton.Image = global::NSass.Tool.Properties.Resources.Delete;
            deleteToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            deleteToolStripButton.Name = "deleteToolStripButton";
            deleteToolStripButton.Size = new System.Drawing.Size(23, 22);
            deleteToolStripButton.Text = "toolStripButton2";
            deleteToolStripButton.ToolTipText = "Delete project...";
            // 
            // toolStripSeparator3
            // 
            toolStripSeparator3.Name = "toolStripSeparator3";
            toolStripSeparator3.Size = new System.Drawing.Size(6, 25);
            // 
            // startStopToolStripButton
            // 
            startStopToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            startStopToolStripButton.Image = global::NSass.Tool.Properties.Resources.Start;
            startStopToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            startStopToolStripButton.Name = "startStopToolStripButton";
            startStopToolStripButton.Size = new System.Drawing.Size(23, 22);
            startStopToolStripButton.Text = "toolStripButton3";
            startStopToolStripButton.ToolTipText = "Start preprocessing";
            // 
            // toolStripSeparator4
            // 
            toolStripSeparator4.Name = "toolStripSeparator4";
            toolStripSeparator4.Size = new System.Drawing.Size(6, 25);
            // 
            // exitToolStripButton
            // 
            exitToolStripButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            exitToolStripButton.Image = global::NSass.Tool.Properties.Resources.Exit;
            exitToolStripButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            exitToolStripButton.Name = "exitToolStripButton";
            exitToolStripButton.Size = new System.Drawing.Size(23, 22);
            exitToolStripButton.Text = "Exit";
            exitToolStripButton.Click += new System.EventHandler(this.Exit);
            // 
            // projectsListView
            // 
            this.projectsListView.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.projectsListView.FullRowSelect = true;
            this.projectsListView.Location = new System.Drawing.Point(12, 52);
            this.projectsListView.MultiSelect = false;
            this.projectsListView.Name = "projectsListView";
            this.projectsListView.Size = new System.Drawing.Size(170, 498);
            this.projectsListView.TabIndex = 2;
            this.projectsListView.UseCompatibleStateImageBehavior = false;
            this.projectsListView.View = System.Windows.Forms.View.List;
            // 
            // logTextBox
            // 
            this.logTextBox.Location = new System.Drawing.Point(188, 52);
            this.logTextBox.Multiline = true;
            this.logTextBox.Name = "logTextBox";
            this.logTextBox.Size = new System.Drawing.Size(584, 498);
            this.logTextBox.TabIndex = 3;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 562);
            this.Controls.Add(this.logTextBox);
            this.Controls.Add(this.projectsListView);
            this.Controls.Add(mainToolStrip);
            this.Controls.Add(mainMenuStrip);
            this.MainMenuStrip = mainMenuStrip;
            this.Name = "MainForm";
            this.Text = "SASS/SCSS preprocessor";
            mainMenuStrip.ResumeLayout(false);
            mainMenuStrip.PerformLayout();
            mainToolStrip.ResumeLayout(false);
            mainToolStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView projectsListView;
        private System.Windows.Forms.TextBox logTextBox;
    }
}