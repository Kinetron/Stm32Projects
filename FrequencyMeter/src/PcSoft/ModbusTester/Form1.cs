using System.Diagnostics;
using System.IO.Ports;
using Modbus.Device;
using Modbus.Serial;

namespace FreqPC
{
	public partial class Form1 : Form
	{
		private const int SerialTimeout = 1000; //1second
		private const byte SlaveAddress = 172;
		private bool _serialIsOpen;

		private string _fileName;
		private Stopwatch _stopWatchTimer;

		private SerialPort _port;
		private SerialPortAdapter _adapter;
		private IModbusSerialMaster _master;
		private ModbusReader _modbusReader;
		public Form1()
		{
			InitializeComponent();

			comboBoxParity.Items.Add("None");
			comboBoxParity.Items.Add("Odd");
			comboBoxParity.Items.Add("Even");
			comboBoxParity.Items.Add("Mark");
			comboBoxParity.Items.Add("Space");

			comboBoxParity.SelectedIndex = 2;
			textBoxPortNumber.Text = "7";
			_stopWatchTimer = new Stopwatch();
		}
		
		/// <summary>
		/// Open port and config modbus.
		/// </summary>
		private void OpenPort()
		{
			if (_serialIsOpen) return;

			//Configure serial port
			_port = new SerialPort($"COM{textBoxPortNumber.Text.Trim()}");
			_port.BaudRate = 9600;//19200;//9600;//19200;//9600;
			_port.DataBits = 8;
			_port.Parity = (Parity)Enum.Parse(typeof(Parity), comboBoxParity.SelectedItem.ToString());
			_port.StopBits = StopBits.One;
			_port.Open();

			_adapter = new SerialPortAdapter(_port);
			//Create modbus master
			_master = ModbusSerialMaster.CreateRtu(_adapter);
			_master.Transport.ReadTimeout = _master.Transport.WriteTimeout = SerialTimeout;
			_serialIsOpen = true;

			_modbusReader = new ModbusReader(_master);
		}

		private void Form1_FormClosed(object sender, FormClosedEventArgs e)
		{
			timerReadData.Stop();
			if (_serialIsOpen)
			{
				_port.Close();
			}
		}
		
		private void buttonRunÑycle_Click(object sender, EventArgs e)
		{
			if (buttonRunÑycle.Text == "Stop survey")
			{
				timerReadData.Stop();
				_stopWatchTimer.Stop();
				_modbusReader.StopTimer();
				
				buttonRunÑycle.Text = "Cyclic survey";
				return;
			}

			DateTime dt = DateTime.Now;
			_fileName = $"measure_{dt.ToString("HH_mm_ss__dd_MM_yyyy")}.txt";

			timerReadData.Interval = int.Parse(textBoxÑycleInterval.Text);
			OpenPort();
			_modbusReader.CreateFileTitle(_fileName); //Ñreate file head.
			buttonRunÑycle.Text = "Stop survey";

			_stopWatchTimer.Start();
			timerReadData.Start();
			_modbusReader.StartTimer();
		}
		
		private void timerReadData_Tick(object sender, EventArgs e)
		{
			timerReadData.Stop();

			string freq = _modbusReader.ReadDevice(SlaveAddress, _fileName); //Òåêóùàÿ ÷àñòîòà.

			TimeSpan ts = _stopWatchTimer.Elapsed;
			// Format and display the TimeSpan value.
			string elapsedTime = String.Format("{0:00}:{1:00}:{2:00}.{3:00}",
				ts.Hours, ts.Minutes, ts.Seconds, ts.Milliseconds / 10);

			string errorCounter = _modbusReader.ErrorConter.ToString(); //Error counter.

			this.Invoke(() =>
			{
				labelResult.Text = freq + " Hz";
				labelTimeSpamValue.Text = elapsedTime;
				labelError.Text = errorCounter;
			});

			timerReadData.Start();
		}
	}
}