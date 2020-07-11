#include <ctime>
#include <iostream>
#include "Process.h"

#define e_chain pair<pair<int, int>, time_t>  // элемент цепочки { { pid, page}, time}
#define HDD 512

/*
Класс "Модель виртуальной страничной памяти"
_count_process - количество процессов
_amount_free_memory - количество свободной RAM
_count_pages - количество занятых страниц процессом в RAM
_list_processes - таблица процессов
_table_pages - таблица страниц процесса
_table_times - таблица, содержащая pid процесса, количество его страниц и время добавления их в RAM
_random_access_memory - вектор, представляющий RAM
_hard_disk_drive - вектор, представляющий HDD
_chain - цепочка элементов по времени
*/
class paging_model {
private:
	int _count_process;
	int _amount_free_memory;
	vector<int> _count_pages;
	vector <process> _list_processes;
	vector<vector<int>> _table_pages;
	vector<vector<e_chain>> _table_times;
	vector<int> _random_access_memory;
	vector<int> _hard_disk_drive;
	vector <e_chain> _chain;

public:
	/*конструктор без параметра, инициализатор*/
	paging_model() {
		_count_process = 0;  // начальное значение количества процессов
		_amount_free_memory = 64; //начальное значение свободной оперативной памяти
		for (int i = 0; i < HDD + 1; i++) _hard_disk_drive.push_back(0); //выделение памяти под HDD
		for (int i = 0; i < 16; i++) _count_pages.push_back(0);
		for (int i = 0; i < 16 * 4; i++) _random_access_memory.push_back(0); //
		for (int i = 0; i < 64 / 4; i++) _chain.push_back({ { 0, 0 }, 0 });
	}

	/*метод вывода _count_process*/
	int get_count_process() { return _count_process; }

	/*метод вывода элемента класса "Процесс"*/
	process get_process(int number) { return _list_processes[number]; }

	/*метод поиска старой страницы(страницы с наименьшим временем)*/
	int not_frequently_used() {
		int I, J;
		time_t minimum_time;

		/*проверка, не выгружена ли первая старая страница на HDD*/
		bool flag = false;
		for (unsigned i = 0; i < _table_times.size() && !flag; i++)
			for (unsigned j = 0; j < _table_times[i].size() && !flag; j++)
				if (_table_pages[i][j] >= 0) {								//проверка не выгруженна ли страница на HDD
					I = i; J = j;
					minimum_time = _table_times[I][J].second;
					flag = true; break;
				}

		/*поиск минимального времени*/
		for (unsigned i = 0; i < _table_times.size(); i++)
			for (unsigned j = 0; j < _table_times[i].size(); j++)
				if (_table_pages[i][j] >= 0) 								//проверка не выгруженна ли страница на HDD
					if (_table_times[i][j].second < minimum_time) {
						minimum_time = _table_times[i][j].second;
						I = i; J = j;
					}

		/*загрузка страницы с минимальным временем на HDD*/
		int index_old_page = _table_pages[I][J];

		for (int i = 1; i < HDD; i += 4)
			if (_hard_disk_drive[i] == 0) {
				for (int j = 0; j < 4; j++) _hard_disk_drive[i + j] = _random_access_memory[index_old_page + j];
				_table_pages[I][J] = i * (-1);	// Файл хранящийся на HDD отображается в таблице страниц с отрицательным знаком
				break;
			}

		for (int j = 0; j < 4; j++) _random_access_memory[index_old_page + j] = 0;
		return index_old_page;
	}

