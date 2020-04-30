
//#include "stdafx.h"
#include"MyWAKE.h"

#define DEFAULT_BAUD_RATE 9600 //значение скорости по умолчанию
#define DEFAULT_TIMEOUT 100    //timeout для TX и RX по умолчанию, мс
#define Signature 0xBEDA       //сигнатура для установки адреса



//---------------------------------------------------------------------------
//------------------ Реализация специфических команд: -----------------------
//---------------------------------------------------------------------------

#define FRAME     250   //размер буфера Wake

#define FEND     0xC0   //Frame END
#define FESC     0xDB   //Frame ESCape
#define TFEND    0xDC   //Transposed Frame END
#define TFESC    0xDD   //Transposed Frame ESCape

#define CRC_INIT 0xDE   //Initial CRC value

#define ERR_NO   0x00   //no error
#define ERR_TX   0x01   //Rx/Tx error
#define ERR_BU   0x02   //device busy error
#define ERR_RE   0x03   //device not ready error
#define ERR_PA   0x04   //parameters value error
#define ERR_NR   0x05   //no replay
#define ERR_NC   0x06   //no carrier

#define CMD_NOP     0   //нет операции
#define CMD_ERR     1   //ошибка приема пакета
#define CMD_ECHO    2   //передать эхо
#define CMD_INFO    3   //передать информацию об устройстве
#define CMD_SETADDR 4   //установить адрес
#define CMD_GETADDR 5   //прочитать адрес


#define CRC_Init 0xDE //CRC Initial value


//----------------------- Очистка буфера порта: -----------------------------


void WAKE_methods::Purge(void)
{
	if (StateCom)ComPort->DiscardOutBuffer();
	if (StateCom)ComPort->DiscardInBuffer();
	//throw gcnew System::NotImplementedException();
}


//----------------------------- Прием байта: --------------------------------


void WAKE_methods::RxByte(int &b)
{
	if (ComPort->BytesToRead) {
		b = ComPort->ReadByte();
	}
	else
	{
		Console::WriteLine("No bytes to read");
		b = 0;
	}
}

//--------------------------- Byte Stuffing: --------------------------------

void WAKE_methods::ByteStuff(unsigned char b, int &bptr,
	unsigned char *buff)
{
	if ((b == FEND) || (b == FESC))
	{
		buff[bptr++] = FESC;
		buff[bptr++] = (b == FEND) ? TFEND : TFESC;
	}
	else buff[bptr++] = b;
}

//-------------------------- Byte DeStuffing: -------------------------------

void WAKE_methods::ByteDeStuff(int &b)
{
	WAKE_methods::RxByte(b);
	//b = WAKE_methods::RxByte();
	if (b == FESC)
	{
		//		RxByte(b);
		if (b == TFEND) b = FEND;
		else if (b == TFESC) b = FESC;
		//	else throw EInOutError("WAKE destuffing error.");
	}
}

//--------------------------- Вычисление CRC: -------------------------------

void DowCRC(unsigned char b, unsigned char &crc)
{
	for (int i = 0; i < 8; i++)
	{
		if (((b ^ crc) & 1) != 0)
			crc = ((crc ^ 0x18) >> 1) | 0x80;
		else crc = (crc >> 1) & ~0x80;
		b = b >> 1;
	}
}

//--------------------------- Прием пакета: ---------------------------------

void WAKE_methods::RxFrame(unsigned char &Addr,
	unsigned char &Cmd,
	unsigned char &Nbt,
	unsigned char *Data)
{
	int DataPtr;                             //указатель буфера данных
	int data_byte = 0;                 //принятый байт
	unsigned char crc = CRC_INIT;            //контрольная сумма

	for (int i = 0; i < 512 && data_byte != FEND; i++)
		RxByte(data_byte);                     //поиск FEND
	if (data_byte != FEND)
		//throw gcnew System::Exception();
		Console::WriteLine("Can't find FEND word ");
	DowCRC(data_byte, crc);

	ByteDeStuff(data_byte);                  //прием адреса
	if (data_byte & 0x80)
	{
		Addr = data_byte & 0x7F;
		DowCRC(data_byte & 0x7F, crc);
		ByteDeStuff(data_byte);                //прием команды
	}
	else Addr = 0;
	Cmd = data_byte;
	DowCRC(data_byte, crc);

	ByteDeStuff(data_byte);                  //прием длины пакета
	Nbt = data_byte;
	DowCRC(data_byte, crc);
	for (DataPtr = 0; DataPtr < Nbt; DataPtr++)
	{
		ByteDeStuff(data_byte);                //прием данных
		Data[DataPtr] = data_byte;
		DowCRC(data_byte, crc);
	}
	ByteDeStuff(data_byte);                  //прием CRC
	DowCRC(data_byte, crc);
	if (crc)
		Console::WriteLine("WAKE CRC error");
}

