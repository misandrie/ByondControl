using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace gui
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>

		static ByondControl bc;

		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Start();
			Application.Run();
		}

		public static bool IsAdministrator()
		{
			var identity = WindowsIdentity.GetCurrent();
			var principal = new WindowsPrincipal(identity);
			return principal.IsInRole(WindowsBuiltInRole.Administrator);
		}

		static void Start()
		{
			if (!Auth.KeySysEnabled())
			{
				bc = new ByondControl();
				bc.Show();
				return;
			}

			string key = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";//i think this fixes an error
			if (Auth.GetSavedKey(ref key))
			{
				if (Auth.CheckKey(key) == 1)
				{
					bc = new ByondControl();
					bc.Show();
					return;
				}
			}

			new Auth().Show();
		}
	}
}