	/*метод добавления процесса*/
	void push_process(process new_process) {

		int need_pages = new_process.get_pages();					//размер процесса в страницах
		int need_bites = new_process.get_size();					//размер процесса в битах
		vector<int> temp_table_pages;								//временная таблица страниц процесса
		vector <e_chain> temp_table_times;							//временная таблица времен процесса

		/*записываем процесс в RAM, пока есть место*/
		for (unsigned i = 0; i < _random_access_memory.size(); i += 4) {
			if (need_bites) {											//записываем полностью процесс, пока есть свободные страницы								
				if (_random_access_memory[i] == 0) {
					need_pages--; _amount_free_memory -= 4;				//уменьшаем количество свободной памяти и требуемых страниц										
					temp_table_pages.push_back(i);
					temp_table_times.push_back({ { new_process.get_pid(), temp_table_times.size() + 1 }, time(NULL) }); //записываем время добавления страницы

					for (int j = 0; j < 4 && need_bites > 0; j++) {		//заполняем страницу
						_random_access_memory[i + j] = new_process.get_pid();
						need_bites--;
					}
					_count_pages[i / 4] = temp_table_pages.size();
				}
			}
			else break;
		}

		/*если не весь процесс записан, переносим старые страницы с RAM на HDD, и записываем оставшийся процесс в RAM*/
		while (need_bites) {
			int index_old_page = not_frequently_used(); // ищем индекс старой страницу
			temp_table_pages.push_back(index_old_page); //записываем адресс страниц процесса
			temp_table_times.push_back({ { new_process.get_pid() ,temp_table_times.size() + 1 },  time(NULL) }); //записываем время добавления страницы в RAM

																												 //Записываем страницу, в освободившееся место в RAM
			for (int j = 0; j < 4 && need_bites > 0; j++) {
				_random_access_memory[index_old_page + j] = new_process.get_pid();
				if (need_bites == 0) _random_access_memory[index_old_page + j] = 0;
				need_bites--;
			}
			_count_pages[index_old_page / 4] = temp_table_pages.size();
		}
		_table_times.push_back(temp_table_times);	// добавляем в таблицу времен, времена нового процесса
		_table_pages.push_back(temp_table_pages);	// добавляем в таблицу страниц, страницы нового процесса
		_list_processes.push_back(new_process);	// добавляем в таблицу процессов, новый процесс
		_count_process++;							// увеличиваем количество процессов
		display_RAM();								// выводим RAM
	}

	/*метод удаления процесса*/
	int pop_process(int pid) {
		int process = -1;

		/*поиск процесса по pid*/
		for (unsigned i = 0; i < _list_processes.size(); i++) 
			if (_list_processes[i].get_pid() == pid) {
				process = i; break;
			}	

		if (process == -1) {
			display_RAM();
			cout << "Process with the " << pid << " pid does not exist! Please, delete another process." << endl;
			return -1;
		}

		/*удалям процесс из RAM и HDD*/
		for (unsigned i = 0; i < _table_pages[process].size(); i++) {
			int index = _table_pages[process][i];

			if (index >= 0)
				for (int j = 0; j < 4; j++) {
					_random_access_memory[index + j] = 0;
					_amount_free_memory++;
				}
			else {
				index = (index * (-1));
				for (int j = 0; j < 4; j++)
					_hard_disk_drive[index + j] = 0;
			}
		}

		/*удаляем из _list_processes*/
		_list_processes.erase(_list_processes.begin() + process);
		/*удаляем из _table_pages*/
		_table_pages.erase(_table_pages.begin() + process);
		/*удаляем из _table_times*/
		_table_times.erase(_table_times.begin() + process);

		_count_process--;
		display_RAM();
		return 0;
	}

