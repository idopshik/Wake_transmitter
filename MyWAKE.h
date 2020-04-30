#pragma once

using namespace System;
using namespace System::IO::Ports;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#define DEFAULT_BAUD_RATE 9600 //значение скорости по умолчанию
#define DEFAULT_TIMEOUT 100    //timeout для TX и RX по умолчанию, мс
#define Signature 0xBEDA       //сигнатура для установки адреса
#define RxBufSize 256

ref class WAKE_methods {

public:
	WAKE_methods() {													// конструктор без параметров.
		ComPort = gcnew System::IO::Ports::SerialPort();
	}



	void WAKE_methods::Purge(void);
	void WAKE_methods::ByteDeStuff(int &b);
	void WAKE_methods::RxFrame(unsigned char &Addr,
		unsigned char &Cmd,
		unsigned char &Nbt,
		unsigned char *Data);

	void WAKE_methods::TxFrame(unsigned char Addr,
		unsigned char Cmd,
		unsigned char Nbt,
		unsigned char *Data);

	void WAKE_methods::SendCmd(int a);
	void WAKE_methods::RxByte(int &b);
	void WAKE_methods::ByteStuff(unsigned char b, int &bptr,
		unsigned char *buff);

	int Send_v(array <unsigned char> ^ mas, int quantity) {

		if (ComPort->IsOpen) {
			ComPort->Write(mas, 0, quantity);

			return 1;
		}
		else {
			return 0;
		}
	}
	int Send_v(String^ str) {

		if (ComPort->IsOpen) {
			ComPort->WriteLine(str);
			return 1;
		}
		else {
			return 0;
		}
	}
	void sampleRead(void) {

		int IN = ComPort->BytesToRead;
		Console::WriteLine("\nRX buff contains {0} byte", IN);

		//String^ str1 = ComPort->ReadExisting();
		//	Console::WriteLine(str1);

		// IN = ComPort->BytesToRead;
		//	Console::WriteLine("\nRX buff contains {0} byte", IN);
	}
	void findPorts(void)
	{
		//получить имена портов
		array<Object^>^ objectArray = SerialPort::GetPortNames();

		Console::WriteLine("COM ports avalible  : {0}\n", sizeof(objectArray));

		for each (Object^s in objectArray) {
			Console::WriteLine(s);
		}

		//this->comboBox1->Items->AddRange(objectArray);//выведем массив доступных портов		
	}
	void ClosePort(String ^Port)
	{
		ComPort->Close();
	}
	int ComIsOpened()
	{
		if (ComPort->IsOpen) return 1;
		return 0;
	}
	int OpenPort(String ^Port)
	{
		ComPort->PortName = Port;
		try {
			if (!ComPort->IsOpen) {
				ComPort->BaudRate = DEFAULT_BAUD_RATE;
				ComPort->Open();
				StateCom = 1;
				return 1;
			}
		}
		catch (UnauthorizedAccessException^) {
			return 0;
		}
		return 1;
	}
	void infoRoutine(void)
	{

		ComPort->ReceivedBytesThreshold = 4;
		Console::WriteLine("ReceivedBytesThreshold  {0}", ComPort->ReceivedBytesThreshold);
		Console::WriteLine("Bits per byte {0}", ComPort->DataBits);
		//ComPort->DataReceived = infoRoutine();

	}



private:
	IO::Ports::SerialPort ^ComPort;
	int StateCom = 0;
	int FTxPtr;            //указатель буфера передачи
	array <unsigned char> ^RxBuf = gcnew array<unsigned char>(RxBufSize);

};


