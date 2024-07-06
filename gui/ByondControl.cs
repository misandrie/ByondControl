using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.Net;

namespace gui
{
	public partial class ByondControl : Form
	{
		[DllImport("serialMath.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool CalcSerialFromId(uint computerId, ref string output);

		public static ByondControl Current;
		private NamedPipes pipes;

		public void SafeLog(string text)
		{
			if (log.InvokeRequired)
			{
				log.Invoke((MethodInvoker)delegate { SafeLog(text); });
				return;
			}
			log.Items.Add(text);
			log.SelectedIndex = log.Items.Count - 1;
		}

		public void FatalError(string text)
		{
			if (InvokeRequired) {
				Invoke((MethodInvoker)delegate { FatalError(text); });
				return;
			}
			MessageBox.Show(text, "error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			Application.Exit();
		}

		private string serial
		{
			get
			{
				try { return (string)Registry.GetValue("HKEY_CURRENT_USER\\SOFTWARE\\Dantom\\BYOND", "hashspoof", new string('1', 32)); }
				catch (Exception ex) { FatalError("registry error 1, have you installed byond and opened the hub? " + ex.Message); }
				return new string('1', 32);
			}
			set
			{
				serialBox.Text = value;
				SafeLog("new serial hash: " + value);
				Registry.SetValue("HKEY_CURRENT_USER\\SOFTWARE\\Dantom\\BYOND", "hashspoof", value, RegistryValueKind.String);
			}
		}

		private void UpdateSettingsCheckboxes(int value)
        {
			spoofingDisabled.Checked = (value & 1) != 0;
			uiDisabled.Checked = (value & 2) != 0;
		}

		private void UpdateSettingsValue()
		{
			int value = (spoofingDisabled.Checked ? 1 : 0) | (uiDisabled.Checked ? 2 : 0);
			Registry.SetValue("HKEY_CURRENT_USER\\SOFTWARE\\Dantom\\BYOND", "bcsetting", value, RegistryValueKind.DWord);
		}

		private void RandomizeSerial()
        {
			Random rand = new Random();
			uint num1 = (uint)rand.Next() + (uint)rand.Next();
			uint num2 = (uint)rand.Next() + (uint)rand.Next();
			uint num3 = (uint)rand.Next() + (uint)rand.Next();
			uint num4 = (uint)rand.Next() + (uint)rand.Next();
			serial = num1.ToString("x") + num2.ToString("x") + num3.ToString("x") + num4.ToString("x");
		}

		static void ChangeIdentity()
		{
			string byondPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments) + @"\BYOND";

			// Kills byond and dreakseeker
			foreach (var proc in Process.GetProcessesByName("byond").Concat(Process.GetProcessesByName("dreamseeker"))) proc.Kill();

			// Deletes all directories inside cache
			foreach (var x in Directory.GetDirectories(byondPath + @"\cache"))
				Directory.Delete(x, true);

			// Deletes all cache files except for ones containing "byond.rsc"
			foreach (var x in Directory.GetFiles(byondPath + @"\cache"))
				if (!Path.GetFileName(x).Contains(".rsc"))
					File.Delete(x);

			// Deletes cauth.txt, sauth.txt, key.txt
			foreach (var x in new string[] { "cfg\\cauth.txt", "cfg\\sauth.txt", "key.txt" })
            {
				string file = $@"{byondPath}\{x}";
				if (File.Exists(file))
					File.Delete(file);
            }

			// Deletes registry values sid, key-info, key-style
			using (RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Software\Dantom\BYOND", true))
				foreach (string value in new string[] { "sid", "key-info", "key-style" })
					if (key.GetValue(value) != null)
						key.DeleteValue(value);

			// Deletes Internet Explorer Cache & Cookies
			Process.Start("RunDll32.exe", "InetCpl.cpl,ClearMyTracksByProcess 266");

			Current.SafeLog("Done changing identity!");
		}

		private void MainThread()
		{
			Injector.Start();

			try
			{
				using (WebClient wc = new WebClient())
				{
					string motd = wc.DownloadString("http://www.austism.net/byondcontrol/motd.php");
					if (motd.Length > 0)
						this.SafeLog(motd);
				}
			}
			catch { }
		}

		public ByondControl()
		{
			InitializeComponent();
			Current = this;

			this.Text = "ByondControl";
			this.serialBox.Text = serial;

			try { UpdateSettingsCheckboxes((int)Registry.GetValue("HKEY_CURRENT_USER\\SOFTWARE\\Dantom\\BYOND", "bcsetting", 0)); }
			catch (Exception ex) { FatalError("registry error 2, have you installed byond and opened the hub? " + ex.Message); }
			

			string[] files = Directory.GetFiles(Directory.GetCurrentDirectory(), "theme*", SearchOption.TopDirectoryOnly);
			if (files.Length > 0)
				LoadTheme(files[0]);

			this.pipes = new NamedPipes();

			Thread m_thread = new Thread(MainThread);
			m_thread.IsBackground = true;
			m_thread.Start();
			this.Show();

			if (File.Exists("version.txt"))
			{
				this.Text = $"BondControl v{File.ReadAllText("version.txt")}";
			}

			SafeLog("whats up?");
		}


		private void LoadTheme(string file)
		{
			try
			{
				theme.Image = Image.FromFile(file);
			}
			catch (Exception)
			{
				SafeLog("theme load failed");
			}
		}

		private void changeserial_Click(object sender, EventArgs e)
		{
			if (MessageBox.Show("WARNING: changing your serial will change your computer_id in byond. some servers will ban you if you have previously logged in with a different computer id", "warning", MessageBoxButtons.OKCancel) == DialogResult.OK)
				RandomizeSerial();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);
			Application.Exit();
		}

		private void killbutton_Click(object sender, EventArgs e)
		{
			foreach (var proc in Process.GetProcessesByName("dreamseeker"))
			{
				proc.Kill();
			}
		}

        private void hashCalc_Click(object sender, EventArgs e)
        {
			string promptResult = Prompt.ShowDialog("id:", "enter a computer id to find the hash of. warning: SLOW");
			try
            {
				uint computerId = UInt32.Parse(promptResult);
				SafeLog("calculating for " + computerId);
				string result = new string('1', 32);
				bool success = CalcSerialFromId(computerId, ref result);
				if (success)
				{
					SafeLog("computation success: " + result);
					serial = result;
				} else {
					SafeLog("computation failed, exceeded allowed computation attempts");
				}
			} catch (Exception ex)
            {
				SafeLog("invalid input or error: " + ex.Message);
            }
        }

        private void spoofingDisabled_CheckedChanged(object sender, EventArgs e)
        {
			UpdateSettingsValue();
		}

        private void uiDisabled_CheckedChanged(object sender, EventArgs e)
        {
			UpdateSettingsValue();
		}

        private void serialBox_FocusLost(object sender, EventArgs e)
        {
			char[] txt = serialBox.Text.ToLower().ToCharArray();
			string built = "";
			for (int i = 0; i < 32; ++i)
            {
				if (i < txt.Length && (txt[i] >= '0' && txt[i] <= '9' || txt[i] >= 'a' && txt[i] <= 'f')) {
					built += txt[i];
				} else
                {
					built += '1';
                }
            }
			serial = built;
        }

		private void evadebutton_Click(object sender, EventArgs e)
		{
			if (MessageBox.Show("This will:\n1. Kill byond\n2. Log you out of byond\n3. Delete some cache files from byond\n4. Delete Internet Explorer cookies and cache\n5. Change your computer id\nREMEMBER TO CHANGE YOUR IP ADDRESS! :)", "warning", MessageBoxButtons.OKCancel) == DialogResult.OK)
			{
				ChangeIdentity();
				RandomizeSerial();
			}
		}
    }

    public static class Prompt
	{
		public static string ShowDialog(string text, string caption)
		{
			Form prompt = new Form()
			{
				Width = 500,
				Height = 150,
				FormBorderStyle = FormBorderStyle.FixedDialog,
				Text = caption,
				StartPosition = FormStartPosition.CenterScreen
			};
			Label textLabel = new Label() { Left = 50, Top = 20, Text = text };
			TextBox textBox = new TextBox() { Left = 50, Top = 50, Width = 400 };
			Button confirmation = new Button() { Text = "Ok", Left = 350, Width = 100, Top = 70, DialogResult = DialogResult.OK };
			confirmation.Click += (sender, e) => { prompt.Close(); };
			prompt.Controls.Add(textBox);
			prompt.Controls.Add(confirmation);
			prompt.Controls.Add(textLabel);
			prompt.AcceptButton = confirmation;

			return prompt.ShowDialog() == DialogResult.OK ? textBox.Text : "";
		}
	}
}
