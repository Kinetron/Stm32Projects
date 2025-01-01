using System.Diagnostics;
using Modbus.Device;

namespace FreqPC
{
	internal class ModbusReader
	{
		private Stopwatch _stopWatchTimer;
		private IModbusSerialMaster _master;
		private int _lineCount = 1;
		public int ErrorConter = 0;

		public ModbusReader(IModbusSerialMaster master)
		{
			_stopWatchTimer = new Stopwatch();
			_master = master;
		}

		/// <summary>
		/// Run inner timer.
		/// </summary>
		public void StartTimer()
		{
			_stopWatchTimer.Start();
		}

		public void StopTimer()
		{
			_stopWatchTimer.Stop();
		}
		
		public void CreateFileTitle(string fileName)
		{
			using (StreamWriter w = File.AppendText(fileName))
			{
				w.WriteLine($"LineNumber Seconds Frequency,Hz");
			}
		}
		
		public string ReadDevice(byte slaveId, string fileName)
		{
			ushort[] modBusregisters = null;
			string frequencyStr = "";

			try
			{
				modBusregisters = _master.ReadInputRegisters(slaveId, 0, 10);
				byte[] asByte = new byte[4];
				Buffer.BlockCopy(modBusregisters, 0, asByte, 0, 4);
				double frequency = BitConverter.ToSingle(asByte, 0);

				frequencyStr = frequency.ToString("0.00000");
				
				TimeSpan ts = _stopWatchTimer.Elapsed;
				var seconds = ts.TotalSeconds;
				
				using (StreamWriter w = File.AppendText(fileName))
				{
					w.WriteLine($"{_lineCount} {seconds.ToString("0.000")} {frequencyStr}");
				}

				_lineCount++;
			}
			catch (Exception ex)
			{
				//TO DO: Add error log!
				ErrorConter++;
			}

			return frequencyStr;
		}
	}
}
