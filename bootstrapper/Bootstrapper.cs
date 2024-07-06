using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.IO.Compression;
using System.Security.Cryptography;
using Microsoft.Win32;

namespace bootstrapper
{
    public partial class Bootstrapper : Form
    {
        string binFolder;
        List<string> filesToGrab;
        bool installTheme = false;

        public Bootstrapper()
        {
            InitializeComponent();
        }

        static string MD5Hash(string file)
        {
            StringBuilder sb = new StringBuilder();

            using (var md5 = MD5.Create())
            {
                using (var stream = File.OpenRead(file))
                {
                    byte[] bytes = md5.ComputeHash(stream);
                    foreach (byte bt in bytes)
                    {
                        sb.Append(bt.ToString("x2"));
                    }
                }
            }

            return sb.ToString();
        }

        static string InitFolder(string folder)
        {
            folder = $"{Directory.GetCurrentDirectory()}\\{folder}\\";

            if (!Directory.Exists(folder))
                Directory.CreateDirectory(folder);

            return folder;
        }

        void Launch()
        {
            using (Process process = new Process())
            {
                process.StartInfo.FileName = $"{binFolder}\\gui.exe";
                process.StartInfo.WorkingDirectory = binFolder;
                process.Start();
            }

            Application.Exit();
        }

        private void SetStatus(string status)
        {
            if (this.InvokeRequired)
                this.Invoke(new MethodInvoker(delegate { SetStatus(status); }));
            else
                statusLabel.Text = status;
        }

        private void WebClient_DownloadProgressChanged(object sender, DownloadProgressChangedEventArgs e)
        {
            this.Invoke(new MethodInvoker(delegate
            {
                progressBar1.Value = e.ProgressPercentage;
            }));
        }

        private void WebClient_DownloadFileCompleted(object sender, AsyncCompletedEventArgs e)
        {
            // extract needed files
            using (ZipArchive archive = ZipFile.OpenRead("package.zip"))
            {
                foreach (ZipArchiveEntry entry in archive.Entries)
                {
                    if (filesToGrab.Contains(entry.Name))
                    {
                        entry.ExtractToFile(binFolder + entry.Name, true);
                    }
                }
            }
            // double check that everything went well
            MainThread();
        }

        Dictionary<string, string> GetFileHashes()
        {
            Dictionary<string, string> hashes = new Dictionary<string, string>();

            string hashData;

            using (WebClient webClient = new WebClient())
                hashData = webClient.DownloadString("http://austism.net/byondcontrol/download.php?version=latest&hash=1");

            using (StringReader reader = new StringReader(hashData))
            {
                string line;
                while ((line = reader.ReadLine()) != null)
                {
                    string[] split = line.Split(';');
                    hashes[split[0]] = split[1];
                }
            }

            return hashes;
        }

        void UpdateBootstrapper()
        {
            SetStatus("updating myself");

            string thisName = Process.GetCurrentProcess().MainModule.FileName;
            File.Move(thisName, "bootstrapper.old");

            using (WebClient webClient = new WebClient())
            {
                string downloadUrl = "http://austism.net/byondcontrol/download.php?version=bootstrapper";
                webClient.DownloadProgressChanged += WebClient_DownloadProgressChanged;
                webClient.DownloadFile(new Uri(downloadUrl), "byondcontrol.exe");
            }

            Process.Start("byondcontrol.exe");

            Environment.Exit(0);
        }

        List<string> CheckFiles()
        {
            List<string> filesToGrab = new List<string>();
            Dictionary<string, string> hashes = GetFileHashes();

            foreach (var item in hashes)
            {
                if (item.Key == "zip" || (item.Key == "theme.gif" && !installTheme))
                    continue;

                if (item.Key == "bootstrapper.exe")
                {
                    if (MD5Hash(Process.GetCurrentProcess().MainModule.FileName) != item.Value)
                        UpdateBootstrapper();

                    continue;
                }

                if (!File.Exists(binFolder + item.Key))
                {
                    filesToGrab.Add(item.Key);
                    continue;
                }

                if (MD5Hash(binFolder + item.Key) != item.Value)
                {
                    filesToGrab.Add(item.Key);
                    continue;
                }
            }

            return filesToGrab;
        }

        void MainThread()
        {
            if (File.Exists("bootstrapper.old"))
                File.Delete("bootstrapper.old");

            if (File.Exists("package.zip"))
                File.Delete("package.zip");

            filesToGrab = CheckFiles();
            Debug.WriteLine(filesToGrab.Count);
            if (filesToGrab.Count > 0)
            {
                SetStatus("downloading");
                using (WebClient webClient = new WebClient())
                {
                    string downloadUrl = "http://austism.net/byondcontrol/download.php?version=latest";
                    webClient.DownloadProgressChanged += WebClient_DownloadProgressChanged;
                    webClient.DownloadFileCompleted += WebClient_DownloadFileCompleted;
                    webClient.DownloadFileAsync(new Uri(downloadUrl), "package.zip");
                }
            }
            else
            {
                Launch();
            }
        }

        void FirstTimeSetup()
        {
            binFolder = InitFolder("bin");

            if (!File.Exists(binFolder + "theme.gif"))
                installTheme = true;

            if (MessageBox.Show($"Welcome to ByondControl!\nIs it okay of we setup inside of this folder?\n{Directory.GetCurrentDirectory()}", "First Time Setup", MessageBoxButtons.OKCancel) == DialogResult.Cancel)
            {
                Environment.Exit(0);
            }
            /*
            if (MessageBox.Show("BTW, Do you want us to automatically exclude ourself from Windows Defender?", "First Time Setup", MessageBoxButtons.YesNo) == DialogResult.Yes)
            {
                try
                {
                    using (Process ps = new Process())
                    {
                        ps.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
                        ps.StartInfo.FileName = "powershell.exe";
                        ps.StartInfo.Arguments = $"Add-MpPreference -ExclusionPath '{binFolder}'";
                        ps.StartInfo.UseShellExecute = true;
                        ps.StartInfo.Verb = "runas";
                        ps.Start();
                    }
                }
                catch { }
            }
            */
            using (RegistryKey key = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\ByondControl"))
            {
                key.SetValue("installdir", Directory.GetCurrentDirectory());
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            using (RegistryKey key = Registry.CurrentUser.CreateSubKey(@"SOFTWARE\ByondControl"))
            {
                string installDir = (string)key.GetValue("installdir");
                if (installDir == null || installDir != Directory.GetCurrentDirectory())
                    FirstTimeSetup();
            }

            binFolder = InitFolder("bin");
            InitFolder("autoexec");

            Thread thread = new Thread(MainThread);
            thread.IsBackground = true;
            thread.Start();
        }
    }
}
