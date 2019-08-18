#include "Lang.h"
#include "idu.h"


#define TotalMsg 27
#define START 0xbfc10000
ST_LANG sLang;


// English
ST_LANG_TABLE EnglishTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " Language"},
	{Msg_ENGLISH, " English  "},
	{Msg_FRENCH, " French  "},
	{Msg_GERMAN, " German  "},
	{Msg_ITALIAN, " Italian  "},
	{Msg_JAPANESE, " Japanese  "},
	{Msg_KOREAN, " Korean  "},
	{Msg_PORTUGESE, " Portuguese  "},
	{Msg_RUSSIAN, " Russian  "},
	{Msg_SPANISH, " Spanish  "},
	{Msg_SIMCHINESE, " Simplified Chinese  "},
	{Msg_TRACHINESE, " Traditional Chinese   "},
	{Msg_Firmware_Update, " Update Firmware "},
	{Msg_Drive_Quick_Format, " Format Device"},
	{Msg_Zoom, " Zoom"},
	{Msg_Zoom_In, " Zoom In"},
	{Msg_Zoom_Out, " Zoom Out"},
	{Msg_Exit, " Exit"},
	{Msg_Not_Enough_Space, " Not enough space !"},
	{Msg_No_Medium_Exist, " Empty Files!!"},
	{Msg_Format_Not_Support, " Format Not Supported!"}
};


// French
ST_LANG_TABLE FrenchTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " Language"},
	{Msg_ENGLISH, " Anglais"},
	{Msg_FRENCH, " Fran�ais"},
	{Msg_GERMAN, " Allemand"},
	{Msg_ITALIAN, " Italien"},
	{Msg_JAPANESE, " Japonais"},
	{Msg_KOREAN, " Cor�en"},
	{Msg_PORTUGESE, " Portugais"},
	{Msg_RUSSIAN, " Russe"},
	{Msg_SPANISH, " Espagnol"},
	{Msg_SIMCHINESE, " Chinois simple"},
	{Msg_TRACHINESE, " Chinois Traditionnel   "},
	{Msg_Firmware_Update, " Mettre � jour Firmware "},
	{Msg_Drive_Quick_Format, " Formater"},
	{Msg_Zoom, " Zoom "},
	{Msg_Zoom_In, " Zoom Avant"},
	{Msg_Zoom_Out, " Zoom Arri�re"},
	{Msg_Exit, " Sortie"},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "Fichier Vide!!"},
	{Msg_Format_Not_Support, "Fichier Non Support�!!"}
};

//German
ST_LANG_TABLE GermanTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " Sprache"},
	{Msg_ENGLISH, " Englisch  "},
	{Msg_FRENCH, " Franz�sisch  "},
	{Msg_GERMAN, " Deutsch  "},
	{Msg_ITALIAN, " Italienisch  "},
	{Msg_JAPANESE, " Japanisch  "},
	{Msg_KOREAN, " Koreanisch  "},
	{Msg_PORTUGESE, " Portugiesisch  "},
	{Msg_RUSSIAN, " Russisch  "},
	{Msg_SPANISH, " Spanisch  "},
	{Msg_SIMCHINESE, " Simplifiziertes Chinesisch     "},
	{Msg_TRACHINESE, " Traditionelles Chinesisch  "},
	{Msg_Firmware_Update, " Firmware aktualisieren "},
	{Msg_Drive_Quick_Format, " Ger�t formatieren"},
	{Msg_Zoom, " Zoom "},
	{Msg_Zoom_In, " Vergr��ern"},
	{Msg_Zoom_Out, " Verkleinern"},
	{Msg_Exit, " Beenden"},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "Dateien sind leer!!"},
	{Msg_Format_Not_Support, "Dateityp wird nicht unterst zt!"}
};

 // Italian
