
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linker.h"
FILE *in;   //for read module
FILE *out;  //for output link result

/*offset of the current moudle
  assume current module is I+1, the offset of I+1
  will be LENGTH(0) + LENGTH(1) + ...... LENGTH(I)
  where the LENGTH(I) reprent the length of module I
*/
int offset = 0;


struct symTable{
	char symbol[10];	
	int addr;			
	int moduleNo;		
	char msg[100];		
} symbolTable[100];//������š���ַ��ģ��š���Ϣ��

int symbolTableLength = 0;//��¼��ǰ���ű�ļ�¼����

struct symTable useTable[100];//��¼ʹ�õķ���

int useTableLength = 0;	//��¼ʹ�÷��ŵĳ���

struct memMap{         
	char type;			
	int opcode;			
	int oprand;			
	int moduleNo;		
	char msg[100];		
	int visit;			
} memoryMap[100];

int memoryMapLength = 0;  //ͨ���ַ����ͣ����������ģ��š���Ϣ�Ƚ��������

struct module{       
	int defNum;		
	int useNum;		
	int progNum;	
	int baseAddr;		
} moduleInfo[100];
int moduleNum = 0;  //ͨ������ţ�ʹ�úţ�����źźͻ�����ַ�Ƚ���ģ����Ϣ��

/*you can modify this function!*/

/*
  test_num: which file test, it must between 1 and 9
  filename: the file that you should do with
*/
void 
link(int test_num, const char *filename)
{
	char outputfile[128];
	memset(outputfile, 0, 128);
	sprintf(outputfile, "output-%d.txt", test_num);
	in = fopen(filename, "r");    //open file for read,the file contains module that you should do with
	out = fopen(outputfile, "w"); //open the "output" file to output link result
	if(in==NULL||out==NULL)
	{
		fprintf(stderr, "can not open file for read or write\n");
		exit(-1);
	}
	process_one(); //resolve
	process_two(); //relocate
	fclose(in);
	fclose(out);

}

//������ļ�����
void writeToFile()
{
	int i = 0;

	//������ű�
	fprintf(out,"Symbol Table\n");
	for(i = 0; i<symbolTableLength; i++)
	{
		if(symbolTable[i].msg[0] == '\0') //�޾��棬ֱ�����
		{
			fprintf(out,"%s=%d\n",symbolTable[i].symbol, symbolTable[i].addr);
		}
		else if(symbolTable[i].msg[0] == 'W')  //���������Ϣ
		{
			fprintf(out,"%s=%d\n",symbolTable[i].symbol, symbolTable[i].addr);
		}
		else  //���������Ϣ
		{
			fprintf(out,"%s=%d %s\n",symbolTable[i].symbol, symbolTable[i].addr, symbolTable[i].msg);
		}
	}

	fprintf(out,"\nMemory Map\n");
	//��Memory Map�������
	for(i = 0; i<memoryMapLength; i++)
	{
		if(memoryMap[i].msg[0] == '\0')
		{
			fprintf(out,"%d:  %d%03d\n",i,memoryMap[i].opcode, memoryMap[i].oprand);//��û�д�����Ϣ���ʱ
		}
		else
		{
			fprintf(out,"%d:  %d%03d %s\n",i,memoryMap[i].opcode, memoryMap[i].oprand, memoryMap[i].msg);	//���д�����Ϣ���ʱ
		}	
	}

	fprintf(out,"\n");

	for(i = 0; i<useTableLength; i++)
	{
		if(useTable[i].msg[0] != '\0')
		{
			fprintf(out,"%s\n", useTable[i].msg);
		}
	}
	for(i = 0; i<symbolTableLength; i++)
	{
		if(symbolTable[i].msg[0] == 'W')
		{
			fprintf(out,"%s\n",symbolTable[i].msg);
		}
	}

}

