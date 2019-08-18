#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream.h>
#include <string.h>
#include <iostream.h>

//#define MANUAL  // user keyin file name from console, if not define then AUTO mode

// Output format setting
//#define ALIGN (no-alignment :eg.1M, VGA : Width=22 or 10)
#undef ALIGN    (alignment : eg. XGA, SVGA, 2M, 3M, 5M : Width=40 or 12)

#define VGA1M // VGA0 // VGA2M // VGA3M // VGA5M // XGA // SVGA 

#define WRITE_HEADER     // WRITE HEADER
//#undef WRITE_HEADER    // Don't WRITE HEADER (only YUV format data)

#define OFFSET_START_ADDRESS_ORIGINAL
//#undef OFFSET_START_ADDRESS_ORIGINAL
#define DEBUG_SIZE 128

// Read BMP data position setting
#define BMP_OFFSET_BYTE 10 // Bitmap file header at the 10 Byte address
#define BMP_WIDTH_BYTE 18  // Bitmap info header at the  8 Byte address
#define BMP_HEIGHT_BYTE 22 // Bitmap info header at the 12 Byte address

// RGB to YUV Color Key setting
#define RED_CK 0         // BLACK is R=0, G=0 and B=0
#define GREEN_CK 0
#define BLUE_CK 0

// Input file parameter setting
#define FILE_NO 1        // Default = 15 files : its always more than 1
#define FINDEX 1         // Output index file name


#pragma hdrstop
#pragma argsused
//----------------------------- DEBUG MESSAGE ------------------------------
//#define MSN_DEBUG
#undef MSN_DEBUG
//#define MSN_DEBUG1
#undef MSN_DEBUG1
//#define MSN_DEBUG2
#undef MSN_DEBUG2
//#define MSN_DEBUG3
#undef MSN_DEBUG3
#define MSN_DEBUG4
//#undef MSN_DEBUG4
//---------------------------------------------------------------------------
typedef struct tagBitMapFileHeader {  // size  offset //
	//char            Type[2];        //  2   0   //
	//unsigned long   Size;           //  4   2   //
	//unsigned short  Reserved1;      //  2   6   //
	//unsigned short  Reserved2;      //  2   8   //
	char   Offset1;                   //  4   10  //
} BitMapFileHeader;                   // Total size: 14 //

typedef struct tagBitMapInfoHeader {  // size  offset //
	//unsigned long   Size;           //  4   0   //
	unsigned char     Width1;         //  4   4   //
	unsigned char     Height1;        //  4   8   //
	//unsigned short  Planes;         //  2   12  //
	//unsigned short  BitCount;       //  2   14  //
	//unsigned long   Compression;    //  4   16  //
	//unsigned long   SizeImage;      //  4   20  //
	//long            XPelsPerMeter;  //  4   24  //
	//long            YPelsPerMeter;  //  4   28  //
	//unsigned long   ClrUsed;        //  4   32  //
	//unsigned long   ClrImportant;   //  4   36  //
} BitMapInfoHeader;                   // Total size: 40 //

 typedef struct RGB_value {
	unsigned char R;
	unsigned char G;
	unsigned char B;
} MAP;