ST_LANG_TABLE ItalianTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " LINGUAGGIO"},
	{Msg_ENGLISH, " ENGLESE"},
	{Msg_FRENCH, " FRANCESE"},
	{Msg_GERMAN, " TEDESCO"},
	{Msg_ITALIAN, " ITALIANO"},
	{Msg_JAPANESE, " GIAPPONESE"},
	{Msg_KOREAN, " KOREANO"},
	{Msg_PORTUGESE, " PORTOGHESE"},
	{Msg_RUSSIAN, " RUSSO"},
	{Msg_SPANISH, " SPAGNOLO"},
	{Msg_SIMCHINESE, " CINESE SEMPLIFICATO "},
	{Msg_TRACHINESE, " CINESE TRADIZIONALE   "},
	{Msg_Firmware_Update, " AGGIORNAMENTO FIRMWARE "},
	{Msg_Drive_Quick_Format, " FORMATTAZIONE APPARECCHIO"},
	{Msg_Zoom, " Zoom"},
	{Msg_Zoom_In, " INGRANDIRE"},
	{Msg_Zoom_Out, " DIMINUIRE"},
	{Msg_Exit, " USCIRE"},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "Files vuoti!!"},
	{Msg_Format_Not_Support, "File non supportato!"}
};

// Japan
ST_LANG_TABLE JapanTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " ����"},
	{Msg_ENGLISH, " �p��"},
	{Msg_FRENCH, " �t�����X��"},
	{Msg_GERMAN, " �h�C�c��"},
	{Msg_ITALIAN, " �C�^���A��"},
	{Msg_JAPANESE, " ���{��"},
	{Msg_KOREAN, " �؍���"},
	{Msg_PORTUGESE, " �|���g�K����"},
	{Msg_RUSSIAN, " ���V�A��"},
	{Msg_SPANISH, " �X�y�C����"},
	{Msg_SIMCHINESE, " �ȑ̎������� "},
	{Msg_TRACHINESE, " �ɑ̎�������   "},
	{Msg_Firmware_Update, " �A�b�v�f�[�g�t�@�[���E�F�A"},
	{Msg_Drive_Quick_Format, " �t�H�[�}�b�g�f�o�C�X"},
	{Msg_Zoom, " �Y�[�� "},
	{Msg_Zoom_In, " Zoom In	  "},
	{Msg_Zoom_Out, " Zoom Out     "},
	{Msg_Exit, " EXIT    "},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "�t�@�C��������܂���!"},
	{Msg_Format_Not_Support, "�t�@�C�����T�|�[�g����Ă��܂���!"}
};

// Korean
ST_LANG_TABLE KoreanTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " ���"},
	{Msg_ENGLISH, " ���� "},
	{Msg_FRENCH, " ������ "},
	{Msg_GERMAN, " ���� "},
	{Msg_ITALIAN, " ��Ż���� "},
	{Msg_JAPANESE, " �Ϻ� "},
	{Msg_KOREAN, " �ѱ� "},
	{Msg_PORTUGESE, " ������Į "},
	{Msg_RUSSIAN, " ���þ� "},
	{Msg_SPANISH, " ������ "},
	{Msg_SIMCHINESE, " �߱���(��ü)   "},
	{Msg_TRACHINESE, " �߱���(��ü)   "},
	{Msg_Firmware_Update, " �߿��� ������Ʈ "},
	{Msg_Drive_Quick_Format, " ���� ��ġ"},
	{Msg_Zoom, " ���ϱ�   "},
	{Msg_Zoom_In, " Zoom In	  "},
	{Msg_Zoom_Out, " Zoom Out     "},
	{Msg_Exit, " EXIT    "},
	{Msg_Not_Enough_Space, " Not enough space !"},
	{Msg_No_Medium_Exist, "�� ��  �� �� !!"},
	{Msg_Format_Not_Support, "���� ������ �ȵ�!!"}
};

