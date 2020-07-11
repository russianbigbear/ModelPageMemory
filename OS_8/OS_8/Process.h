#include <vector>
using namespace std;

/*
Класс "Процесс"
_pid - идентификатор процесса
_size - размер, занимаемый процессом, в битах
_pages - размер процесса в страницах (одна страница 4 бита)
_name - имя процесса
*/
class process {
private:
	int _pid;
	int _size;
	int _pages;
	string _name;
	
public:
	/*конструктор без параметра*/
	process() {
		_size = 1;
		_pid = 228;
		_name = "name";
	}

	/*конструктор с параметром*/
	process(int pid, int size, string name) {
		set_pid(pid);
		set_size(size);
		set_name(name);
		set_pages(size);
	}

	/*инициализатор с параметром*/
	void init(int pid, int size, string name) {
		set_pid(pid);
		set_size(size);
		set_name(name);
		set_pages(size);
	}

	/*метод присваивания _pid*/
	void set_pid(int pid) { _pid = pid; }

	/*метод присваивания _size*/
	void set_size(int size) { _size = size; }

	/*метод присваивания _name*/
	void set_name(string name) { _name = name; }

	/*метод подсчёта _need_сount_pages*/
	void set_pages(int size) {
		_pages = size / 4;
		if (size % 4 > 0) _pages++;
	}

	/*метод вывода _pid*/
	int get_pid() { return _pid; }

	/*метод вывода _size*/
	int get_size() { return _size; }

	/*метод вывода _need_сount_pages*/
	int get_pages() { return _pages; }

	/*метод вывода _name*/
	string get_name() { return _name; }
};