	int page_call(int pid, int page) {
		int process_index = -1;	//индекс вызываемоего процесса

		for (unsigned i = 0; i < _list_processes.size(); i++)
			if (_list_processes[i].get_pid() == pid)
				process_index = i;

		/*проверка на наличии процесса в списке*/
		if (process_index == -1) {
			display_RAM();
			cout << "Process with " << pid << " pid does not exist! Please call the process with another pid." << endl;
			return -1;
		}

		/*проверка на наличие страницы процесса*/
		if (_table_pages[process_index].size() < page) {
			display_RAM();
			cout << "Process with pid " << pid << " does not contain a " << page << " page!" << endl;
			return -1;
		}

		/*меняем время вызова страницы, если она в данный момент в RAM*/
		if (_table_pages[process_index][page - 1] >= 0) {
			_table_times[process_index][page - 1].second = time(NULL);
			display_RAM();
			cout << "The page of " << page << " process with pid " << pid << " is called." << endl;
			return 1;
		}

		/*пишем страницу в свободное место RAM, если она на HDD*/
		if (_amount_free_memory >= 4) {
			int page_index = (-1) * _table_pages[process_index][page - 1];
			for (int i = 0; _random_access_memory.size(); i += 4)
				if (!_random_access_memory[i]) {
					for (int j = 0; j < 4; j++) {
						_random_access_memory[i + j] = _hard_disk_drive[page_index + j];
						_hard_disk_drive[page_index + j] = 0;
					}
					_count_pages[i / 4] = page;
					_table_pages[process_index][page - 1] = i;
					_table_times[process_index][page - 1].second = time(NULL);
					display_RAM();
					cout << "The page of " << page << " process with pid " << pid << " is called." << endl;
					return 1;
				}
		}

		/*если страница на HDD и места в RAM нет, то выгружаем самую старую на HDD и на ёё место записываем вызванную*/
		if (_amount_free_memory < 4) {
			int index_old_page = 0;
			if (_table_pages[process_index][page - 1] < 0) {
				index_old_page = not_frequently_used(); //поиск старой страницы
				int page_index = (-1) * _table_pages[process_index][page - 1];

				for (int j = 0; j < 4; j++) {
					_random_access_memory[index_old_page + j] = _hard_disk_drive[page_index + j];
					_hard_disk_drive[page_index + j] = 0;
				}

				_count_pages[index_old_page / 4] = page;
				_table_pages[process_index][page - 1] = index_old_page;
				_table_times[process_index][page - 1].second = time(NULL);
				display_RAM();
				cout << "The page of " << page << " process with pid " << pid << " is called." << endl;
				return 1;
			}
		}
		return 0;
	}

	/*метод вывода RAM*/
	void display_RAM() {
		cout << "\t\t\t\tRandom Access Memory" << endl;

		/*верхняя часть таблицы*/
		cout << (char)218;
		for (int i = 0; i < 79; i++) cout << (char)196;
		cout << (char)191 << endl << (char)179;

		/*средняя часть таблицы*/
		for (unsigned i = 0; i < _count_pages.size(); i++)
			if (_count_pages[i] < 10) cout << "  " << _count_pages[i] << " " << (char)179;
			else cout << " " << _count_pages[i] << " " << (char)179;
			cout << endl << (char)179;

			for (int i = 0; i < 64; i++) {
				cout << _random_access_memory[i];
				if ((i + 1) % 4 == 0) cout << (char)179;
			}

			/*нижняя часть таблицы*/
			cout << endl << (char)192;
			for (int i = 0; i < 79; i++) cout << (char)196;
			cout << (char)217 << endl;
	}

	/*метод вывода HDD*/
	void display_HDD() {
		cout << "\t\t\tHard Disk Drive" << endl;

		cout << (char)218;
		for (int i = 0; i < 63; i++) cout << (char)196;
		cout << (char)191 << endl;

		for (int i = 1; i < HDD + 1; i++) {
			if ((i - 1) % 32 == 0) cout << (char)179;
			else cout << " " << _hard_disk_drive[i];
			if (i % 32 == 0) cout << " " << (char)179 << endl;
		}

		cout << (char)192;
		for (int i = 0; i < 63; i++) cout << (char)196;
		cout << (char)217 << endl;
	}

	/*метод вывода _list_processes*/
	void display_list_processes() {
		cout << " PID    Name          Size     Pages" << endl;

		for (int i = 0; i < _count_process; i++) {
			cout << " ";

			cout.width(7);
			cout.setf(ios::left);
			cout << get_process(i).get_pid();

			cout.width(14);
			cout.setf(ios::left);
			cout << get_process(i).get_name();

			cout.width(9);
			cout.setf(ios::left);
			cout << get_process(i).get_size();

			cout.width(9);
			cout.setf(ios::left);
			cout << get_process(i).get_pages();

			cout << endl;
		}
	}
};