// Portugese
ST_LANG_TABLE PortugeseTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " L�ngua"},
	{Msg_ENGLISH, " Ingl�s"},
	{Msg_FRENCH, " Franc�s"},
	{Msg_GERMAN, " Alem�o"},
	{Msg_ITALIAN, " Italiano"},
	{Msg_JAPANESE, " Japon�s"},
	{Msg_KOREAN, " Koreano"},
	{Msg_PORTUGESE, " Portugu�s"},
	{Msg_RUSSIAN, " Russo"},
	{Msg_SPANISH, " Espanhol"},
	{Msg_SIMCHINESE, " Chin�s Simplificado   "},
	{Msg_TRACHINESE, " Chin�s Tradicional "},
	{Msg_Firmware_Update, " Atualiza��o do Firmware "},
	{Msg_Drive_Quick_Format, " Formato do Dispositivo"},
	{Msg_Zoom, " Zoom"},
	{Msg_Zoom_In, " Aumentar"},
	{Msg_Zoom_Out, " Diminuir"},
	{Msg_Exit, " Sair"},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "Ficheiros Vazios!!"},
	{Msg_Format_Not_Support, "Ficheiros n�o     Suportados!"}
};

// Russian
ST_LANG_TABLE RussianTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " ����� �����"},
	{Msg_ENGLISH, " ����������"},
	{Msg_FRENCH, " �����������"},
	{Msg_GERMAN, " ��������"},
	{Msg_ITALIAN, " �����������"},
	{Msg_JAPANESE, " ��������"},
	{Msg_KOREAN, " ���������"},
	{Msg_PORTUGESE, " �������������"},
	{Msg_RUSSIAN, " �������  "},
	{Msg_SPANISH, " ��������� "},
	{Msg_SIMCHINESE, " ���������� ���������"},
	{Msg_TRACHINESE, " ������������ ���������    "},
	{Msg_Firmware_Update, " ���������� ��"},
	{Msg_Drive_Quick_Format, " ��������������"},
	{Msg_Zoom, " �������   "},
	{Msg_Zoom_In, " Zoom In	  "},
	{Msg_Zoom_Out, " Zoom Out     "},
	{Msg_Exit, " EXIT    "},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "������ �����!!"},
	{Msg_Format_Not_Support, "���� ��           ������������!"}
};

// Spanish
ST_LANG_TABLE SpanishTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " Idioma"},
	{Msg_ENGLISH, " Ingles"},
	{Msg_FRENCH, " Franc�s"},
	{Msg_GERMAN, " Alem�n"},
	{Msg_ITALIAN, " Italiano"},
	{Msg_JAPANESE, " Japon�s"},
	{Msg_KOREAN, " Coreano"},
	{Msg_PORTUGESE, " Portuguese"},
	{Msg_RUSSIAN, " Ruso"},
	{Msg_SPANISH, " Espa�ol"},
	{Msg_SIMCHINESE, " Chino Simplificado   "},
	{Msg_TRACHINESE, " Chino Tradicional   "},
	{Msg_Firmware_Update, " Actualizaci�n de Firmware "},
	{Msg_Zoom, " Zoom  "},
	{Msg_Zoom_In, " Zoom In	  "},
	{Msg_Zoom_Out, " Zoom Out     "},
	{Msg_Exit, " EXIT    "},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "Archivos Vac�os!!"},
	{Msg_Format_Not_Support, "File Not Supported!"}
};

