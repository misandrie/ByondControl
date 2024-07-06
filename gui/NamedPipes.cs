using System.IO.Pipes;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace gui
{
	class NamedPipes
    {

        static NamedPipeServerStream cmdPipeServer;
        static NamedPipeServerStream logPipeServer;

        public NamedPipes()
		{
            cmdPipeServer = new NamedPipeServerStream("byondcontrol13cmd", PipeDirection.InOut, 1, PipeTransmissionMode.Byte);
            logPipeServer = new NamedPipeServerStream("byondcontrol13log", PipeDirection.InOut, 1, PipeTransmissionMode.Message);

            Thread p_thread = new Thread(PipeThread);
            p_thread.IsBackground = true;
            p_thread.Start();
        }

        public async void PipeToDll(string data)
		{
            if (!cmdPipeServer.IsConnected)
            {
                var task = cmdPipeServer.WaitForConnectionAsync();
                if (await Task.WhenAny(task, Task.Delay(100)) != task)
				{
                    ByondControl.Current.SafeLog("pipe failure");
                    return;
				}
            }

            cmdPipeServer.Write(Encoding.ASCII.GetBytes(data), 0, data.Length);
            cmdPipeServer.Flush();
            cmdPipeServer.WaitForPipeDrain();
            cmdPipeServer.Disconnect();
        }

        public void PipeThread()
		{
            while (true)
			{
                logPipeServer.WaitForConnection();
                byte[] buffer = new byte[300];
                logPipeServer.Read(buffer, 0, 300);
                logPipeServer.Flush();
                logPipeServer.Disconnect();
                ByondControl.Current.SafeLog(Encoding.ASCII.GetString(buffer));
            }
        }
	}
}
