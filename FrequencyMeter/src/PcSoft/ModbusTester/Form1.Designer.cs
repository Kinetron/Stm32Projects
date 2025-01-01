namespace FreqPC
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
		///  Required method for Designer support - do not modify
		///  the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			components = new System.ComponentModel.Container();
			labelResult = new Label();
			buttonRunСycle = new Button();
			timerReadData = new System.Windows.Forms.Timer(components);
			label1 = new Label();
			textBoxPortNumber = new TextBox();
			comboBoxParity = new ComboBox();
			label2 = new Label();
			textBoxСycleInterval = new TextBox();
			label3 = new Label();
			labelErr = new Label();
			labelError = new Label();
			labelTimeSpamTitle = new Label();
			labelTimeSpamValue = new Label();
			timerCustom = new System.Windows.Forms.Timer(components);
			SuspendLayout();
			// 
			// labelResult
			// 
			labelResult.AutoSize = true;
			labelResult.Location = new Point(62, 375);
			labelResult.Margin = new Padding(8, 0, 8, 0);
			labelResult.Name = "labelResult";
			labelResult.Size = new Size(108, 45);
			labelResult.TabIndex = 2;
			labelResult.Text = "Result";
			// 
			// buttonRunСycle
			// 
			buttonRunСycle.Location = new Point(568, 7);
			buttonRunСycle.Margin = new Padding(4);
			buttonRunСycle.Name = "buttonRunСycle";
			buttonRunСycle.Size = new Size(338, 60);
			buttonRunСycle.TabIndex = 3;
			buttonRunСycle.Text = "Cyclic survey";
			buttonRunСycle.UseVisualStyleBackColor = true;
			buttonRunСycle.Click += buttonRunСycle_Click;
			// 
			// timerReadData
			// 
			timerReadData.Tick += timerReadData_Tick;
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Location = new Point(18, 12);
			label1.Margin = new Padding(4, 0, 4, 0);
			label1.Name = "label1";
			label1.Size = new Size(278, 45);
			label1.TabIndex = 4;
			label1.Text = "Com port number";
			// 
			// textBoxPortNumber
			// 
			textBoxPortNumber.Location = new Point(306, 12);
			textBoxPortNumber.Margin = new Padding(4);
			textBoxPortNumber.Name = "textBoxPortNumber";
			textBoxPortNumber.Size = new Size(198, 51);
			textBoxPortNumber.TabIndex = 5;
			textBoxPortNumber.Text = "5";
			// 
			// comboBoxParity
			// 
			comboBoxParity.FormattingEnabled = true;
			comboBoxParity.Location = new Point(307, 78);
			comboBoxParity.Margin = new Padding(4);
			comboBoxParity.Name = "comboBoxParity";
			comboBoxParity.Size = new Size(196, 53);
			comboBoxParity.TabIndex = 7;
			// 
			// label2
			// 
			label2.AutoSize = true;
			label2.Location = new Point(140, 78);
			label2.Margin = new Padding(4, 0, 4, 0);
			label2.Name = "label2";
			label2.Size = new Size(100, 45);
			label2.TabIndex = 8;
			label2.Text = "Parity";
			// 
			// textBoxСycleInterval
			// 
			textBoxСycleInterval.Location = new Point(929, 16);
			textBoxСycleInterval.Margin = new Padding(4);
			textBoxСycleInterval.Name = "textBoxСycleInterval";
			textBoxСycleInterval.Size = new Size(174, 51);
			textBoxСycleInterval.TabIndex = 9;
			textBoxСycleInterval.Text = "300";
			// 
			// label3
			// 
			label3.AutoSize = true;
			label3.Location = new Point(1127, 22);
			label3.Margin = new Padding(4, 0, 4, 0);
			label3.Name = "label3";
			label3.Size = new Size(63, 45);
			label3.TabIndex = 10;
			label3.Text = "mc";
			// 
			// labelErr
			// 
			labelErr.AutoSize = true;
			labelErr.Location = new Point(62, 288);
			labelErr.Margin = new Padding(4, 0, 4, 0);
			labelErr.Name = "labelErr";
			labelErr.Size = new Size(103, 45);
			labelErr.TabIndex = 11;
			labelErr.Text = "Errors";
			// 
			// labelError
			// 
			labelError.AutoSize = true;
			labelError.Location = new Point(372, 288);
			labelError.Margin = new Padding(4, 0, 4, 0);
			labelError.Name = "labelError";
			labelError.Size = new Size(38, 45);
			labelError.TabIndex = 12;
			labelError.Text = "0";
			// 
			// labelTimeSpamTitle
			// 
			labelTimeSpamTitle.AutoSize = true;
			labelTimeSpamTitle.Location = new Point(62, 212);
			labelTimeSpamTitle.Name = "labelTimeSpamTitle";
			labelTimeSpamTitle.Size = new Size(204, 45);
			labelTimeSpamTitle.TabIndex = 13;
			labelTimeSpamTitle.Text = "Elapsed time";
			// 
			// labelTimeSpamValue
			// 
			labelTimeSpamValue.AutoSize = true;
			labelTimeSpamValue.Location = new Point(372, 212);
			labelTimeSpamValue.Name = "labelTimeSpamValue";
			labelTimeSpamValue.Size = new Size(38, 45);
			labelTimeSpamValue.TabIndex = 14;
			labelTimeSpamValue.Text = "0";
			// 
			// Form1
			// 
			AutoScaleDimensions = new SizeF(18F, 45F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(1217, 717);
			Controls.Add(labelTimeSpamValue);
			Controls.Add(labelTimeSpamTitle);
			Controls.Add(labelError);
			Controls.Add(labelErr);
			Controls.Add(label3);
			Controls.Add(textBoxСycleInterval);
			Controls.Add(label2);
			Controls.Add(comboBoxParity);
			Controls.Add(textBoxPortNumber);
			Controls.Add(label1);
			Controls.Add(buttonRunСycle);
			Controls.Add(labelResult);
			Font = new Font("Segoe UI", 14.1428576F, FontStyle.Regular, GraphicsUnit.Point);
			Margin = new Padding(8, 9, 8, 9);
			Name = "Form1";
			StartPosition = FormStartPosition.CenterScreen;
			Text = "Modbus frequency meter";
			FormClosed += Form1_FormClosed;
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion
		private BindingSource bindingSource1;
        private ImageList imageList1;
        private Label labelResult;
		private Button buttonRunСycle;
		private System.Windows.Forms.Timer timerReadData;
		private Label label1;
		private TextBox textBoxPortNumber;
		private ComboBox comboBoxParity;
		private Label label2;
		private TextBox textBoxСycleInterval;
		private Label label3;
		private Label labelErr;
		private Label labelError;
		private Label labelTimeSpamTitle;
		private Label labelTimeSpamValue;
		private System.Windows.Forms.Timer timerCustom;
	}
}