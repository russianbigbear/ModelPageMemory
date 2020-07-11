#include <string>
#include <cstdio>
#include <windows.h>
#include <algorithm>
#include "paging_model.h"

/*функция рандомной генерации pids*/
void shuffle_pids(vector<int>& pids) {
	srand(unsigned(time(0)));
	random_shuffle(pids.begin(), pids.end());
	random_shuffle(pids.begin(), pids.end());
}

/*функция отметки использованных PID'ов*/
int used_pids(vector<int> pids, vector<int>& flag_pids) {
	for(int i =0; i < 9; i++)
		if (!flag_pids[i]) {
			flag_pids[i] = 1;
			return pids[i];
		}
	 return 0;
}

/*функция снятия пометки с использованных PID'ов*/
void not_used_pids(vector<int> pids, vector<int>& flag_pids, int pid) {
	for (int i = 0; i < 9; i++)
		if (pids[i] == pid)
			flag_pids[i] = 0;
}

int main() {
	paging_model programm;	// главная модель виртуальной памяти
	vector<int> pids;		// вектор PID'ов
	vector<int> flag_pids; //вектор использованных PID'ов  
	for (int i = 0; i < 9; i++) {
		pids.push_back(i + 1);
		flag_pids.push_back(0);
	}
	shuffle_pids(pids);

	process proc;
	proc.init(used_pids(pids, flag_pids), 11, "Aimp.exe");
	programm.push_process(proc);

	system("cls");
	proc.init(used_pids(pids, flag_pids), 3, "process.exe");
	programm.push_process(proc);
	programm.display_list_processes();

	system("cls");
	proc.init(used_pids(pids, flag_pids), 20, "google.exe");
	programm.push_process(proc);
	programm.display_list_processes();

	while (1) {
		string str;
		cout << endl;
		cin >> str;
	
		/*добавление процесса*/
		if (str == "push") {
			int size;
			string name;
			cin >> name >> size;
			name += ".exe";
			system("cls");

			if (size > 64) {
				programm.display_RAM();
				programm.display_list_processes();
				cout << "Size more 64 bit's!" << endl;
			}
			else {
				if (programm.get_count_process() < 9) {
					proc.init(used_pids(pids, flag_pids), size, name);
					programm.push_process(proc);
					programm.display_list_processes();
				}
				else {
					programm.display_RAM();
					programm.display_list_processes();
					cout << "Process list is full! Please, delete process.";
				}
			}
		}

		/*удаление процесса*/
		if (str == "pop") {
			int pid;
			cin >> pid;
			system("cls");

			programm.pop_process(pid);
			not_used_pids(pids, flag_pids, pid);
			programm.display_list_processes();

		}

		if (str == "call") {
			int pid, page;
			cin >> pid >> page;
			system("cls");

			programm.page_call(pid, page);
			programm.display_list_processes();
		}

		/*вывод HDD*/
		if (str == "HDD") 
			programm.display_HDD();
		
		/*вывод подсказки*/
		if (str == "?") {
			cout << "push NAME SIZE  -  adds a process;" << endl;
			cout << "pop PID   -  removes the process;" << endl;
			cout << "call PID PAGE   - calls PAGE of the process that has PID;" << endl;
			cout << "HDD   -  shows the hard drive;" << endl;
			cout << "close   -  close the programm." << endl;
		}

		if (str == "close")
			exit(0);
	}

	return 0;
}



