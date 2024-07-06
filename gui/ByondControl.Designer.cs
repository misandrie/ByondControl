
namespace gui
{
	partial class ByondControl
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ByondControl));
            this.changeserial = new System.Windows.Forms.Button();
            this.log = new System.Windows.Forms.ListBox();
            this.serial_label = new System.Windows.Forms.Label();
            this.theme = new System.Windows.Forms.PictureBox();
            this.killbutton = new System.Windows.Forms.Button();
            this.serialBox = new System.Windows.Forms.TextBox();
            this.spoofingDisabled = new System.Windows.Forms.CheckBox();
            this.hashCalc = new System.Windows.Forms.Button();
            this.uiDisabled = new System.Windows.Forms.CheckBox();
            this.evadebutton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.theme)).BeginInit();
            this.SuspendLayout();
            // 
            // changeserial
            // 
            this.changeserial.Location = new System.Drawing.Point(12, 7);
            this.changeserial.Name = "changeserial";
            this.changeserial.Size = new System.Drawing.Size(83, 23);
            this.changeserial.TabIndex = 0;
            this.changeserial.Text = "random serial";
            this.changeserial.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.changeserial.UseVisualStyleBackColor = true;
            this.changeserial.Click += new System.EventHandler(this.changeserial_Click);
            // 
            // log
            // 
            this.log.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(50)))), ((int)(((byte)(50)))), ((int)(((byte)(50)))));
            this.log.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.log.ForeColor = System.Drawing.Color.White;
            this.log.FormattingEnabled = true;
            this.log.HorizontalExtent = 5000;
            this.log.HorizontalScrollbar = true;
            this.log.ItemHeight = 15;
            this.log.Location = new System.Drawing.Point(12, 105);
            this.log.Name = "log";
            this.log.Size = new System.Drawing.Size(286, 154);
            this.log.TabIndex = 1;
            // 
            // serial_label
            // 
            this.serial_label.AutoSize = true;
            this.serial_label.BackColor = System.Drawing.SystemColors.Control;
            this.serial_label.Location = new System.Drawing.Point(101, 12);
            this.serial_label.Name = "serial_label";
            this.serial_label.Size = new System.Drawing.Size(60, 13);
            this.serial_label.TabIndex = 2;
            this.serial_label.Text = "serial hash:";
            // 
            // theme
            // 
            this.theme.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.theme.Location = new System.Drawing.Point(0, 0);
            this.theme.Name = "theme";
            this.theme.Size = new System.Drawing.Size(311, 271);
            this.theme.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.theme.TabIndex = 4;
            this.theme.TabStop = false;
            // 
            // killbutton
            // 
            this.killbutton.Location = new System.Drawing.Point(215, 76);
            this.killbutton.Name = "killbutton";
            this.killbutton.Size = new System.Drawing.Size(83, 23);
            this.killbutton.TabIndex = 6;
            this.killbutton.Text = "kill dseeker";
            this.killbutton.UseVisualStyleBackColor = true;
            this.killbutton.Click += new System.EventHandler(this.killbutton_Click);
            // 
            // serialBox
            // 
            this.serialBox.Location = new System.Drawing.Point(164, 9);
            this.serialBox.Name = "serialBox";
            this.serialBox.Size = new System.Drawing.Size(138, 20);
            this.serialBox.TabIndex = 7;
            this.serialBox.LostFocus += new System.EventHandler(this.serialBox_FocusLost);
            // 
            // spoofingDisabled
            // 
            this.spoofingDisabled.AutoSize = true;
            this.spoofingDisabled.Location = new System.Drawing.Point(104, 39);
            this.spoofingDisabled.Name = "spoofingDisabled";
            this.spoofingDisabled.Size = new System.Drawing.Size(146, 17);
            this.spoofingDisabled.TabIndex = 8;
            this.spoofingDisabled.Text = "disable computer id spoof";
            this.spoofingDisabled.UseVisualStyleBackColor = true;
            this.spoofingDisabled.CheckedChanged += new System.EventHandler(this.spoofingDisabled_CheckedChanged);
            // 
            // hashCalc
            // 
            this.hashCalc.Location = new System.Drawing.Point(13, 35);
            this.hashCalc.Name = "hashCalc";
            this.hashCalc.Size = new System.Drawing.Size(83, 23);
            this.hashCalc.TabIndex = 9;
            this.hashCalc.Text = "calc from id";
            this.hashCalc.UseVisualStyleBackColor = true;
            this.hashCalc.Click += new System.EventHandler(this.hashCalc_Click);
            // 
            // uiDisabled
            // 
            this.uiDisabled.AutoSize = true;
            this.uiDisabled.BackColor = System.Drawing.SystemColors.Control;
            this.uiDisabled.Location = new System.Drawing.Point(12, 82);
            this.uiDisabled.Name = "uiDisabled";
            this.uiDisabled.Size = new System.Drawing.Size(110, 17);
            this.uiDisabled.TabIndex = 10;
            this.uiDisabled.Text = "disable ingame UI";
            this.uiDisabled.UseVisualStyleBackColor = false;
            this.uiDisabled.CheckedChanged += new System.EventHandler(this.uiDisabled_CheckedChanged);
            // 
            // evadebutton
            // 
            this.evadebutton.Location = new System.Drawing.Point(128, 76);
            this.evadebutton.Name = "evadebutton";
            this.evadebutton.Size = new System.Drawing.Size(81, 23);
            this.evadebutton.TabIndex = 11;
            this.evadebutton.Text = "evade ban";
            this.evadebutton.UseVisualStyleBackColor = true;
            this.evadebutton.Click += new System.EventHandler(this.evadebutton_Click);
            // 
            // ByondControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(310, 269);
            this.Controls.Add(this.evadebutton);
            this.Controls.Add(this.uiDisabled);
            this.Controls.Add(this.hashCalc);
            this.Controls.Add(this.spoofingDisabled);
            this.Controls.Add(this.serialBox);
            this.Controls.Add(this.killbutton);
            this.Controls.Add(this.serial_label);
            this.Controls.Add(this.log);
            this.Controls.Add(this.changeserial);
            this.Controls.Add(this.theme);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ByondControl";
            this.Text = "ByondControl";
            ((System.ComponentModel.ISupportInitialize)(this.theme)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Button changeserial;
		private System.Windows.Forms.ListBox log;
		private System.Windows.Forms.Label serial_label;
		private System.Windows.Forms.PictureBox theme;
		private System.Windows.Forms.Button killbutton;
        private System.Windows.Forms.TextBox serialBox;
        private System.Windows.Forms.CheckBox spoofingDisabled;
        private System.Windows.Forms.Button hashCalc;
        private System.Windows.Forms.CheckBox uiDisabled;
        private System.Windows.Forms.Button evadebutton;
    }
}

