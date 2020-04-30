#pragma once
//#include "stdafx.h"


#include"MyWAKE.h"

#using<system.dll>  

using namespace System;
using namespace System::IO;
#define JSI_SAVE_FILE 0
#define JSI_SAVE_FILE_AS 1
namespace JSI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
ref class Methods {								//Это класс .NET - он ведёт себя не так как класс С++
	OpenFileDialog ^openFile;
	SaveFileDialog ^saveFile;
	IO::StreamReader ^reader;
	IO::StreamWriter ^writer;
	String ^pathToFile = nullptr;


	WAKE_methods ^secondtry;
	unsigned char a2, a1, a0, n1, n0;
	String^ Com_port;
	IO::Ports::SerialPort ^ComPort;
	int StateCom = 0;
	

public:
	Methods() {													// конструктор без параметров.
		openFile = gcnew OpenFileDialog;
		openFile->Filter = "comma separated file |* .csv";
		saveFile = gcnew SaveFileDialog;
		saveFile->Filter = "comma separated file |* .csv";

		secondtry = gcnew WAKE_methods;
		ComPort = gcnew System::IO::Ports::SerialPort();
		Com_port = nullptr;
	}
	//Открываем порт 
	int OpenCom (String^ port)
	{
		Com_port = port;
		if (secondtry->OpenPort(Com_port))return 1;
		else return 0;

	}
	void CloseCom(String ^Port) {
		secondtry->ClosePort(Port);
	}
	int ComIsOpened()
	{
		if (secondtry->ComIsOpened()) return 1;
		return 0;
	}
	
	//Открываем файл | Open file
	void OpenFile(TextBox ^container) {
		
		container->Text = "";
		openFile->FileName = "";
		openFile->ShowDialog();
		if (openFile->FileName != "") {
			reader = gcnew IO::StreamReader(openFile->FileName);
			while (!reader->EndOfStream) {
				container->Text += reader->ReadLine() + Environment::NewLine;
			}
			reader->Close();
			delete reader;
		}
		pathToFile = openFile->FileName;
	}

	// Сохраням файл |save file
	void SaveFile(TextBox ^container, int mode) {
		switch (mode)
		{
		case JSI_SAVE_FILE: {


			if (!pathToFile|| pathToFile == "") {
				saveFile->FileName = "";
	 			saveFile->ShowDialog();
				if (saveFile->FileName == "")
					return;
				else
					pathToFile = saveFile->FileName;

				}
			break;
			}

		 case JSI_SAVE_FILE_AS: {
			saveFile->FileName = "";
			saveFile->ShowDialog();
			if (saveFile->FileName == "")return;
			else pathToFile = saveFile->FileName;
			break;
		}
		default:
			break;
		}
		writer = gcnew IO::StreamWriter(pathToFile);
		for (register int i = 0; i < container->Lines->Length; i++) {
			writer->WriteLine(container->Lines[i]);
		}
		writer->Close();
		delete writer;

	}

	// очистка контейнера clear container
	void Clear(TextBox ^anyword) {
		anyword->Clear();
	}
	void setAddr(unsigned char A2, unsigned char A1, unsigned char A0, unsigned char N1, unsigned char N0)
	{
		a2 = A2; a1 = A1; a0 = A0; n1 = N1; n0 = N0;
	}
	int setAddr(TextBox ^containerAddr)
	{
		using namespace System::Text;
		using namespace System::Globalization;
		try
		{
			int num = Int32::Parse(containerAddr->Text, NumberStyles::HexNumber);
			if (num > 0xFFFFFF) {
				throw gcnew IndexOutOfRangeException();
			}
			char tmp0, tmp1, tmp2;
			tmp0 = tmp1 = tmp2 = 0;
			a0 = num;
			num >>= 8;
			a1 = num;
			num >>= 8;
			a2 = num;
		
		}
		catch (Exception^ e)
		{
			(void)e;
			MessageBox::Show("Address incorrect!\n\n Input 3 hex nibbles\n without x or 0");
			return 0; // ошибка
		}
		return 1;
	}


	int Calculate_bytes(TextBox ^container)
	{
		int byte_counter = 0;
		String ^str = container->Text;
		int trig = 0;
		for each (char ch in str) {

			
			
				if ((ch == '0') && (!trig))
					{
						trig = 1;
					}
				else if (((ch == 'x')||(ch='X')) && (trig==1))
					{
						trig = 2;
					}
			
				else if (trig == 2)
				{			
					trig = 3;
				}
				else if (trig == 3) 
				{	
					byte_counter++;
					trig = 0;
				}
				else
				{
					trig = 0; 
				}
			}
	
		//MessageBox::Show(byte_counter.ToString(), "bytes");

		int tmp = byte_counter;
		n0 = tmp;  //lsb
		tmp >>= 8;
		n1 = tmp;
		return byte_counter;
	}
	void WakeTx_pic(TextBox ^container, ToolStripProgressBar ^Textcontained)
	{
		if (!secondtry->ComIsOpened())
				{
					MessageBox::Show("Com isn't opened!");
					return;
				}

			int TX_counter = 0;
			int RX_counter = 0;
		
			int trig = 0;
			char buf[2];
			unsigned char out_buf[8];
			unsigned char value;
			array <int> ^mng_buf = gcnew array <int>(8); // одномерный массив на 8
		
			unsigned char count = 7;
			unsigned char a, c, n = 0;
			unsigned char m[8];
			unsigned char u[] = { a2,a1,a0,n1,n0 };
			unsigned char k[] = { 0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA };
			int rest = 1024;
		
			int amount_of_bytes;
			//сколько байт?
			amount_of_bytes = n1;
			amount_of_bytes <<= 8;
			amount_of_bytes |= n0;

			//Вписываем адресс и количество.
			secondtry->TxFrame(0, 0x31, 5, u); 
			System::Threading::Thread::Sleep(50);
			secondtry->RxFrame(a, c, n, m);


		String ^str = container->Text;
		for each (char ch in str) {
		


		
				if ((ch == ' ') || (ch == '\n') || (ch == ',')) { trig = 0; }
				else if (ch == 'x')
				{

					trig = 1;
				}
				else {

					if (trig == 1)
					{
						//Console::WriteLine(ch);
						if ('0' <= ch && ch <= '9') buf[0] = ch - '0';
						else if ('a' <= ch && ch <= 'f') buf[0] = ch - 'a' + 10;
						else if ('A' <= ch && ch <= 'F') buf[0] = ch - 'A' + 10;
						trig = 2;
					}
					else if (trig == 2) {
						//Console::WriteLine(ch);
						trig = 0;
						if ('0' <= ch && ch <= '9') buf[1] = ch - '0';
						else if ('a' <= ch && ch <= 'f') buf[1] = ch - 'a' + 10;
						else if ('A' <= ch && ch <= 'F') buf[1] = ch - 'A' + 10;
						value = buf[0] * 16 + buf[1];

						out_buf[count] = value;
						count--;
					}
				}


				// Добавь проверку на некратность восьми!
				if ( (count < 0) || (count > 7))
				{
					count = 7;
					secondtry->TxFrame(0, 0x32, 8, out_buf); 
					TX_counter++;
					int tmp = 100 - (rest * 100) / amount_of_bytes;
					//String^ s = String::Format("{0}%", tmp);
					if ((tmp>=0)&&(tmp<=100)&&(tmp%2==0))Textcontained->Value = tmp;
					
					
					System::Threading::Thread::Sleep(50);
					secondtry->RxFrame(a, c, n, m);
					if (m[2] == 0)RX_counter++;
					m[2] = 0;
					rest = m[1] * 256 + m[0];
				
					Console::WriteLine(rest);
			
				}
				
		//Далее идёт скобка-конец блока for each!
		}
		//String^ message;
		//String^ messag = TX_counter.ToString();
		//String^ tmpMessage = RX_counter.ToString();
		String^ message = TX_counter.ToString() + "  frames sent\n" + RX_counter.ToString() + "  frames sussesfuly recieved\n\nDONE !";
	MessageBox::Show(message, "TxPicResult");
	}

	void ShowPic(ToolStripStatusLabel ^Textcontained) {

		if (!secondtry->ComIsOpened())
		{
			MessageBox::Show("Com isn't opened!");
			return;
		}
		
		unsigned char a, c, n = 0;
		unsigned char m[8];
		unsigned char u[] = { a2,a1,a0 };

		secondtry->TxFrame(0, 0x22, 3, u);
		System::Threading::Thread::Sleep(1500);
		secondtry->RxFrame(a, c, n, m);
		Textcontained->Text = "Cmd sent.";
		
	}

	void Clear4KByteBlock(ToolStripStatusLabel ^Textcontained) {
		if (!secondtry->ComIsOpened())
		{
			MessageBox::Show("Com isn't opened!");
			return;
		}
		unsigned char a, c, n = 0;
		unsigned char m[8];
		unsigned char u[] = { a2,a1,a0 };
		secondtry->TxFrame(0, 0x30, 3, u);
		System::Threading::Thread::Sleep(150);
		secondtry->RxFrame(a, c, n, m);
		System::Threading::Thread::Sleep(150);
		Textcontained->Text = "Block cleared";
	}


};

}