// SChinese
ST_LANG_TABLE SChineseTable[TotalMsg] = {
	{Msg_Ok, " Ok"},
	{Msg_Cancel, " Cancel"},
	{Msg_No, " No"},
	{Msg_Yes, " Yes"},
	{Msg_Card_Backup, " Data Backup "},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " �� �� ѡ ��"},
	{Msg_ENGLISH, " Ӣ�� "},
	{Msg_FRENCH, " ���� "},
	{Msg_GERMAN, " ���� "},
	{Msg_ITALIAN, " ������� "},
	{Msg_JAPANESE, " ���� "},
	{Msg_KOREAN, " ���� "},
	{Msg_PORTUGESE, " �������� "},
	{Msg_RUSSIAN, " ���� "},
	{Msg_SPANISH, " �������� "},
	{Msg_SIMCHINESE, " ��������  "},
	{Msg_TRACHINESE, " ��������  "},
	{Msg_Firmware_Update, " �� �� �� ��"},
	{Msg_Drive_Quick_Format, " �� ʽ ��"},
	{Msg_Zoom, " ���� "},
	{Msg_Zoom_In, " Zoom In	  "},
	{Msg_Zoom_Out, " Zoom Out     "},
	{Msg_Exit, " EXIT    "},
	{Msg_Not_Enough_Space, "Not enough space !"},
	{Msg_No_Medium_Exist, "û �� �� �� !!"},
	{Msg_Format_Not_Support, "�� ʽ �� ֧ ��!!"}
};

// �c�餤��
ST_LANG_TABLE TChineseTable[TotalMsg] = {
	{Msg_Ok, " �n"},
	{Msg_Cancel, " ����"},
	{Msg_No, " �_"},
	{Msg_Yes, " �O"},
	{Msg_Card_Backup, " ��Ƴƥ�"},
	{Msg_Empty, ""},
	{Msg_Language_Selection, " �y �� �� ��"},
	{Msg_ENGLISH, " �^�y"},
	{Msg_FRENCH, " �k�y"},
	{Msg_GERMAN, " �w�y"},
	{Msg_ITALIAN, " �q�j�Q�y"},
	{Msg_JAPANESE, " ��y"},
	{Msg_KOREAN, " ���y"},
	{Msg_PORTUGESE, " ������y"},
	{Msg_RUSSIAN, " �X�y"},
	{Msg_SPANISH, " ��Z���y"},
	{Msg_SIMCHINESE, " ²�餤��"},
	{Msg_TRACHINESE, " �c�餤��"},
	{Msg_Firmware_Update, " �� �s �� ��"},
	{Msg_Drive_Quick_Format, " �� �� ��"},
	{Msg_Zoom, " �Y �� "},
	{Msg_Zoom_In, " �� �j "},
	{Msg_Zoom_Out, " �Y �p "},
	{Msg_Exit, " �� �} "},
	{Msg_Not_Enough_Space, "�� �� �� �� !!"},
	{Msg_No_Medium_Exist, "�S �� �� �� !!"},
	{Msg_Format_Not_Support, "�� �� �� �� ��!!"}
};



ST_LANG_TABLE *Lang_GetCurrentTable()
{
	switch (bCurrentLanguage)
	{
	case 0:
		// English
		return EnglishTable;
	case 1:
		// French
		return FrenchTable;
	case 2:
		// German
		return GermanTable;
	case 3:
		// Italian
		return ItalianTable;
	case 4:
		// Japan
		return JapanTable;
	case 5:
		// Korean
		return KoreanTable;
	case 6:
		// Portugese
		return PortugeseTable;
	case 7:
		// Russian
		return RussianTable;
	case 8:
		// Spanish
		return SpanishTable;
	case 9:
		// SChinese
		return SChineseTable;
	case 10:
		// TChinese
		return TChineseTable;
	}

}


BYTE *Lang_GetText(ST_LANG_TABLE * table, DWORD key)
{
	BOOL i;

	for (i = 0; i < TotalMsg; i++)
		if (table[i].ID == key)
			return (BYTE *) (table[i].Text);

	return " --- ";
}




void Lang_Init()
{
	sLang.pbFontAddr = (BYTE *) GetResource(0 /*RES_FONT */ );
	sLang.bFontImgColor = 0;	//IMG_FONT_COLOR;
	sLang.bFontOsdColor = 0;	//OSD_FONT_COLOR;
	sLang.bFontLanguage = English;
	sLang.bTextGap = 0;			//TEXT_GAP;

}


void Lang_SetGap(BYTE BGap)
{
	sLang.bTextGap = BGap;
}


