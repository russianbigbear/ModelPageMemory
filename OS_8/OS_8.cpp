#include <locale.h>

#include <iostream>
#include <cstdio>
#include <string>

#include <vector>

#include <ctime>
#include <dos.h>
#include <windows.h>
#define DisplayMemoryVersion 4 //	 1 = short ; 2 = long ; 3 - средний)0 ; 4 - средний с выводо номеров страниц в памяти
#define SizeHDD 256
#define TIMESLEEP 1000
using namespace std;

int GLOBALcountprocess = 0;
int numprocess = 1;

struct ElemSequence {
	int process;
	int page;
	time_t time;
	ElemSequence() {
		this->page = 0;
		this->process = 0;
	}
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class process {
private:
	int size;
	int pid;
	string name;
	int needpages;
	vector<int> table;
public:
	process();
	process(int pid, int size, string name);
	void set_size(int size);
	int get_size();
	void set_pid(int pid);
	int get_pid();
	int get_pages();
	void set_name(string name);
	string get_name();
};

int process::get_pages()
{
	int need = (this->size / 4);			//сколько СТРАНИЦ в памяти занимает добавляемая программа
	if (this->size % 4 > 0) {
		need++;
	}
	this->needpages = need;
	return this->needpages;
}

process::process()
{
	this->size = 1;
	this->pid = GLOBALcountprocess++;
	this->name = "name";
}

process::process(int pid, int size, string name)
{
	this->size = size;
	this->pid = pid;
	this->name = name;
	int needpages = (this->size / 4);			//сколько СТРАНИЦ в памяти занимает добавляемая программа
	if (this->size % 4 > 0) {
		needpages++;
	}
	for (int i = 0; i < needpages; i++) {
		table.push_back(0);
	}
	this->needpages = needpages;
}

void process::set_size(int size)
{
	this->size = size;
}

int process::get_size()
{
	return this->size;
}

void process::set_pid(int pid)
{
	this->pid = pid;
}

int process::get_pid()
{
	return this->pid;
}

void process::set_name(string name)
{
	this->name = name;
}

string process::get_name()
{
	return this->name;
}
//////////////////////////////////////////////////////
class AllProcess {
private:
	vector <process> ListProcess;
	vector <ElemSequence> Sequence;	// в какой последовательности вызывались процессы ; самый старый процесс - кандидат на перенос на внешний носитель

	vector<vector<int>> TableAdress;	//таблица адресов расположения страниц процесса в ОЗУ(memoryBit)
	int countProcess;

	vector<vector<ElemSequence>> TableTime;

	int countFreeMemory;	//количество свободной оперативной памяти
	vector<int> memoryBit;
	vector<int> memoryPages; // не использую почти)00

	vector<int> memoryStorage;	//внешний носитель, выделил ему памяти в два раза оперативки


	void addToSequence(process tmp);		//добавляет процесс в последоватеьность вызовов

	int SearchFreeSeatRAM();

public:
	AllProcess();
	void set_countProcess(int countProcess);
	int get_countProcess();
	void add_process(process tmp);
	process get_process(int num);
	void displayMemory();
	int delet_process(int proc);
	int call_process_page(int id, int pag);

