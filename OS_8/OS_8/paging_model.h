#include <ctime>
#include <iostream>
#include "Process.h"

#define e_chain pair<pair<int, int>, time_t>  // ������� ������� { { pid, page}, time}
#define HDD 512

/*
����� "������ ����������� ���������� ������"
_count_process - ���������� ���������
_amount_free_memory - ���������� ��������� RAM
_count_pages - ���������� ������� ������� ��������� � RAM
_list_processes - ������� ���������
_table_pages - ������� ������� ��������
_table_times - �������, ���������� pid ��������, ���������� ��� ������� � ����� ���������� �� � RAM
_random_access_memory - ������, �������������� RAM
_hard_disk_drive - ������, �������������� HDD
_chain - ������� ��������� �� �������
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
	/*����������� ��� ���������, �������������*/
	paging_model() {
		_count_process = 0;  // ��������� �������� ���������� ���������
		_amount_free_memory = 64; //��������� �������� ��������� ����������� ������
		for (int i = 0; i < HDD + 1; i++) _hard_disk_drive.push_back(0); //��������� ������ ��� HDD
		for (int i = 0; i < 16; i++) _count_pages.push_back(0);
		for (int i = 0; i < 16 * 4; i++) _random_access_memory.push_back(0); //
		for (int i = 0; i < 64 / 4; i++) _chain.push_back({ { 0, 0 }, 0 });
	}

	/*����� ������ _count_process*/
	int get_count_process() { return _count_process; }

	/*����� ������ �������� ������ "�������"*/
	process get_process(int number) { return _list_processes[number]; }

	/*����� ������ ������ ��������(�������� � ���������� ��������)*/
	int not_frequently_used() {
		int I, J;
		time_t minimum_time;

		/*��������, �� ��������� �� ������ ������ �������� �� HDD*/
		bool flag = false;
		for (unsigned i = 0; i < _table_times.size() && !flag; i++)
			for (unsigned j = 0; j < _table_times[i].size() && !flag; j++)
				if (_table_pages[i][j] >= 0) {								//�������� �� ���������� �� �������� �� HDD
					I = i; J = j;
					minimum_time = _table_times[I][J].second;
					flag = true; break;
				}

		/*����� ������������ �������*/
		for (unsigned i = 0; i < _table_times.size(); i++)
			for (unsigned j = 0; j < _table_times[i].size(); j++)
				if (_table_pages[i][j] >= 0) 								//�������� �� ���������� �� �������� �� HDD
					if (_table_times[i][j].second < minimum_time) {
						minimum_time = _table_times[i][j].second;
						I = i; J = j;
					}

		/*�������� �������� � ����������� �������� �� HDD*/
		int index_old_page = _table_pages[I][J];

		for (int i = 1; i < HDD; i += 4)
			if (_hard_disk_drive[i] == 0) {
				for (int j = 0; j < 4; j++) _hard_disk_drive[i + j] = _random_access_memory[index_old_page + j];
				_table_pages[I][J] = i * (-1);	// ���� ���������� �� HDD ������������ � ������� ������� � ������������� ������
				break;
			}

		for (int j = 0; j < 4; j++) _random_access_memory[index_old_page + j] = 0;
		return index_old_page;
	}

	/*����� ���������� ��������*/
	void push_process(process new_process) {

		int need_pages = new_process.get_pages();					//������ �������� � ���������
		int need_bites = new_process.get_size();					//������ �������� � �����
		vector<int> temp_table_pages;								//��������� ������� ������� ��������
		vector <e_chain> temp_table_times;							//��������� ������� ������ ��������

		/*���������� ������� � RAM, ���� ���� �����*/
		for (unsigned i = 0; i < _random_access_memory.size(); i += 4) {
			if (need_bites) {											//���������� ��������� �������, ���� ���� ��������� ��������								
				if (_random_access_memory[i] == 0) {
					need_pages--; _amount_free_memory -= 4;				//��������� ���������� ��������� ������ � ��������� �������										
					temp_table_pages.push_back(i);
					temp_table_times.push_back({ { new_process.get_pid(), temp_table_times.size() + 1 }, time(NULL) }); //���������� ����� ���������� ��������

					for (int j = 0; j < 4 && need_bites > 0; j++) {		//��������� ��������
						_random_access_memory[i + j] = new_process.get_pid();
						need_bites--;
					}
					_count_pages[i / 4] = temp_table_pages.size();
				}
			}
			else break;
		}

		/*���� �� ���� ������� �������, ��������� ������ �������� � RAM �� HDD, � ���������� ���������� ������� � RAM*/
		while (need_bites) {
			int index_old_page = not_frequently_used(); // ���� ������ ������ ��������
			temp_table_pages.push_back(index_old_page); //���������� ������ ������� ��������
			temp_table_times.push_back({ { new_process.get_pid() ,temp_table_times.size() + 1 },  time(NULL) }); //���������� ����� ���������� �������� � RAM

																												 //���������� ��������, � �������������� ����� � RAM
			for (int j = 0; j < 4 && need_bites > 0; j++) {
				_random_access_memory[index_old_page + j] = new_process.get_pid();
				if (need_bites == 0) _random_access_memory[index_old_page + j] = 0;
				need_bites--;
			}
			_count_pages[index_old_page / 4] = temp_table_pages.size();
		}
		_table_times.push_back(temp_table_times);	// ��������� � ������� ������, ������� ������ ��������
		_table_pages.push_back(temp_table_pages);	// ��������� � ������� �������, �������� ������ ��������
		_list_processes.push_back(new_process);	// ��������� � ������� ���������, ����� �������
		_count_process++;							// ����������� ���������� ���������
		display_RAM();								// ������� RAM
	}

	/*����� �������� ��������*/
	int pop_process(int pid) {
		int process = -1;

		/*����� �������� �� pid*/
		for (unsigned i = 0; i < _list_processes.size(); i++) 
			if (_list_processes[i].get_pid() == pid) {
				process = i; break;
			}	

		if (process == -1) {
			display_RAM();
			cout << "Process with the " << pid << " pid does not exist! Please, delete another process." << endl;
			return -1;
		}

		/*������ ������� �� RAM � HDD*/
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

		/*������� �� _list_processes*/
		_list_processes.erase(_list_processes.begin() + process);
		/*������� �� _table_pages*/
		_table_pages.erase(_table_pages.begin() + process);
		/*������� �� _table_times*/
		_table_times.erase(_table_times.begin() + process);

		_count_process--;
		display_RAM();
		return 0;
	}

	int page_call(int pid, int page) {
		int process_index = -1;	//������ ������������ ��������

		for (unsigned i = 0; i < _list_processes.size(); i++)
			if (_list_processes[i].get_pid() == pid)
				process_index = i;

		/*�������� �� ������� �������� � ������*/
		if (process_index == -1) {
			display_RAM();
			cout << "Process with " << pid << " pid does not exist! Please call the process with another pid." << endl;
			return -1;
		}

		/*�������� �� ������� �������� ��������*/
		if (_table_pages[process_index].size() < page) {
			display_RAM();
			cout << "Process with pid " << pid << " does not contain a " << page << " page!" << endl;
			return -1;
		}

		/*������ ����� ������ ��������, ���� ��� � ������ ������ � RAM*/
		if (_table_pages[process_index][page - 1] >= 0) {
			_table_times[process_index][page - 1].second = time(NULL);
			display_RAM();
			cout << "The page of " << page << " process with pid " << pid << " is called." << endl;
			return 1;
		}

		/*����� �������� � ��������� ����� RAM, ���� ��� �� HDD*/
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

		/*���� �������� �� HDD � ����� � RAM ���, �� ��������� ����� ������ �� HDD � �� �� ����� ���������� ���������*/
		if (_amount_free_memory < 4) {
			int index_old_page = 0;
			if (_table_pages[process_index][page - 1] < 0) {
				index_old_page = not_frequently_used(); //����� ������ ��������
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

	/*����� ������ RAM*/
	void display_RAM() {
		cout << "\t\t\t\tRandom Access Memory" << endl;

		/*������� ����� �������*/
		cout << (char)218;
		for (int i = 0; i < 79; i++) cout << (char)196;
		cout << (char)191 << endl << (char)179;

		/*������� ����� �������*/
		for (unsigned i = 0; i < _count_pages.size(); i++)
			if (_count_pages[i] < 10) cout << "  " << _count_pages[i] << " " << (char)179;
			else cout << " " << _count_pages[i] << " " << (char)179;
			cout << endl << (char)179;

			for (int i = 0; i < 64; i++) {
				cout << _random_access_memory[i];
				if ((i + 1) % 4 == 0) cout << (char)179;
			}

			/*������ ����� �������*/
			cout << endl << (char)192;
			for (int i = 0; i < 79; i++) cout << (char)196;
			cout << (char)217 << endl;
	}

	/*����� ������ HDD*/
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

	/*����� ������ _list_processes*/
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