/*you must implement this function*/
void 
process_one()
{
	
	int i = 0;       
	int count = 0;
	char symbol[10];
	int value = 0;
	int nowBaseAddr = 0;
	setbuf(in, NULL);    //�������
	while(fscanf(in, "%d", &count) != EOF)
	{
		for(i = 0; i<count; i++)	
		{
			fscanf(in, "%s %d", symbol,&value);//ͨ��fscanf������difinition list ���ж�ȡ
			strcpy(symbolTable[symbolTableLength].symbol, symbol);
			symbolTable[symbolTableLength].addr = nowBaseAddr + value;
			symbolTable[symbolTableLength].moduleNo = moduleNum;	
			symbolTableLength++;
		}
		moduleInfo[moduleNum].defNum = count;  

	
		fscanf(in, "%d", &count); 
		for(i = 0; i<count; i++)   
		{
			fscanf(in, "%s %d", symbol,&value);
			strcpy(useTable[useTableLength].symbol, symbol);
		//��strcpy��������÷��ŵĵľ��Ե�ַ
			useTable[useTableLength].addr = nowBaseAddr + value;	
			useTable[useTableLength].moduleNo = moduleNum;
			useTableLength++;
		}
		moduleInfo[moduleNum].useNum = count;

		fscanf(in, "%d", &count);    
		//��program text
		for(i = 0; i<count; i++)
		{
			fscanf(in, "%s %d", symbol,&value);
	
			memoryMap[memoryMapLength].type = symbol[0];
			memoryMap[memoryMapLength].opcode = value/1000;
			memoryMap[memoryMapLength].oprand = value%1000;
			memoryMap[memoryMapLength].moduleNo = moduleNum;
			memoryMapLength++; //ͨ��fscanf������program text ���ж�ȡ
		}
		moduleInfo[moduleNum].progNum = count;

		nowBaseAddr = nowBaseAddr + count;	
		moduleInfo[moduleNum+1].baseAddr = nowBaseAddr;
		
		moduleNum++;	//����һģ��Ļ���ַ���м���
	
	}
}