	void ShowHDD();
};

/// 
int AllProcess::SearchFreeSeatRAM()
{
	//Поиск самого старого процесса(поиск наименьшего времени)
	time_t minTime;
	int indexI;
	int indexJ;
	// нужно ввести проверку на ПЕРВУЮ минимальную дату(он уже может бытвь ыгружен на внешку)
	bool flagFindFirstMin = false;
	for (int i = 0; i < TableTime.size() && !flagFindFirstMin; i++) {
		for (int j = 0; j < TableTime[i].size() && !flagFindFirstMin; j++) {
			if (TableAdress[i][j] >= 0) {	//если эта страница не выгружена на внешний носитель
				indexI = i;
				indexJ = j;
				minTime = TableTime[indexI][indexJ].time;
				flagFindFirstMin = true;
				break;
			}
		}
	}
	//поис минимальной даты
	for (int i = 0; i < TableTime.size(); i++) {
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		for (int j = 0; j <TableTime[i].size(); j++) {
			if (TableAdress[i][j] >= 0) {	//если эта страница не выгружена на внешний носитель
				if (TableTime[i][j].time < minTime) {
					minTime = TableTime[i][j].time;
					indexI = i;
					indexJ = j;
				}
			}
		}
	}
	printf("");
	//выгрузка этого процесса во внешнюю память;
	int indexInTableAdress = TableAdress[indexI][indexJ];
	for (int i = 1; i < SizeHDD; i = i + 4) {
		//Если страница во внешней памяти пуста, то можем туда записать
		if (memoryStorage[i] == 0) {
			for (int b = 0; b < 4; b++) {	//заполняем страницу во внешней памяти
				memoryStorage[i + b] = memoryBit[indexInTableAdress + b];
			}
			TableAdress[indexI][indexJ] = i * (-1);	// Файл хранящийся на HDD отображается в таблице адресов с отрицательным знаком
			break;
		}
	}
	//чистка освободившегося места
	for (int b = 0; b < 4; b++) {
		memoryBit[indexInTableAdress + b] = 0;
	}
	return indexInTableAdress;
}

//не дописан метод
int AllProcess::call_process_page(int id, int pag)
{
	int numInTable = 0; // порядковый номер процесса в списке
	for (int i = 0; i < ListProcess.size(); i++) {
		if (id == ListProcess[i].get_pid()) {
			numInTable = i;
			break;
		}
	}
	//Если такой страницы не существует
	if (TableAdress[numInTable].size() < pag - 1) {
		cout << endl << "\tПроцесс PID = " << id << " не содержит страницу " << pag << endl;

		this->displayMemory();						// выыводит содержание памяти
		return -1;
	}
	//Если Страница в ОЗУ, просто меняем время вызова
	if (TableAdress[numInTable][pag - 1] >= 0) {
		TableTime[numInTable][pag - 1].time = time(NULL);
		Sleep(500);
		cout << "\tСтраница " << pag << " процесса PID = " << id << " была успешно вызвана" << endl;
		Sleep(500);

		this->displayMemory();						// выыводит содержание памяти
		return 1;
	}
	//свободное место в озу
	int free = 0;
	for (int i = 0; i < 64; i = i + 4) {
		if (memoryBit[i] == 0)
			free++;
	}
	this->countFreeMemory = free;
	//Если Страница на HDD, и в ОЗУ ЕСТЬ МЕСТО, то пишем в это свободное место
	if (this->countFreeMemory > 0) {
		int indexInMemoryStorage = TableAdress[numInTable][pag - 1] * (-1);
		for (int i = 0; i < 64; i = i + 4) {
			if (memoryBit[i] == 0) {
				//нашли свободное место в оперативке - туда и пишем
				for (int b = 0; b < 4; b++) {
					memoryBit[i + b] = memoryStorage[indexInMemoryStorage + b];
					memoryStorage[indexInMemoryStorage + b] = 0;
				}
				memoryPages[i / 4] = pag;
				TableAdress[numInTable][pag - 1] = i;		//меняем адрес этой, HDD -> RAM, страницы
				TableTime[numInTable][pag - 1].time = time(NULL);				//меняем время этой, HDD -> RAM, страницы
				this->displayMemory();						// выыводит содержание памяти
				break;
			}
		}
	}
	else {
		//Если Страница на HDD, и в ОЗУ НЕТ МЕСТА, то нужно выгрузить наиболее старую страницу, и поместить туда нашу страницу
		int indexFreeMemoryBit = 0;
		if (TableAdress[numInTable][pag - 1] < 0) {
			// поиск и выгрузка самого старого процесса, возвращаем свободное место в оперативке
			indexFreeMemoryBit = this->SearchFreeSeatRAM();
			// загружаем с HDD в RAM вызываемую страницу процесса
			int indexInMemoryStorage = TableAdress[numInTable][pag - 1] * (-1);
			cout << "\tСтраница " << pag << " процесса PID = " << id << " находится на HDD, идет процесс выделения свободной памяти в RAM" << endl;
			Sleep(333);
			cout << "\tСтраница " << pag << " процесса PID = " << id << " загружена в RAM" << endl;
			Sleep(333);
			cout << "\tСтраница " << pag << " процесса PID = " << id << " была успешно вызвана" << endl;
			Sleep(333);
			for (int b = 0; b < 4; b++) {
				memoryBit[indexFreeMemoryBit + b] = memoryStorage[indexInMemoryStorage + b];
				memoryStorage[indexInMemoryStorage + b] = 0;
			}
			memoryPages[indexFreeMemoryBit / 4] = pag;
			TableAdress[numInTable][pag - 1] = indexFreeMemoryBit;		//меняем адрес этой, HDD -> RAM, страницы
			TableTime[numInTable][pag - 1].time = time(NULL);				//меняем время этой, HDD -> RAM, страницы

			this->displayMemory();						// выыводит содержание памяти
		}
	}
	return 0;
}

void AllProcess::ShowHDD()
{
	cout << "\tHDD:" << endl;
	for (int i = 1; i < SizeHDD + 1; i++) {
		cout << "|" << this->memoryStorage[i];
		if (i % 16 == 0)
			cout << "|" << endl;
	}
	cout << endl;
}



AllProcess::AllProcess()
{
	//выделение памяти под внешний носитель
	for (int i = 0; i < SizeHDD + 1; i++) {
		memoryStorage.push_back(0);
	}
	memoryStorage[0] = -1337;
	for (int i = 0; i < 16; i++) {
		memoryPages.push_back(0);
	}

	for (int i = 0; i < 16 * 4; i++) {
		memoryBit.push_back(0);
	}
	//последовательность вызовов страниц
	for (int i = 0; i < 64 / 4; i++) {
		ElemSequence tmp;
		tmp.page = 0;
		tmp.process = 0;
		tmp.time = 0;
		Sequence.push_back(tmp);
	}
	countProcess = 0;
	countFreeMemory = 64;		//количество свободной оперативной памяти
}

int AllProcess::delet_process(int pid) {

	int proc = -1;
	for (int i = 0; i < ListProcess.size(); i++) {
		if (ListProcess[i].get_pid() == pid) {
			proc = i;
			break;
		}
	}
	if (proc == -1) {
		cout << "Такого PID'a нет!" << endl;
		this->displayMemory();
		return -1;
	}
	//удаляем из ОЗУ и HDD
	for (int i = 0; i < TableAdress[proc].size(); i++) {
		int index = TableAdress[proc][i];
		if (index >= 0) {		//удаляем из ОЗУ
			for (int j = 0; j < 4; j++) {
				memoryBit[index + j] = 0;
			}
		}
		else {					//удаляем с HDD
			index = (index * (-1));
			for (int j = 0; j < 4; j++) {
				memoryStorage[index + j] = 0;
			}
		}
	}
	//удаляем из списка процессов
	ListProcess.erase(ListProcess.begin() + proc);
	//удаляем из ТАБЛИЦЫ ардесов
	TableAdress.erase(TableAdress.begin() + proc);
	//удаляем из ТАБЛИЦЫ времени
	TableTime.erase(TableTime.begin() + proc);

	this->countProcess = this->countProcess - 1;
	this->displayMemory();
	return 0;
}

void AllProcess::set_countProcess(int countProcess)
{
	this->countProcess = countProcess;
}

int AllProcess::get_countProcess()
{
	return countProcess;
}

void AllProcess::add_process(process tmp)
{
	int needpages = (tmp.get_size() / 4);			//сколько СТРАНИЦ в памяти занимает добавляемая программа
	if (tmp.get_size() % 4 > 0) {
		needpages++;
	}
	vector<int> tmpTableAdress;					//таблица расположения страниц в оперативной памяти

	vector <ElemSequence> tmpTableTime;

	int needBites = tmp.get_size();				//сколько бит в памяти занимает добавляемая программа
	int countAddedPages = 0;
	for (int i = 0; i < memoryBit.size(); i = i + 4) {
		if (needBites) {		// если еще не вся прога записана в память, то продолжаем искать свободные страницы и писать
			if (memoryBit[i] == 0) {		//значит страница свободна(и 4 байта свободных подряд есть)

				needpages--; //уменьшаем коилчество нераспределенныйх страниц
				countFreeMemory = countFreeMemory - 4;		//уменьшаем количесвто свободной памяти
															//записываем адресс процесса
				tmpTableAdress.push_back(i);
				//записываем время добавления страницы в ОЗУ
				ElemSequence elemSequence;
				elemSequence.process = tmp.get_pid();
				elemSequence.page = tmpTableTime.size() + 1;
				elemSequence.time = time(NULL);
				tmpTableTime.push_back(elemSequence);

				for (int b = 0; b < 4; b++) {	//заполняем страницу настолько, сколько нужно

					if (needBites > 0) {		//если есть еще не записанный бит процесса, то пишем \ иначе оставляем его пустым
						memoryBit[i + b] = tmp.get_pid();	//пишем очередной бит в память(записываем id процесса)
						needBites--;		// уменьшаем количество нераспределенных битов
					}
				}
				memoryPages[i / 4] = tmpTableAdress.size();
			}
		}
		// если нераспределенных битов не осталось, то выходим из цикла
		else {
			break;
		}
	}
	printf("");
	//Если не все влезло в оперативку, то нужно выгрузить наименее востребованную страницу на ВИНТ
	// апгрейт, выгружаем только одну страницу, а все остальное пихаем в оперативку
	if (needBites > 0) {
		//если процесс не влез в оперативку, то загружаем лишь одну страницу в ОЗУ
		printf("");
		// Ищем самую старую добавленную страницу, выгружаем ее на диск, освобождая место для страницы нового процесса
		int indexInTableAdress = 0;
		indexInTableAdress = this->SearchFreeSeatRAM();
		printf("");
		//записываем адресс процесса
		tmpTableAdress.push_back(indexInTableAdress);
		//записываем время добавления страницы в ОЗУ
		ElemSequence elemSequence;
		elemSequence.process = tmp.get_pid();
		elemSequence.page = tmpTableTime.size() + 1;
		elemSequence.time = time(NULL);
		tmpTableTime.push_back(elemSequence);
		printf("");
		//Записываем страницу, в освободившееся место в ОЗУ
		for (int b = 0; b < 4; b++) {	//заполняем страницу настолько, сколько нужно
			if (needBites > 0) {		//если есть еще не записанный бит процесса, то пишем \ иначе оставляем его пустым
				memoryBit[indexInTableAdress + b] = tmp.get_pid();	//пишем очередной бит в память(записываем id процесса)
				needBites--;		// уменьшаем количество нераспределенных битов
			}
			else {
				memoryBit[indexInTableAdress + b] = 0;
			}
		}
		memoryPages[indexInTableAdress / 4] = tmpTableAdress.size();
		printf("");
		// теперь все остальные страницы добалвяемого процесса мы пихаем на HDD
		while (needBites > 0) {
			for (int i = 1; i < SizeHDD && needBites > 0; i = i + 4) {
				//Если страница во внешней памяти пуста, то можем туда записать
				if (memoryStorage[i] == 0) {
					for (int b = 0; b < 4; b++) {	//заполняем страницу настолько, сколько нужно
						if (needBites > 0) {		//если есть еще не записанный бит процесса, то пишем \ иначе оставляем его пустым
							memoryStorage[i + b] = tmp.get_pid();	//пишем очередной бит в память(записываем id процесса)
							needBites--;		// уменьшаем количество нераспределенных битов
						}
						else {
							memoryStorage[i + b] = 0;
						}
					}
					int index;
					index = i *(-1);
					// записываем адрес прцоесса в TableAdress
					tmpTableAdress.push_back(index);
					// Записываем время в таблицу tableTime
					ElemSequence elemSequence;
					elemSequence.process = tmp.get_pid();
					elemSequence.page = tmpTableTime.size() + 1;
					elemSequence.time = time(NULL);
					tmpTableTime.push_back(elemSequence);
				}
			}
		}
	}
	TableTime.push_back(tmpTableTime);
	TableAdress.push_back(tmpTableAdress);		// Добавляем таблицу адресов текущего процесса в список
	(this->countProcess)++;						// увеличиваем коилчество процессов
	ListProcess.push_back(tmp);				// добавляем процесс в список
	Sleep(TIMESLEEP);
	this->displayMemory();						// выыводит содержание памяти
}

process AllProcess::get_process(int num)
{
	return ListProcess[num];
}

void AllProcess::displayMemory()
{
	switch (DisplayMemoryVersion)
	{
	case 1:
	{
		cout << "\tRAM:" << endl;
		for (int i = 0; i < 66; i++)
			cout << "=";
		cout << endl << "|";
		for (int i = 0; i < 64; i++)
			cout << memoryBit[i];
		cout << "|" << endl;
		for (int i = 0; i < 66; i++)
			cout << "=";
		printf("\n");
		break;
	}
	case 2:
	{

		cout << "\tRAM:" << endl;
		for (int i = 0; i < 130; i++)
			cout << "=";
		cout << endl << "|";
		for (int i = 0; i < 64; i++)
			cout << memoryBit[i] << "|";
		cout << "|" << endl;
		for (int i = 0; i < 130; i++)
			cout << "=";
		printf("\n");
		break;
	}
	case 3:
	{
		cout << "\tRAM:" << endl;
		for (int i = 0; i < 82; i++)
			cout << "=";
		cout << endl << "|";
		int k = 0;
		for (int i = 0; i < 64; i++) {
			cout << memoryBit[i];
			k++;
			if (k == 4) {
				cout << "|";
				k = 0;
			}
		}
		cout << "|" << endl;
		for (int i = 0; i < 82; i++)
			cout << "=";
		printf("\n");
		break;
	}
	case 4:
	{
		cout << "\tRAM:" << endl;
		for (int i = 0; i < 82; i++)
			cout << "=";

		cout << endl << "|";
		for (int i = 0; i < memoryPages.size(); i++) {
			cout << " " << memoryPages[i] << "  |";
		}
		cout << endl;

		int k = 0;
		cout << "|";
		for (int i = 0; i < 64; i++) {
			cout << memoryBit[i];
			k++;
			if (k == 4) {
				cout << "|";
				k = 0;
			}
		}
		cout << endl;
		for (int i = 0; i < 82; i++)
			cout << "=";
		printf("\n");
		break;
	}
	}


}
//////////////////////////////////////////////////////
void ReloadProcess(AllProcess AP);
int getPid(int(&masPID)[10]);
//////////////////////////////////////////////////////
int main()
{
	int masPID[10];
	for (int i = 0; i < 10; i++)
		masPID[i] = 0;
	int PID = 1;
	setlocale(LC_ALL, "rus");
	AllProcess allprocess;
	for (int i = 0; i < 66; i++)
		cout << "=";
	cout << endl;
	cout << "|";
	for (int j = 0; j < 2; j++)
		for (int i = 1; i <= 8; i++)
			for (int z = 0; z < 4; z++)
				printf("%d", i);

	cout << "|" << endl;
	for (int i = 0; i < 66; i++)
		cout << "=";
	cout << endl;

	PID = getPid(masPID);
	process tmp(PID, 11, "everest");
	allprocess.add_process(tmp);

	PID = getPid(masPID);
	tmp.set_name("nvidia");
	tmp.set_pid(PID);
	tmp.set_size(3);
	allprocess.add_process(tmp);
	ReloadProcess(allprocess);

	PID = getPid(masPID);
	tmp.set_name("torrent");
	tmp.set_pid(PID);
	tmp.set_size(2);
	allprocess.add_process(tmp);
	ReloadProcess(allprocess);

	PID = getPid(masPID);
	tmp.set_name("antivirus");
	tmp.set_pid(PID);
	tmp.set_size(20);
	system("cls");
	allprocess.add_process(tmp);
	ReloadProcess(allprocess);

	printf("\n");
	while (1) {
		printf("\n");
		string str;
		cin >> str;
		if (str == "call") {
			int proc;
			int page;
			cin >> proc;
			cin >> page;

			system("cls");
			allprocess.call_process_page(proc, page);
			ReloadProcess(allprocess);
		}
		if (str == "add") {
			int err = 0;
			string name;
			int size;
			cin >> name;
			cin >> size;
			system("cls");
			err = PID = getPid(masPID);
			if (size > 100) {
				cout << "\tРазмер процесса должен быть меньше 100!" << endl;
				allprocess.displayMemory();
				ReloadProcess(allprocess);
			}
			else {
				if (err != -1) {
					tmp.set_name(name);
					tmp.set_pid(PID);
					tmp.set_size(size);
					allprocess.add_process(tmp);
					ReloadProcess(allprocess);
				}
				else {
					cout << "Закончились PID'ы(крч, хватит добавлять процессы, удалите чего-нибудь)";
					allprocess.displayMemory();
					ReloadProcess(allprocess);
				}
			}
		}
		if (str == "delete") {
			int num;
			cin >> num;

			system("cls");
			allprocess.delet_process(num);
			ReloadProcess(allprocess);

		}
		if (str == "showHDD") {
			allprocess.ShowHDD();
		}
		if (str == "help") {
			cout << endl;
			cout << "call аргумент1 аргумент2" << endl;
			cout << "\t Аргумент1 - PID процесса, страницу которого хотим вызвать" << endl;
			cout << "\t Аргумент2 - номер страницы которую хотим вызвать" << endl;
			cout << "add аргумент1 аргумент2" << endl;
			cout << "\t Аргумент1 - Имя добавляемого процесса" << endl;
			cout << "\t Аргумент2 - размер добавляемого процесса" << endl;
			cout << "delete аргумент1 " << endl;
			cout << "\t Аргумент1 -PID удаляемого процесса" << endl;
			cout << "showHDD Показать состояние памяти HDD" << endl;
		}
		printf("");
	}
	return 0;
}

void ReloadProcess(AllProcess allprocess)
{
	printf("\n PID");
	for (int i = 0; i < 4; i++)
		cout << " ";
	cout << "Name";
	for (int i = 0; i < 10; i++)
		cout << " ";
	cout << "Size";
	for (int i = 0; i < 5; i++)
		cout << " ";
	cout << "Pages";
	for (int i = 0; i < 10; i++)
		cout << " ";
	cout << endl;

	int countProcess = allprocess.get_countProcess();
	for (int i = 0; i < countProcess; i++) {
		cout << " ";
		process tmp;
		tmp = allprocess.get_process(i);

		cout.width(7);
		cout.setf(ios::left);
		cout << tmp.get_pid();

		cout.width(14);
		cout.setf(ios::left);
		cout << tmp.get_name();

		cout.width(9);
		cout.setf(ios::left);
		cout << tmp.get_size();

		cout.width(9);
		cout.setf(ios::left);
		cout << tmp.get_pages();

		cout << endl;
	}

}

int getPid(int(&masPID)[10]) {
	int countFree = -1;
	for (int i = 0; i < 10; i++) {
		if (masPID[i] == 0)
			countFree++;
	}
	if (countFree) {
		int PID;
		do {
			PID = rand() % 9 + 1;

		} while (masPID[PID] != 0);
		masPID[PID] = 1;
		return PID;
	}
	else
		return -1;
}