int main(int argc, char* argv[])
{
	BitMapFileHeader BMPf;
	BitMapInfoHeader BMPi;
	MAP **BUFFER; // Declare dynamic memory BUFFER size

	#ifdef MSN)DEBUG2
	BUFFER[1][1].R = 1; // Test write to BUFFER[1][1] value
	printf("\nBUFFER[1][1].R=%d", BUFFER[1][1].R);
	#endif
	printf("## BMPtoYUV v1.0.0.20110811 ##\n");
	// The statements of code can work successfully
	fstream file;
	FILE *fPtr, *fPtr1, *fPtr2;
	unsigned char r, g, b, r_ck, g_ck, b_ck, R0, G0, B0, R1, G1, B1; // unsigned long = DWORD
	unsigned char y0_ck, cb0_ck, cr0_ck, y0, cb0, cr0, y1, cb1, cr1; // unsigned long = DWORD
	char *ptr, *str, str_emp[1]={NULL}, write_header[1]={NULL}; // If variable str[100000] can cover at least 15 files that include 1760 bytes each file
	unsigned char Width, Height;  // Bitmap width and height
	int j = 0, i = 0, offset = 0;
	unsigned int onlyone = 0;
	bool RGB_FP = true;
	char ifilename[100] = {NULL}, ofilename[100] = {NULL}; // The ifilename&ofilename can receive at most 100 characters for file name
	int WIDTH_i = 0, HEIGHT_i = 0, RGB_cul = 0, RGB_i = 0;
	int WIDTH_tmp, j2 = 0, i2 = 0, align = 0, file_no[1], temp_w, Len, temp_h;
	int Width_hb; // High bit to Width

	// Init settings
	write_header[0] = 'y';
	printf(" - The header content will be writen into a YUV file\n");
	sprintf(ofilename,"FONT.yuv");
	printf(" - Output file is FONT.yuv.\n");
Wait_user_type:
	printf("*** User Type List\n");
	printf("0). Manual input\n");
	printf("1). 1M3\n");
	printf("2). 2M\n");
	printf("3). 3M\n");
	printf("4). 5M\n");
	printf("5). VGA\n");
	printf("6). XGA\n");
	printf("7). SVGA\n");
	printf("Please select:");
	char ch = getchar();
	int user_type = 0;
	if(ch >= 0x30 && ch <= 0x37)
	{
		user_type = ch - '0';
		printf("user_type = %d\n", user_type);
	}
	else
	{
		printf("Invalid choice!, must betwwen 0..7\n");
		goto Wait_user_type;
	}
	if(user_type > 0)
	{
		file_no[0] = 15;
		printf(" - There are 15 input files.\n");
	}
	else // manual
	{
    	printf("\nPlease type a number of input files: ");
		scanf("%d", file_no);
    }

	fPtr = fopen(ofilename, "wb+");
	if (!fPtr)
	{
		printf("open file fail!!...\n");
		exit(1);
	}
	fclose(fPtr);
	fPtr1 = fopen(ofilename, "ab+");
	if (!fPtr1)
	{
		printf("open file fail!!...\n");
		exit(1);
	}

	// Each file do BMPtoYUV convert
	for (j=0;j<file_no[0];j++)
	{
		offset = 0;
		RGB_FP = true;
		// Input BMP file name
		if(user_type == 0)
		{
			printf("\nPlease type input 32 bitmap file name: ");
			scanf("%s",ifilename);
		}
		else if(user_type == 1)
		{
			if(j < 10)
				sprintf(ifilename,"font_1M3_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_1M3_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_1M3_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_1M3_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_1M3_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_1M3_t.bmp");
		}
		else if(user_type == 2)
		{
			if(j < 10)
				sprintf(ifilename,"font_2M_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_2M_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_2M_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_2M_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_2M_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_2M_t.bmp");
		}
		else if(user_type == 3)
		{
			if(j < 10)
				sprintf(ifilename,"font_2M_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_2M_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_2M_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_2M_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_2M_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_2M_t.bmp");
		}
		else if(user_type == 3)
		{
			if(j < 10)
				sprintf(ifilename,"font_3M_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_3M_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_3M_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_3M_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_3M_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_3M_t.bmp");
		}
		else if(user_type == 4)
		{
			if(j < 10)
				sprintf(ifilename,"font_5M_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_5M_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_5M_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_5M_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_5M_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_5M_t.bmp");
		}
		else if(user_type == 5)
		{
			if(j < 10)
				sprintf(ifilename,"font_VGA_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_VGA_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_VGA_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_VGA_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_VGA_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_VGA_t.bmp");
			else if(j == 99)
				sprintf(ifilename,"font_VGA_t.bmp");
		}
		else if(user_type == 6)
		{
			if(j < 10)
				sprintf(ifilename,"font_XGA_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_XGA_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_XGA_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_XGA_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_XGA_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_XGA_t.bmp");
		}
		else if(user_type == 7)
		{
			if(j < 10)
				sprintf(ifilename,"font_SVGA_%d.bmp", j);
			else if(j == 10)
				sprintf(ifilename,"font_SVGA_a.bmp");
			else if(j == 11)
				sprintf(ifilename,"font_SVGA_d.bmp");
			else if(j == 12)
				sprintf(ifilename,"font_SVGA_m.bmp");
			else if(j == 13)
				sprintf(ifilename,"font_SVGA_p.bmp");
			else if(j == 14)
				sprintf(ifilename,"font_SVGA_t.bmp");
		}

		printf("ifilename=%s\n", ifilename);
		fPtr = fopen(ifilename, "rb+");
		if (!fPtr)
		{
			printf("open file fail! file maybe not exist!\n");
			printf("Any key exit!\n");
			getchar();
			getchar();
			exit(1);
		}
		fseek (fPtr, 0, SEEK_END);
		Len = ftell(fPtr);    // Len = File size
		printf("BMP %d file size = %d Byte\n", j, Len);
		fclose(fPtr);

		// Memory free
		if(j > 0)
			free(str);

//##############################################################################
if(j==0)
{
	str = (char *) malloc(Len*sizeof(char));
	memset(str, 0, sizeof(str));

	// Read BMP file
	file.open(ifilename, ios::in|ios::binary);
	file.read(str, Len);
	file.close();
	// Initialize parameter values
	RGB_cul = 0;
	RGB_i = 0;
	RGB_FP =true;

	for(i=0;i<Len;i++)
	{
		if(i == BMP_OFFSET_BYTE)
		{ // "BMP DATA OFFSET" = The 10 byte
			//printf("i == BMP_OFFSET_BYTE = %d\n", i);
			offset = str[BMP_OFFSET_BYTE];
			#ifdef OFFSET_START_ADDRESS_ORIGINAL
			offset = offset;        // Data offset from start address
			#else
			offset = i + offset;    // Data offset from current address
			#endif
			BMPf.Offset1 = offset;
			#ifdef MSN_DEBUG4
				//printf("BMPf.Offset1=%d\n", BMPf.Offset1);
			#endif
		}
		else if(i == BMP_WIDTH_BYTE)
		{  // "BMP Width" = The 18 byte
			//printf("i == BMP_WIDTH_BYTE = %d\n", i);
			Width = str[BMP_WIDTH_BYTE]; // This line can't delete
			BMPi.Width1 = str[BMP_WIDTH_BYTE];
			align = Width % 4; // Adjudge to whether or not memory alignment
			temp_w = Width;
			#ifdef MSN_DEBUG4
				//printf("align=%d\n", align);
			#endif
			#ifdef MSN_DEBUG4
				//printf("Width=%d, BMPi.width1=%d\n", Width, BMPi.Width1);
			#endif
		}
		else if(i == (BMP_WIDTH_BYTE + 1) )
		{
			//printf("i == (BMP_WIDTH_BYTE + 1) = %d\n", i);
			if(str[i] != 0)
			{
				Width_hb = str[i]; // Read high bit data
				#ifdef MSN_DEBUG4
					//printf("before Width_hb=%d\n", Width_hb);
				#endif
				Width_hb = Width_hb * 256;
				#ifdef MSN_DEBUG4
					//printf("before_1 Width_hb=%d\n", Width_hb);
				#endif
				temp_w = Width + Width_hb;
				Width = Width + Width_hb;
				#ifdef MSN_DEBUG4
					//printf("Real Width=%d\n", temp_w);
				#endif
			} // if(str[i] != 0)
		}
		else if(i == BMP_HEIGHT_BYTE)
		{    // "BMP Height" = The 22 byte
			//printf("i == BMP_HEIGHT_BYTE = %d\n", i);
			Height = str[BMP_HEIGHT_BYTE];
			BMPi.Height1 = str[BMP_HEIGHT_BYTE];
			HEIGHT_i = Height-1; // HEIGHT_i = 40 (0~39)
			WIDTH_i = 0;         // WIDTH_i = 24 (0~23) including 2 reserved bytes
			#ifdef MSN_DEBUG4
				//printf("Real Height=%d\n", BMPi.Height1);
			#endif
			// Dynamic allocate memory BUFFER size
			//printf("Dynamic memory allication\n");
			temp_h = Height;

			if(j>0)
			{
				for(int it=0; it<temp_w; it++)
					delete [] BUFFER[it];
				delete [] BUFFER;
			}

			int even = temp_w % 4; // divided by 4 => even(mod) is only 0 or 2
			// Handle memory alignment to first index i
			if(even != 0){
				BUFFER = new MAP*[temp_w + 2];
				for(int iii=0; iii<(temp_w + 2); iii++)
				{
					BUFFER[iii]= new MAP[temp_h];
				}
			}
			else
			{
				BUFFER = new MAP*[temp_w];
				for(int iii=0; iii<temp_w; iii++)
				{
					BUFFER[iii]= new MAP[temp_h];
				}
			}
		}  //if(i == BMP_HEIGHT_BYTE
		// Offset value must be subtracted by one for obtaining correct data
		else
		if( i >= (BMPf.Offset1-1) )
		{ // Get "BMP DATA" from offset position
			//printf("i >= (BMPf.Offset1-1) = %d\n", i); // too maly log, until 2735
			 //offset = 54(0x36h)
			 RGB_cul ++;
			 RGB_i ++;
			 if(RGB_cul == 1 ){
			   r = str[i];
			 }
			 else if(RGB_cul == 2){
			   g = str[i];
			 }
			 else if(RGB_cul == 3){
			   b = str[i];
			   RGB_cul = 0;
			 // Save BMP data to BUFFER
			 if(RGB_i <= (temp_w *3) ){ // Pixel: WIDTH*3
			   BUFFER[WIDTH_i][HEIGHT_i].R = r;
			   BUFFER[WIDTH_i][HEIGHT_i].G = g;
			   BUFFER[WIDTH_i][HEIGHT_i].B = b;
			   // Move one pixel at one time
			   WIDTH_i ++;
			   if(WIDTH_i >= temp_w){ // Real WIDTH
				 WIDTH_i = 0;
			   }
			   if(align == 0){ // Non-alignment
				 #ifdef MSN_DEBUG2
				 //printf("RGB_i=%d\n", RGB_i);
				 #endif
				 if(RGB_i == (temp_w*3)){
				   RGB_i = 0;
				   HEIGHT_i --;
				 }
				 if(HEIGHT_i < 0){ // Stop condition
				   i = Len;
				 }
			   } //if(align == 0)
			 }
			 // if RGB_i = 69 is presented next pixel by 3 Bytes
			 // But we only fill two Bytes for avoiding memory alignment
			 // Moreover the BUFFER[24][HEIGHT_i] that don't fill value 0
			if(align != 0)
			{ // Alignment
				if(RGB_i == (temp_w*3+3))
				{ // WIDTH*3+3
					BUFFER[temp_w][HEIGHT_i].R = 0;
					BUFFER[temp_w][HEIGHT_i].G = 0;
					BUFFER[temp_w][HEIGHT_i].B = 0;
					BUFFER[temp_w+1][HEIGHT_i].R = 0;
					BUFFER[temp_w+1][HEIGHT_i].G = 0;
					BUFFER[temp_w+1][HEIGHT_i].B = 0;

					HEIGHT_i --; // Range:0~39
					if(HEIGHT_i < 0)
					{ // Stop condition
						i = Len;
					}
					RGB_i = 0;
					i = i-1;          // Data consistent to last loop
				}
			} //if(align != 0)
		 } // if( i >= (BMPf.Offset1-1) )
	   } // else if(i == BMP_OFFSET_BYTE)
	} // for(i=0;i<Len;i++)
	// Filter all BUFFER[0][Height].R = 0 for VGA BMP case
	for(int jj = 0; jj < Height ; jj++)
	{
		BUFFER[0][jj].R = 0;
	}
	// Stop condition & start save Buffer data to YUV file
	// Define "Color Key" for transforming BMP to YUV
	r_ck = RED_CK;  // RGB Black is R=0, G=0 and B=0
	g_ck = GREEN_CK;// YUV Black is Y0=0x00,Y1=0x00,Cb0=0x80,Cr0=0x80
	b_ck = BLUE_CK;
	// RGB888 to YUV422 for Color Key - test ok
	y0_ck  = (( 77 * r_ck + 150 * g_ck +  29 * b_ck) >> 8) & 0xFF;
	cb0_ck = (((-43 * r_ck -  85 * g_ck + 128 * b_ck) >> 8) + 128) & 0xFF;
	cr0_ck = (((128 * r_ck - 107 * g_ck -  21 * b_ck) >> 8) + 128) & 0xFF;

	if(write_header[0] == 'y')
	{ // The header length has 8 bytes
		//printf("\nTo start writing header content to a YUV file");
		if(j == 0)
		{ // Write Width, Height and Color Key at first
			if(onlyone == 0)
			{
				onlyone = 1;
				fwrite(&str_emp, sizeof(char), 1, fPtr1);
				fwrite(&temp_w, sizeof(char), 1, fPtr1);
				fwrite(&str_emp, sizeof(char), 1, fPtr1);
				fwrite(&BMPi.Height1, sizeof(char), 1, fPtr1);
				fwrite(&y0_ck, sizeof(char), 1, fPtr1);
				fwrite(&y0_ck, sizeof(char), 1, fPtr1);
				fwrite(&cb0_ck, sizeof(char), 1, fPtr1);
				fwrite(&cr0_ck, sizeof(char), 1, fPtr1);
			}
		}
	}
} // if(j==0)
//##############################################################################

//******************************************************************************
		str = (char *) malloc(Len*sizeof(char));
	memset(str, 0, sizeof(str));

	// Read BMP file
	file.open(ifilename, ios::in|ios::binary);
	file.read(str, Len);
	file.close();
	// Initialize parameter values
	RGB_cul = 0;
	RGB_i = 0;
	RGB_FP =true;

	for(i=0;i<Len;i++)
	{
		if(i == BMP_OFFSET_BYTE)
		{ // "BMP DATA OFFSET" = The 10 byte
			//printf("i == BMP_OFFSET_BYTE = %d\n", i);
			offset = str[BMP_OFFSET_BYTE];
			#ifdef OFFSET_START_ADDRESS_ORIGINAL
			offset = offset;        // Data offset from start address
			#else
			offset = i + offset;    // Data offset from current address
			#endif
			BMPf.Offset1 = offset;
			#ifdef MSN_DEBUG4
				//printf("BMPf.Offset1=%d\n", BMPf.Offset1);
			#endif
		}
		else if(i == BMP_WIDTH_BYTE)
		{  // "BMP Width" = The 18 byte
			//printf("i == BMP_WIDTH_BYTE = %d\n", i);
			Width = str[BMP_WIDTH_BYTE]; // This line can't delete
			BMPi.Width1 = str[BMP_WIDTH_BYTE];
			align = Width % 4; // Adjudge to whether or not memory alignment
			temp_w = Width;
			#ifdef MSN_DEBUG4
				//printf("align=%d\n", align);
			#endif
			printf("Width=%d, BMPi.width1=%d\n", Width, BMPi.Width1);

		}
		else if(i == (BMP_WIDTH_BYTE + 1) )
		{
			//printf("i == (BMP_WIDTH_BYTE + 1) = %d\n", i);
			if(str[i] != 0)
			{
				Width_hb = str[i]; // Read high bit data
				#ifdef MSN_DEBUG4
					//printf("before Width_hb=%d\n", Width_hb);
				#endif
				Width_hb = Width_hb * 256;
				#ifdef MSN_DEBUG4
					//printf("before_1 Width_hb=%d\n", Width_hb);
				#endif
				temp_w = Width + Width_hb;
				Width = Width + Width_hb;
				printf("Real Width=%d\n", temp_w);

			} // if(str[i] != 0)
		}
		else if(i == BMP_HEIGHT_BYTE)
		{    // "BMP Height" = The 22 byte
			//printf("i == BMP_HEIGHT_BYTE = %d\n", i);
			Height = str[BMP_HEIGHT_BYTE];
			BMPi.Height1 = str[BMP_HEIGHT_BYTE];
			HEIGHT_i = Height-1; // HEIGHT_i = 40 (0~39)
			WIDTH_i = 0;         // WIDTH_i = 24 (0~23) including 2 reserved bytes
			printf("Real Height=%d\n", BMPi.Height1);

			// Dynamic allocate memory BUFFER size
			//printf("Dynamic memory allication\n");
			temp_h = Height;

			if(j>0)
			{
				for(int it=0; it<temp_w; it++)
					delete [] BUFFER[it];
				delete [] BUFFER;
			}

			int even = temp_w % 4; // divided by 4 => even(mod) is only 0 or 2
			// Handle memory alignment to first index i
			if(even != 0){
				BUFFER = new MAP*[temp_w + 2];
				for(int iii=0; iii<(temp_w + 2); iii++)
				{
					BUFFER[iii]= new MAP[temp_h];
				}
			}
			else
			{
				BUFFER = new MAP*[temp_w];
				for(int iii=0; iii<temp_w; iii++)
				{
					BUFFER[iii]= new MAP[temp_h];
				}
			}
		}  //if(i == BMP_HEIGHT_BYTE
		// Offset value must be subtracted by one for obtaining correct data
		else
		if( i >= (BMPf.Offset1-1) )
		{ // Get "BMP DATA" from offset position
			//printf("i >= (BMPf.Offset1-1) = %d\n", i); // too maly log, until 2735
			 //offset = 54(0x36h)
			 RGB_cul ++;
			 RGB_i ++;
			 if(RGB_cul == 1 ){
			   r = str[i];
			 }
			 else if(RGB_cul == 2){
			   g = str[i];
			 }
			 else if(RGB_cul == 3){
			   b = str[i];
			   RGB_cul = 0;
			 // Save BMP data to BUFFER
			 if(RGB_i <= (temp_w *3) ){ // Pixel: WIDTH*3
			   BUFFER[WIDTH_i][HEIGHT_i].R = r;
			   BUFFER[WIDTH_i][HEIGHT_i].G = g;
			   BUFFER[WIDTH_i][HEIGHT_i].B = b;
			   // Move one pixel at one time
			   WIDTH_i ++;
			   if(WIDTH_i >= temp_w){ // Real WIDTH
				 WIDTH_i = 0;
			   }
			   if(align == 0){ // Non-alignment
				 #ifdef MSN_DEBUG2
				 //printf("RGB_i=%d\n", RGB_i);
				 #endif
				 if(RGB_i == (temp_w*3)){
				   RGB_i = 0;
				   HEIGHT_i --;
				 }
				 if(HEIGHT_i < 0){ // Stop condition
				   i = Len;
				 }
			   } //if(align == 0)
			 }
			 // if RGB_i = 69 is presented next pixel by 3 Bytes
			 // But we only fill two Bytes for avoiding memory alignment
			 // Moreover the BUFFER[24][HEIGHT_i] that don't fill value 0
			if(align != 0)
			{ // Alignment
				if(RGB_i == (temp_w*3+3))
				{ // WIDTH*3+3
					BUFFER[temp_w][HEIGHT_i].R = 0;
					BUFFER[temp_w][HEIGHT_i].G = 0;
					BUFFER[temp_w][HEIGHT_i].B = 0;
					BUFFER[temp_w+1][HEIGHT_i].R = 0;
					BUFFER[temp_w+1][HEIGHT_i].G = 0;
					BUFFER[temp_w+1][HEIGHT_i].B = 0;

					HEIGHT_i --; // Range:0~39
					if(HEIGHT_i < 0)
					{ // Stop condition
						i = Len;
					}
					RGB_i = 0;
					i = i-1;          // Data consistent to last loop
				}
			} //if(align != 0)
		 } // if( i >= (BMPf.Offset1-1) )
	   } // else if(i == BMP_OFFSET_BYTE)
	} // for(i=0;i<Len;i++)
	// Filter all BUFFER[0][Height].R = 0 for VGA BMP case
	for(int jj = 0; jj < Height ; jj++)
	{
		BUFFER[0][jj].R = 0;
	}
	// Stop condition & start save Buffer data to YUV file
	// Define "Color Key" for transforming BMP to YUV
	r_ck = RED_CK;  // RGB Black is R=0, G=0 and B=0
	g_ck = GREEN_CK;// YUV Black is Y0=0x00,Y1=0x00,Cb0=0x80,Cr0=0x80
	b_ck = BLUE_CK;
	// RGB888 to YUV422 for Color Key - test ok
	y0_ck  = (( 77 * r_ck + 150 * g_ck +  29 * b_ck) >> 8) & 0xFF;
	cb0_ck = (((-43 * r_ck -  85 * g_ck + 128 * b_ck) >> 8) + 128) & 0xFF;
	cr0_ck = (((128 * r_ck - 107 * g_ck -  21 * b_ck) >> 8) + 128) & 0xFF;

	if(write_header[0] == 'y')
	{ // The header length has 8 bytes
		//printf("\nTo start writing header content to a YUV file");
		if(j == 0)
		{ // Write Width, Height and Color Key at first
			if(onlyone == 0)
			{
				onlyone = 1;
				fwrite(&str_emp, sizeof(char), 1, fPtr1);
				fwrite(&temp_w, sizeof(char), 1, fPtr1);
				fwrite(&str_emp, sizeof(char), 1, fPtr1);
				fwrite(&BMPi.Height1, sizeof(char), 1, fPtr1);
				fwrite(&y0_ck, sizeof(char), 1, fPtr1);
				fwrite(&y0_ck, sizeof(char), 1, fPtr1);
				fwrite(&cb0_ck, sizeof(char), 1, fPtr1);
				fwrite(&cr0_ck, sizeof(char), 1, fPtr1);
			}
		}
	}

//******************************************************************************


	for(j2 = 0; j2 < Height ; j2++)
	{
		for(i2 = 0; i2 < temp_w ; i2++)
		{
			//printf("\nWidth=%d,Height=%d",i2,j2);
			// RGB888 to YUV422 for first point
			if(RGB_FP == true)
			{
				#ifdef MSN_DEBUG3
					printf("\noffset=%d, RGB_FP=%d", i2, RGB_FP);
				#endif
				#ifdef MSN_DEBUG2
					printf("\nRGB0:i2=%d, j2=%d", i2, j2);
				#endif
				R0 = BUFFER[i2][j2].R;
				G0 = BUFFER[i2][j2].G;
				B0 = BUFFER[i2][j2].B;
				y0  = (( 77 * BUFFER[i2][j2].R + 150 * BUFFER[i2][j2].G +  29 * BUFFER[i2][j2].B) >> 8) & 0xFF;
				cb0 = (((-43 * BUFFER[i2][j2].R -  85 * BUFFER[i2][j2].G + 128 * BUFFER[i2][j2].B) >> 8) + 128) & 0xFF;
				cr0 = (((128 * BUFFER[i2][j2].R - 107 * BUFFER[i2][j2].G -  21 * BUFFER[i2][j2].B) >> 8) + 128) & 0xFF;
				RGB_FP = false;

			}
			else
			{ // RGB888 to YUV422 for second point
				#ifdef MSN_DEBUG2
					printf("\noffset=%d, RGB_FP=%d", i2, RGB_FP);
				#endif
				#ifdef MSN_DEBUG2
					printf("\nRGB1:i2=%d, j2=%d", i2, j2);
				#endif
				R1 = BUFFER[i2][j2].R;
				G1 = BUFFER[i2][j2].G;
				B1 = BUFFER[i2][j2].B;
				y1  = (( 77 * BUFFER[i2][j2].R + 150 * BUFFER[i2][j2].G +  29 * BUFFER[i2][j2].B) >> 8) & 0xFF;
				cb1 = (((-43 * BUFFER[i2][j2].R -  85 * BUFFER[i2][j2].G + 128 * BUFFER[i2][j2].B) >> 8) + 128) & 0xFF;
				cr1 = (((128 * BUFFER[i2][j2].R - 107 * BUFFER[i2][j2].G -  21 * BUFFER[i2][j2].B) >> 8) + 128) & 0xFF;

				// Boundary conditions
				// if ((rgb0 == colorKey) && (rgb1 != colorKey))
				if( ( (R0 == RED_CK) && (G0 == GREEN_CK) && (B0 == BLUE_CK) ) && ( (R1 != RED_CK) || (G1 != GREEN_CK) || (B1 != BLUE_CK) ) )
				{
					y0 = y1 >> 1;
					cb0 = cb1;
					cr0 = cr1;
					//printf("C1:y0=%d,y1=%d,cb0=%d,cr0=%d\n", y0, y1, cb0, cr0);
				 }//else if ((rgb0 != colorKey) && (rgb1 == colorKey))
				else if( ( (R0 != RED_CK) || (G0 != GREEN_CK) || (B0 != BLUE_CK) ) && ( (R1 == RED_CK) && (G1 == GREEN_CK) && (B1 == BLUE_CK) ) )
				{
					y1 = y0 >> 1;
					//printf("C2:y0=%d,y1=%d,cb0=%d,cr0=%d\n", y0, y1, cb0, cr0);
				}
				else
				{ // RGB888 to YUV422 for combining first and second point
					cb0 = (cb0 + cb1) >> 1;
					cr0 = (cr0 + cr1) >> 1;
					//printf("C3:y0=%d,y1=%d,cb0=%d,cr0=%d\n", y0, y1, cb0, cr0);

				}
				//printf("R0=%u, G0=%u, B0=%u\nR1=%u, G1=%u, B1=%u\n", R0, G0, B0, R1, G1, B1);
				//printf("Y0=%d(0x%x), Y1=%d(0x%x), Cb0=%d(0x%x), Cr0=%d(0x%x)\n", y0, y0, y1, y1, cb0, cb0, cr0, cr0);
				fwrite(&y0, sizeof(unsigned char), 1, fPtr1);
				fwrite(&y1, sizeof(unsigned char), 1, fPtr1);
				fwrite(&cb0, sizeof(unsigned char), 1, fPtr1);
				fwrite(&cr0, sizeof(unsigned char), 1, fPtr1);
				RGB_FP = true;
			} // else :write data to YUV file
		} // for(i2 = 0; i2 < WIDTH ; i2++)
	} // for(j2 = 0; j2 < HEIGHT ; j2++)
} // for j loop
	  fclose(fPtr1);
	  printf("ofilename=%s\n", ofilename);

	  free(str); // Release 1 dimension array to malloc

	  printf("\nTransformation finished! Any key exit!");
	  getchar();
	  getchar();
	  return 0;
}
//---------------------------------------------------------------------------