/*you must implement this function*/
void
process_two()
{
	int i = 0;
	int j = 0;
	int addr = 0;
	char msgtmp[100];	
	int k = 0;
	int flag = 0;
	

	for(i = 0; i<symbolTableLength; i++)
	{
		for(j = i+1; j<symbolTableLength; j++)
		{
			if(strcmp(symbolTable[i].symbol, symbolTable[j].symbol) == 0)
			{
				strcpy(symbolTable[i].msg,"Error: This variable is multiply defined; first value used.");

				for(k = j; k<symbolTableLength; k++)
				{
					if(k == symbolTableLength-1)	
					{
						strcpy(symbolTable[k].symbol,"");
						symbolTable[k].addr = 0;
						break;
					}
					strcpy(symbolTable[k].symbol, symbolTable[k+1].symbol);
					symbolTable[k].addr = symbolTable[k+1].addr;
				}
				symbolTableLength--;
			}	//�����ظ�������
		}

		for(j = 0; j<useTableLength; j++)
		{
			if(strcmp(symbolTable[i].symbol, useTable[j].symbol) == 0)
			{
				break;
			}
		}
		if(j == useTableLength)
		{
			sprintf(msgtmp, "Warning: %s was defined in module %d but never used.",symbolTable[i].symbol, symbolTable[i].moduleNo+1);
			strcpy(symbolTable[i].msg, msgtmp);
			
		}
		//����ȴδʹ�ü��

	
		if(symbolTable[i].addr > (moduleInfo[symbolTable[i].moduleNo].baseAddr + moduleInfo[symbolTable[i].moduleNo].progNum - 1) )
		{
			symbolTable[i].addr = moduleInfo[symbolTable[i].moduleNo].baseAddr + 0;
			sprintf(msgtmp, "Error: The value of %s is outside module %d; zero (relative) used",symbolTable[i].symbol, symbolTable[i].moduleNo+1);
			strcpy(symbolTable[i].msg, msgtmp); 
		}
	}	//���Ŷ����ַ����������ģ��������

	
	for(i = 0; i<memoryMapLength; i++)
	{
		if(memoryMap[i].type == 'R' || (memoryMap[i].type == 'E' && memoryMap[i].oprand != 777))
		{
			memoryMap[i].oprand = memoryMap[i].oprand + moduleInfo[memoryMap[i].moduleNo].baseAddr;//��memory map �е�R ��E����ָ��
			//����Ե�ַת��Ϊ���Ե�ַ
		}
	
	}

	
	for(i = 0; i<useTableLength; i++)
	{
		int addr = 0;
		int tmpAddr = 0;
		int nextAddr = 0;
		int flag = 0;	

		for(j = 0; j<symbolTableLength; j++)
		{
			if(strcmp(symbolTable[j].symbol, useTable[i].symbol) == 0)
			{
				addr = symbolTable[j].addr;
				break;
			}//����E��������
		}

		
		if(j == symbolTableLength)
		{
			flag = 1;	
			sprintf(msgtmp, "Error: %s is not defined; zero used.",useTable[i].symbol);
			strcpy(memoryMap[useTable[i].addr].msg, msgtmp);
		}//����δ������

		if(memoryMap[useTable[i].addr].type != 'E')
		{
			sprintf(msgtmp, "Error: %c type address on use chain; treated as E type.",memoryMap[useTable[i].addr].type);
			strcpy(memoryMap[useTable[i].addr].msg, msgtmp);
			
			if(memoryMap[useTable[i].addr].oprand != 777 && memoryMap[useTable[i].addr].type != 'R')
			{
				memoryMap[useTable[i].addr].oprand = memoryMap[useTable[i].addr].oprand + moduleInfo[memoryMap[useTable[i].addr].moduleNo].baseAddr;
			}
			memoryMap[useTable[i].addr].type = 'E';
		}
		//������е�ָ���E���ͼ��
		nextAddr = memoryMap[useTable[i].addr].oprand;
		memoryMap[useTable[i].addr].visit = 1;	
		memoryMap[useTable[i].addr].oprand = addr;
		while(1)
		{
			if(nextAddr == 777)
			{
				break;
			}
			if(memoryMap[nextAddr].type != 'E')
			{
				sprintf(msgtmp, "Error: %c type address on use chain; treated as E type.",memoryMap[nextAddr].type);
				strcpy(memoryMap[nextAddr].msg, msgtmp);
				
				if(memoryMap[nextAddr].oprand != 777 && memoryMap[nextAddr].type != 'R')
				{
					memoryMap[nextAddr].oprand = memoryMap[nextAddr].oprand + moduleInfo[memoryMap[nextAddr].moduleNo].baseAddr;		
				}
				memoryMap[nextAddr].type = 'E';
			}	//������е�ָ���E���ͼ��
			
		
			if(flag == 1)
			{
				sprintf(msgtmp, "Error: %s is not defined; zero used.",useTable[i].symbol);
				strcpy(memoryMap[nextAddr].msg, msgtmp);
			}	//����δ������
			
			memoryMap[nextAddr].visit = 1; 

	
			if(memoryMap[nextAddr].oprand != 777 && memoryMap[nextAddr].oprand >(moduleInfo[useTable[i].moduleNo].baseAddr + moduleInfo[useTable[i].moduleNo].progNum - 1))
			{
				
				memoryMap[nextAddr].oprand = addr;
				strcpy(memoryMap[nextAddr].msg, "Error: Pointer in use chain exceeds module size; chain terminated.");
				break;		//ָ�����һ����ַ�����˸�ģ�鷶Χ���
			}

			tmpAddr = nextAddr;
			nextAddr = memoryMap[nextAddr].oprand;
			memoryMap[tmpAddr].oprand = addr;
		}

	}
 
	for(i = 0; i<memoryMapLength; i++)
	{
		if(memoryMap[i].type == 'E' && memoryMap[i].visit != 1)
		{
			strcpy(memoryMap[i].msg, "Error: E type address not on use chain; treated as I type.");
			memoryMap[i].type = 'I';
		}
	}
	writeToFile();                
} //E���͵�ַ���ڴ�����м��
	