//--------------------------- Передача пакета: ------------------------------

void WAKE_methods::TxFrame(unsigned char Addr,
	unsigned char Cmd,
	unsigned char Nbt,
	unsigned char *Data)
{
	unsigned char Buff[518];                 //буфер для передачи
	int  BuffPtr = 0;                    //указатель буфера
	unsigned char crc = CRC_INIT;            //контрольная сумма

											 //заполнение буфера:
	Buff[BuffPtr++] = FEND;                  //передача FEND-192
	DowCRC(FEND, crc);

	if (Addr)
	{
		ByteStuff(Addr | 0x80, BuffPtr, Buff); //передача адреса
		DowCRC(Addr & 0x7F, crc);
	}

	ByteStuff(Cmd & 0x7F, BuffPtr, Buff);    //передача команды
	DowCRC(Cmd & 0x7F, crc);

	ByteStuff(Nbt, BuffPtr, Buff);           //передача длины пакета
	DowCRC(Nbt, crc);

	for (int i = 0; i < Nbt; i++)
	{
		ByteStuff(Data[i], BuffPtr, Buff);     //передача данных
		DowCRC(Data[i], crc);
	}
	ByteStuff(crc, BuffPtr, Buff);           //передача CRC


											 //Перегружаем в управляемый массив. Это костыльный код.
	array <unsigned char> ^arr = gcnew array <unsigned char>(BuffPtr);
	for (int a=0; a < BuffPtr; a++) {
		arr[a] = Buff[a];
	}
	//передача буфера:

	WAKE_methods::Send_v(arr, arr->Length);	 //передача буфера:



}





//--------------------------- Передача команды: -----------------------------

void WAKE_methods::SendCmd(int a)
{
	/*
	unsigned char rxAdd = 0, rxCmd = 0, rxN = 0;
	int err;

	if (StateCom)                                 //если порт открыт
	{
	FTxAddr = a;                           //сохранение адреса
	TxFrame((unsigned char)FTxAddr,        //передача пакета
	(unsigned char)FTxCmd,
	(unsigned char)FTxPtr,
	FTxData);
	RxFrame(rxAdd,                         //прием пакета
	rxCmd,
	rxN,
	FRxData);

	if ((rxCmd == CMD_INFO) || (rxCmd == CMD_ECHO)) err = ERR_NO;
	else err = FRxData[0];
	if (err > ERR_NC) err = ERR_TX;
	if (rxAdd != FTxAddr) err = ERR_NC;
	else if (rxCmd != FTxCmd) err = ERR_PA;

	if (err)
	{
	AnsiString EStr = "Error of command " + FTxCmdName + ":\r";
	switch (err)
	{
	case ERR_TX: EStr += "invalid packet."; break;
	case ERR_BU: EStr += "devise busy."; break;
	case ERR_RE: EStr += "device not ready."; break;
	case ERR_PA: EStr += "invalid parameters."; break;
	case ERR_NC: EStr += "invalid address."; break;
	case ERR_NR: EStr += "timeout reached."; break;
	default: EStr += "unknown error.";
	}
	throw EInOutError(EStr);
	}
	}
	*/
}

//--------------------- Начало формирования пакета: -------------------------


//---------------------- Добавление байта в буфер: --------------------------


//---------------------- Добавление слова в буфер: --------------------------


//---------------- Добавление двойного слова в буфер: -----------------------



//-------------------- Добавление данных в буфер: ---------------------------

//-------------------- Чтение байта из буфера: ------------------------------


//-------------------- Чтение слова из буфера: ------------------------------



//--------------- Чтение двойного слова из буфера: --------------------------


//-------------------- Чтение данных из буфера: -----------------------------

//---------------------------------------------------------------------------
//---------------------------- Общие команды: -------------------------------
//---------------------------------------------------------------------------

//------------------------------ CMD_NOP: ------------------------------------

//Not used

//------------------------------ CMD_ERR: ------------------------------------

//Not used

//------------------------------ CMD_ECHO: -----------------------------------

//Not used

//------------------------------ CMD_INFO: -----------------------------------

//---------------------------------------------------------------------------

