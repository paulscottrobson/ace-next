/*

	Based on the theory behind TAPH.PAS by TechnoMancer.

	Requires a 22khz Mono Sample in .WAV format

*/

#include <stdio.h>
#include <stdlib.h>

FILE *f,*o;

void WaveOpen(char *FName)
{
f = fopen(FName,"rb");					/* Open input file */
fseek(f,44L,SEEK_SET);					/* Skip the header */
o = fopen("out.tap","wb");				/* Open the output file */
}


static long last;						/* Last Edge offset */

int flip = 0;							/* 0 = last went up,1 = last down */

int GetEdgeSize(void)
{
int n1,n2,n3;
long res,curr;

n1 = n2 = n3 = 0;						/* Initial values */
if (flip == 0)
		n1 = n2 = n3 = 255;

while (1)
	{
	n3 = fgetc(f);						/* Get new one */
	n3 = n3 & 0xFF;
										/* When in order and zero-x */
	if ( (n1 < n2 && n2 < n3 && n1 < 128 && n3 > 128) ||
		 (n1 > n2 && n2 > n3 && n1 > 128 && n3 < 128))
		{
		flip = (n1 < n2) ? 0 : 1;		/* Which way are we going ? */
		curr = ftell(f);				/* Work out offset to last */
		res = curr-last;
		last = curr;					/* update last */
		return((int)res);				/* return distance */
		}
	n1 = n2;n2 = n3;					/* Otherwise rotate next in */
	}
return(0);
}

struct _Block							/* Structure of ACE Header */
{
unsigned char Type;
char Name[10];
unsigned int Size;
char Padding[32];
} Block;

void ReadBlock(int Size,unsigned char *Block)
{
int i,b,n,e1,e2;
while (i = GetEdgeSize(),				/* Skip leader */
				i >= 11 && i < 17) { }

Size++;

for (i = 0;i < 8*2+2;i++)				/* Skip miscellaneous stuff... */
						GetEdgeSize();

fputc(Size & 0xFF,o);					/* Header to output file */
fputc(Size >> 8,o);

for (i = 0;i < Size;i++)				/* Copy bytes */
	{
	b = 0;
	for (n = 0;n < 8;n++)				/* Read in a bit at a time */
		{
		b = b * 2;
		e1 = GetEdgeSize();
		e2 = GetEdgeSize();				/* One complete wave per bit */
/*		printf("%d:%d ",e1,e2); */
		if (e1 > 8) b++;				/* If leading edge > 8 its a '1' */
		}
/*	printf("%02x %c ",b,b & 0x7F); */
	fputc(b,o);							/* Write to .TAP file */
	if (Block != NULL) *(Block+i) = b;	/* Store in header if applicable */
	}
printf("Read block %d bytes.\n",Size);
for (i = 0;i < 40;i++) printf("%d ",GetEdgeSize());
}

void main(int argc,char *argv[])
{
int i;
if (argc != 2) exit(printf("TAPH <wave file>\n"));
WaveOpen(argv[1]);
if (f == NULL) exit(printf("Couldn't open....\n"));

for (i = 0;i < 14;i++) GetEdgeSize();	/* Skip any junk */

ReadBlock(25,(unsigned char *)&Block);	/* Read header */

printf("Block Type : %d\n",Block.Type);	/* Print information */
printf("Block Size : %d\n",Block.Size);
printf("Block Name : ");
for (i = 0;i < 10;i++) printf("%c",Block.Name[i] & 0x7F);
printf("\n");

ReadBlock(Block.Size,NULL);				/* Read body */
fclose(f);
fclose(o);
}







