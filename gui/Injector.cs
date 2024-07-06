using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace gui
{
	class Injector
	{
		[DllImport("kernel32.dll")]
		public static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

		[DllImport("kernel32.dll", CharSet = CharSet.Auto)]
		public static extern IntPtr GetModuleHandle(string lpModuleName);

		[DllImport("kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
		static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

		[DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
		static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

		[DllImport("kernel32.dll", SetLastError = true)]
		static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, UIntPtr lpNumberOfBytesWritten);

		[DllImport("kernel32.dll")]
		static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

		[DllImport("kernel32.dll")]
		public static extern bool CloseHandle(IntPtr hThread);

		const int PROCESS_ALL_ACCESS = (int)(0x000F0000L | 0x00100000L | 0xFFFF);
		const uint MAX_PATH = 260;
		const uint MEM_COMMIT = 0x00001000;
		const uint MEM_RESERVE = 0x00002000;
		const uint PAGE_READWRITE = 0x04;
		
		private static bool Inject(int procId, string dllPath)
		{
			try
			{
				IntPtr hProc = OpenProcess(PROCESS_ALL_ACCESS, false, procId);

				{
					// make BYOND load dlls from ByondControl directory
					// without this, the injected dll won't find authlib.dll
					IntPtr alloc = VirtualAllocEx(hProc, IntPtr.Zero, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
					WriteProcessMemory(hProc, alloc, Encoding.Default.GetBytes(Directory.GetCurrentDirectory()), (uint)dllPath.Length + 1, UIntPtr.Zero);
					IntPtr setDllDirectoryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "SetDllDirectoryA");
					IntPtr hThread = CreateRemoteThread(hProc, IntPtr.Zero, 0, setDllDirectoryAddr, alloc, 0, IntPtr.Zero);
					CloseHandle(hThread);
				}
				Thread.Sleep(100);
				{
					// allocate some memory to hold the path to the dll
					IntPtr alloc = VirtualAllocEx(hProc, IntPtr.Zero, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
					// write the dll path into the allocated memory block
					WriteProcessMemory(hProc, alloc, Encoding.Default.GetBytes(dllPath), (uint)dllPath.Length + 1, UIntPtr.Zero);
					// make the target process run LoadLibrary with the injected dll path
					IntPtr loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
					IntPtr hThread = CreateRemoteThread(hProc, IntPtr.Zero, 0, loadLibraryAddr, alloc, 0, IntPtr.Zero);
					CloseHandle(hThread);
				}

				return true;
			}
			catch (Exception)
			{
				return false;
			}
		}

		static List<int> Injected = new List<int>();

		private static void InjectorThread()
		{
			string dllPath = Path.Combine(Directory.GetCurrentDirectory(), "main.dll");

			if (!File.Exists(dllPath))
			{
				ByondControl.Current.FatalError("main.dll is missing");
				return;
			}

			while (true)
			{
				// Check Injected list and see if the processes are still open.
				// ToList is needed for some reason.
				foreach (int procId in Injected.ToList())
				{
					try {Process.GetProcessById(procId);}
					catch (Exception)
					{
						int index = Injected.FindIndex(x => x == procId);
						if (index != -1)
							Injected.RemoveAt(index);
					}
				}

				// Inject new processes
				foreach (Process process in Process.GetProcessesByName("dreamseeker"))
				{
					if (Injected.FindIndex(x => x == process.Id) != -1)
						continue;

					if (Inject(process.Id, dllPath))
					{
						Injected.Add(process.Id);
						ByondControl.Current.SafeLog("attached a client");
					}
				}

				Thread.Sleep(10);
			}
		}

		public static void Start()
		{
			Thread injection_thread = new Thread(InjectorThread);
			injection_thread.IsBackground = true;
			injection_thread.Start();
		}
	}
}