void Lang_SetImgColor(BYTE bColor)
{
	sLang.bFontImgColor = bColor;
}


void Lang_SetOsdColor(BYTE bColor)
{
	sLang.bFontOsdColor = bColor;
}


void Lang_SetbFontLanguage(BYTE blang)
{
	sLang.bFontLanguage = blang;
}


BYTE Lang_GetGap()
{
	return sLang.bTextGap;
}


BYTE Lang_GetImgColor()
{
	return sLang.bFontImgColor;
}


BYTE Lang_GetOsdColor()
{
	return sLang.bFontOsdColor;
}


BYTE Lang_GetFontLanguage()
{
	return sLang.bFontLanguage;
}



WORD *Lang_GetFstPixel(WORD code)
{
	DWORD location;
	DWORD *source;

	//source = 0xbfc10000;
	source = (DWORD *) START;

	//while source != "MRDB"
	while (*source != 0x4d524442)
	{
		if ((DWORD) source > 0xbfffffff)
			return 0;
		source += 0x2000;
	}

	//search for the first bit corresponding to 'code'
	location = Lang_GetLocation(code);

	//empty space, character not found
	if (location == 0x1600)
	{
		return (WORD *) source;
	}

	//skip the 'unwanted' part and return the first 'wanted' pixel
	source += (location / 4);

	return (WORD *) source;
}


