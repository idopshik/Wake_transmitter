#pragma once
#include "stdafx.h"


#include"MyWAKE.h"

#using<system.dll>  
using namespace System;
using namespace System::IO;


ref class PicWakeTX {
private:
	WAKE_methods ^secondtry;
	unsigned char a2, a1, a0, n1, n0;
	String^ Com_port;
	String ^pathToFile = nullptr;
public:

	PicWakeTX(String^ port) 
	{
		secondtry = gcnew WAKE_methods;
		Com_port = port;
	}

	void setAddr(unsigned char A2, unsigned char A1, unsigned char A0, unsigned char N1, unsigned char N0)
		{
			a2 = A2; a1 = A1; a0 = A0; n1 = N1; n0 = N0;
		}
	
	void setFilePath(String ^path)
	{
		pathToFile = path;
	}
	
	void Prepare_to_write() {
		
		if (secondtry->OpenPort("COM1"))Console::WriteLine("Port has been opened");
		else Console::WriteLine("COM1 not availible! ");

		unsigned char a, c, n = 0;
		unsigned char m[8];

		unsigned char u[] = { a2,a1,a0,n1,n0 };

		System::Threading::Thread::Sleep(3000);


		//Стираем
		secondtry->TxFrame(0, 0x30, 3, u);
		System::Threading::Thread::Sleep(500);
		secondtry->RxFrame(a, c, n, m);
		System::Threading::Thread::Sleep(150);
		Console::WriteLine("cleared");


		//Пошлём команду, куда писать.
		secondtry->TxFrame(0, 0x31, 5, u);


		System::Threading::Thread::Sleep(150);
		secondtry->RxFrame(a, c, n, m);
		System::Threading::Thread::Sleep(150);
		Console::WriteLine("address saved");

		secondtry->ClosePort(Com_port); //////////////////////////////////////
	}
	
	void ShowPic(unsigned char a2, unsigned char a1, unsigned char a0) {

		
		if (secondtry->OpenPort(Com_port))Console::WriteLine("Port has been opened");
		else Console::WriteLine("{0} not availible!", Com_port);

		unsigned char a, c, n = 0;
		unsigned char m[8];

		unsigned char u[] = { a2,a1,a0 };

		secondtry->TxFrame(0, 0x22, 3, u);
		System::Threading::Thread::Sleep(1500);
		secondtry->RxFrame(a, c, n, m);
		System::Threading::Thread::Sleep(3000);
		Console::WriteLine("showed");

		secondtry->ClosePort(Com_port); //////////////////////////////////////
	}
	void ShowPic() {


		if (secondtry->OpenPort(Com_port))Console::WriteLine("Port has been opened");
		else Console::WriteLine("{0} not availible!", Com_port);

		unsigned char a, c, n = 0;
		unsigned char m[8];

		unsigned char u[] = { a2,a1,a0 };

		secondtry->TxFrame(0, 0x22, 3, u);
		System::Threading::Thread::Sleep(1500);
		secondtry->RxFrame(a, c, n, m);
		System::Threading::Thread::Sleep(3000);
		Console::WriteLine("showed");

		secondtry->ClosePort(Com_port); //////////////////////////////////////
	}
	void Clear4KByteBlock() {


		if (secondtry->OpenPort(Com_port))Console::WriteLine("Port has been opened");
		else Console::WriteLine("{0} not availible!", Com_port);

		unsigned char a, c, n = 0;
		unsigned char m[8];

		unsigned char u[] = { a2,a1,a0 };

		secondtry->TxFrame(0, 0x30, 3, u);
		System::Threading::Thread::Sleep(150);
		secondtry->RxFrame(a, c, n, m);
		System::Threading::Thread::Sleep(150);
		Console::WriteLine("showed");

		secondtry->ClosePort(Com_port); //////////////////////////////////////
	}
	void Tx_pic()
	{
		
		if (secondtry->OpenPort(Com_port))Console::WriteLine("Port has been opened");
		else Console::WriteLine("{0} not availible!", Com_port);
	
		try
		{
			Console::WriteLine("trying to open file {0}...", pathToFile);
			StreamReader^ din = File::OpenText(pathToFile);

			char ch;
			int trig = 0;
			char buf[2];
			unsigned char out_buf[8];

			unsigned char value;

			array <int> ^mng_buf = gcnew array <int>(8); // одномерный массив
			String^ str;
			unsigned char count = 7;
			unsigned char a, c, n = 0;
			unsigned char m[8];



			//Console::WriteLine("Attention wake sending");
			unsigned char u[] = { 0x1E,0x80,0x00,0x04,0x00,0x00 };
			unsigned char k[] = { 0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA };
			int rest;
			secondtry->TxFrame(0, 0x31, 5, u); // Показываем картику
			System::Threading::Thread::Sleep(50);
			secondtry->RxFrame(a, c, n, m);


			while (din->Peek() >= 0)
			{
				ch = (Char)din->Read();


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


				//Console::WriteLine(out_buf[count]);


				//System::Threading::Thread::Sleep(1000);

				if ((din->Peek() < 0) || (count < 0) || (count > 7))
				{

					count = 7;

					secondtry->TxFrame(0, 0x32, 8, out_buf); // Показываем картику
					System::Threading::Thread::Sleep(50);
					secondtry->RxFrame(a, c, n, m);

					rest = m[1] * 256 + m[0];
					//Console::WriteLine(" ");
					Console::WriteLine(rest);
					if (rest < 1);

					//Console::WriteLine("  {0}  {1}  {2}  {3}  {4}  {5}  {6} {7} ", out_buf[0], out_buf[1], out_buf[2], out_buf[3], out_buf[4], out_buf[5], out_buf[6], out_buf[7]);



				}
			}
		}
		catch (Exception^ e)
		{
			if (dynamic_cast<FileNotFoundException^>(e))
				Console::WriteLine("file '{0}' not found", pathToFile);
			else
				Console::WriteLine("problem reading file '{0}'", pathToFile);
		}



		secondtry->ClosePort(Com_port);
		
	}

};
