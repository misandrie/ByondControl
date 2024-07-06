using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

namespace gui
{
	public partial class Auth : Form
	{
		[DllImport("authlib.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern int CheckKey(string key);
		[DllImport("authlib.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool SaveKey(string key);
		[DllImport("authlib.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool GetSavedKey(ref string key);
		[DllImport("authlib.dll", CallingConvention = CallingConvention.Cdecl)]
		public static extern bool KeySysEnabled();//BUG: THIS ALWAYS RETURNS TRUE

		public Auth()
		{
			InitializeComponent();
		}

		private void submit_Click(object sender, EventArgs e)
		{
			string key = textBox1.Text;
			key = string.Concat(key.Where(c => !char.IsWhiteSpace(c)));
			if (key == string.Empty)
			{
				status.Text = "enter a key retard";
				return;
			}

			int res = CheckKey(key);
			switch (res)
			{
				case 1:
					status.Text = "ok";
					break;
				case 2:
					status.Text = "invalid key";
					break;
				case 3:
					status.Text = "key used on another computer";
					break;
				default:
					status.Text = "something went wrong " + res;
					break;
			}

			if (res == 1)
			{
				SaveKey(key);
				new ByondControl().Show();
				this.Hide();
			}
		}

		private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
		{
			Process.Start("http://www.austism.net/byondcontrol/trial/checkpoint1.php");
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);
			Application.Exit();
		}
	}
}