DWORD Lang_GetLocation(WORD code)
{
	DWORD total = 0;
	DWORD temp = 0xA17E;
	DWORD offset, unwanted;

	//default value is empty space
	unwanted = 0x1600;

	//ASCII code (ENGLISH)
	if (code > 0x1f && code < 0x80)
	{
		unwanted = (code - 0x20) * 0x10 + 0x28;
		return unwanted;
	}
	//European Languages
	else if (code > 0x9f && code < 0x100 && bCurrentLanguage != Russian)
	{
		unwanted = (code - 0x40) * 0x10 + 0x28;
	}
	else if (code > 0x9f && code < 0x100 && bCurrentLanguage == Russian)
	{
		unwanted = (code - 0xa1) * 0x10 + 0x76ba4;
	}
	else if (bCurrentLanguage == Japanese)
	{
		if (code > 0x813f)
		{
			if (code > 0x817e)
				total += 0x01;
			if (code > 0x81ac)
				total += 0x0b;
			if (code > 0x81bf)
				total += 0x08;
			if (code > 0x81ce)
				total += 0x0b;
			if (code > 0x81e8)
				total += 0x07;
			if (code > 0x81f7)
				total += 0x04;
			if (code > 0x81fc)
				total += 0x52;
			if (code > 0x8258)
				total += 0x07;
			if (code > 0x8279)
				total += 0x07;
			if (code > 0x829a)
				total += 0x04;
			if (code > 0x82f1)
				total += 0x4e;
			if (code > 0x837e)
				total += 0x01;
			if (code > 0x8396)
				total += 0x08;
			if (code > 0x83b6)
				total += 0x08;
			if (code > 0x83d6)
				total += 0x69;
			if (code > 0x8460)
				total += 0x0f;
			if (code > 0x847e)
				total += 0x01;
			if (code > 0x8491)
				total += 0x0d;

			if (code > 0x84bf)
			{
				total += 0x3e0;
				offset = 0x88fc;

				while (code > offset)
				{
					if (code > offset)
						total += 0x43;
					offset += 0x83;
					if (code > offset)
						total += 0x01;
					offset += 0x7d;

					if (offset == 0x97fc)
					{
						if (code > offset)
						{
							total += 0x43;
							offset = 0x9872;
						}
						if (code > offset)
						{
							total += 0x2c;
							offset = 0x98fc;
						}
					}

					if (offset == 0x9ffc)
					{
						if (code > 0x9ffc)
							total += 0x4043;
						if (code > 0xe07f)
							total += 0x01;
						offset = 0xe0fc;
					}
				}
			}
			unwanted = (code - total - 0x8140) * 0x20 + 0x771ac;
		}
	}
	else if (bCurrentLanguage == Korean)
	{
		if (code == 0xA2A6)
			code = 0xa1ad;
		if (code == 0x89C0)
			code = 0xb5c6;
		if (code > 0xa1fe)
			total += 0x00a2;
		if (code > 0xa2e5)
			total += 0x00bb;
		if (code > 0xa3fe)
			total += 0x00a2;
		if (code > 0xa4fe)
			total += 0x00a2;
		if (code > 0xa5aa)
			total += 0x0005;
		if (code > 0xa5b9)
			total += 0x0007;
		if (code > 0xa5d8)
			total += 0x0008;
		if (code > 0xa5f8)
			total += 0x00a8;
		if (code > 0xa6e4)
			total += 0x00bc;
		if (code > 0xa7ef)
			total += 0x00b1;
		if (code > 0xa8a4)
			total += 0x0001;
		if (code > 0xa8a6)
			total += 0x0001;
		if (code > 0xa8af)
			total += 0x0001;
		if (code > 0xa8fe)
			total += 0x00a2;
		if (code > 0xa9fe)
			total += 0x00a2;
		if (code > 0xaaf3)
			total += 0x00ad;
		if (code > 0xabf6)
			total += 0x00aa;
		if (code > 0xacc1)
			total += 0x000f;
		if (code > 0xacf1)
			total += 0x03af;

		offset = 0xb0fe;
		while (code > offset)
		{
			total += 0xa2;
			offset += 0x100;
			if (offset == 0xc8fe)
			{
				if (code > offset)
					total += 0x1a2;
				offset = 0xcafe;
			}
		}
		unwanted = (code - total - 0xa1a1) * 0x20 + 0xb38f4;
	}
	else if (bCurrentLanguage == SChinese)
	{
		if (code > 0xa1fe)
			total += 0x00b2;
		if (code > 0xa2e2)
			total += 0x0002;
		if (code > 0xa2ee)
			total += 0x0002;
		if (code > 0xa2fc)
			total += 0x00a4;
		if (code > 0xa3fe)
			total += 0x00a2;
		if (code > 0xa4f3)
			total += 0x00ad;
		if (code > 0xa5f6)
			total += 0x00aa;
		if (code > 0xa6b8)
			total += 0x0008;
		if (code > 0xa6d8)
			total += 0x00c8;
		if (code > 0xa7c1)
			total += 0x000f;
		if (code > 0xa7f1)
			total += 0x00af;
		if (code > 0xa8ba)
			total += 0x000a;
		if (code > 0xa8e9)
			total += 0x00ba;
		if (code > 0xa9ef)
			total += 0x06b1;

		offset = 0xb0fe;
		while (code > offset && offset <= 0xf6fe)
		{
			if (code > offset)
				total += 0x00a2;
			if (offset == 0xd6fe)
			{
				if (code > 0xd7f9)
					total += 0xa7;
				offset = 0xd8fe;
			}
			else
				offset += 0x100;
		}
		unwanted = (code - total - 0xa1a1) * 0x20 + 0xfbda4;
	}
	else if (code > 0xa13f && code < 0xf9d6 && bCurrentLanguage == TChinese)
	{
		offset = 0xa140;

		while (code > temp)
		{
			if (temp == 0xc67e)
			{
				total += 0x2c1;
				temp = 0xc97e;
			}
			else
			{
				//the way that font table is
				if (code > temp)
					total += 0x22;
				temp += 0x80;

				if (temp > 0xa37e && temp < 0xa47e)
				{
					total += 0x80;
					temp = 0xa47e;
				}
				else
				{
					if (code > temp)
						total += 0x41;
					temp += 0x80;
				}
			}
		}

		code -= total;
		unwanted = (code - offset) * 0x20;
		unwanted += 0xc40;
	}

	return unwanted;
}
