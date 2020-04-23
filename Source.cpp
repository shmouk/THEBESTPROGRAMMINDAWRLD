#include<stdio.h>
#include<windows.h>
#include<conio.h>
#include<time.h>
#include<iostream>
#include<string>

using namespace std;

void Server(char* path);
void Client();



int main(int argc, char* argv[])
{
	switch(argc)
	{

	case 1:
		Server(argv[0]);		
		break;

	default:
		Client();		
		break;
	}
	}



void Server(char* path)
{
	STARTUPINFO si;//для определения свойств главного окна
	ZeroMemory( &si, sizeof(si) );//заполняет блок памяти нулями.// указатель на блок памятиобнуления памятиобнуления памятиобнуления памяти
	ZeroMemory( &si, sizeof(si) );//заполняет блок памяти нулями.// указатель на блок памятиобнуления памятиобнуления памятиобнуления памяти
	si.cb = sizeof(si);

	PROCESS_INFORMATION childProcessInfo;//заполняется функцией CreateProcess с информацией о недавно созданном процессе и его первичном поток
	ZeroMemory(&childProcessInfo, sizeof(childProcessInfo));//обнуления памяти

	HANDLE hMyPipe;
	HANDLE Semaphores[3];	

	char buffer[20];				 // 
	int bufferSize = sizeof(buffer); // 

	string message;


	Semaphores[0] = CreateSemaphore(NULL, 0 , 1, "SEMAPHORE_lab3");      //защитта нач.знач.счётч.семаф   макс знач счётчика  указате на строку имя семафорра
	Semaphores[1] = CreateSemaphore(NULL, 0 , 1, "SEMAPHORE_end_lab3");      // защитта нач.знач.счётч.семаф   макс знач счётчика  указате на строку имя семафорра
	Semaphores[2] = CreateSemaphore(NULL, 0 , 1, "SEMAPHORE_EXIT_lab3");	// 		защитта нач.знач.счётч.семаф   макс знач счётчика  указате на строку имя семафорра

	cout<<"Server process\n\n";

	hMyPipe = CreateNamedPipe("\\\\.\\pipe\\MyPipe",PIPE_ACCESS_OUTBOUND,PIPE_TYPE_MESSAGE|PIPE_WAIT,PIPE_UNLIMITED_INSTANCES ,0,0,INFINITE,(LPSECURITY_ATTRIBUTES)NULL);// kanal режим открытия =макм.кол.реализ.к=размер вых.буфера=(вх)=ожид в милсек=защита

	CreateProcess(path, " 2", NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &childProcessInfo);

	if(!ConnectNamedPipe(hMyPipe,(LPOVERLAPPED)NULL))
		cout<<"Connection failure\n";

	while(1)
	{				
		DWORD NumberOfBytesWritten;

		cout << "\nEnter message:\n";
		cin.clear();//восстанавливает поток
		getline(cin, message);


		if(message == "0")
		{
			ReleaseSemaphore(Semaphores[2], 1, NULL);  //Значение счетчика ресурсов увеличивается
			WaitForSingleObject(childProcessInfo.hProcess, INFINITE);
			break;
		}

		ReleaseSemaphore(Semaphores[0], 1, NULL);   // Значение счетчика ресурсов увеличивается

		//cout << "w1\n";
		int NumberOfBlocks = message.size() / bufferSize + 1;	//		
		WriteFile(hMyPipe, &NumberOfBlocks, sizeof(NumberOfBlocks), &NumberOfBytesWritten, (LPOVERLAPPED)NULL);//сбрасывает событие

		//cout << "w2\n";
		int size = message.size();
		WriteFile(hMyPipe, &size, sizeof(size), &NumberOfBytesWritten, (LPOVERLAPPED)NULL);//для ассин

	//	cout << "w_blocks\n";
		for(int i = 0; i < NumberOfBlocks; i++)
		{					
			message.copy(buffer, bufferSize, i*bufferSize);		
			if(!WriteFile(hMyPipe, buffer, bufferSize, &NumberOfBytesWritten,(LPOVERLAPPED)NULL)) cout<<"Write Error\n";
		}				

		WaitForSingleObject(Semaphores[1], INFINITE); // 
	}			

	CloseHandle(hMyPipe);
	CloseHandle(Semaphores[0]);
	CloseHandle(Semaphores[1]);
	cout << "\n\n";
	system("pause");
	return;
}

void Client()
{
	HANDLE hMyPipe;
	HANDLE Semaphores[3];	

	char buffer[20];				 
	int bufferSize = sizeof(buffer);

	string message;

	bool successFlag;
	Semaphores[0] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "SEMAPHORE_lab3");
	Semaphores[1] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "SEMAPHORE_end_lab3");
	Semaphores[2] =  OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "SEMAPHORE_EXIT_lab3");

	cout<<"Child process\n\n";

	hMyPipe = CreateFile("\\\\.\\pipe\\MyPipe",GENERIC_READ,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);//создание канала


	while(1)
	{				
		successFlag = TRUE;
		DWORD NumberOfBytesRead;
		message.clear();

		int index = WaitForMultipleObjects(3, Semaphores, FALSE, INFINITE) - WAIT_OBJECT_0; // 
		if (index == 2) // 
			break;					

		int NumberOfBlocks;
		if(!ReadFile(hMyPipe, &NumberOfBlocks, sizeof(NumberOfBlocks), &NumberOfBytesRead, NULL)) break;

		int size;
		if(!ReadFile(hMyPipe, &size, sizeof(size), &NumberOfBytesRead, NULL)) break;

		for( int i=0; i < NumberOfBlocks; i++)
		{
			successFlag = ReadFile(hMyPipe, buffer, bufferSize, &NumberOfBytesRead, NULL);
			if(!successFlag) break;

			message.append(buffer, bufferSize); 
		}
		if(!successFlag) break;

		message.resize(size);

	//	cout << message << "\n\n";
		for(int i =0; i < size; i++)
		{
			cout << message[i];
			Sleep(100);
		}
		cout<<endl;
		//				cout<<"\n\t\t\tMessage was printed successfully\n";

		ReleaseSemaphore(Semaphores[1], 1, NULL);	
	}
	CloseHandle(hMyPipe);
	CloseHandle(Semaphores[0]);
	CloseHandle(Semaphores[1]);
	return;
}