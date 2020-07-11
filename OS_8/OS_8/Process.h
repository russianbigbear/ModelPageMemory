#include <vector>
using namespace std;

/*
����� "�������"
_pid - ������������� ��������
_size - ������, ���������� ���������, � �����
_pages - ������ �������� � ��������� (���� �������� 4 ����)
_name - ��� ��������
*/
class process {
private:
	int _pid;
	int _size;
	int _pages;
	string _name;
	
public:
	/*����������� ��� ���������*/
	process() {
		_size = 1;
		_pid = 228;
		_name = "name";
	}

	/*����������� � ����������*/
	process(int pid, int size, string name) {
		set_pid(pid);
		set_size(size);
		set_name(name);
		set_pages(size);
	}

	/*������������� � ����������*/
	void init(int pid, int size, string name) {
		set_pid(pid);
		set_size(size);
		set_name(name);
		set_pages(size);
	}

	/*����� ������������ _pid*/
	void set_pid(int pid) { _pid = pid; }

	/*����� ������������ _size*/
	void set_size(int size) { _size = size; }

	/*����� ������������ _name*/
	void set_name(string name) { _name = name; }

	/*����� �������� _need_�ount_pages*/
	void set_pages(int size) {
		_pages = size / 4;
		if (size % 4 > 0) _pages++;
	}

	/*����� ������ _pid*/
	int get_pid() { return _pid; }

	/*����� ������ _size*/
	int get_size() { return _size; }

	/*����� ������ _need_�ount_pages*/
	int get_pages() { return _pages; }

	/*����� ������ _name*/
	string get_name() { return _name; }
};
