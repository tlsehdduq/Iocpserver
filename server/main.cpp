#include"pch.h"
#include"iocp.h"
#include"DB.h"
extern DB Gdatabase;

int main()
{
	auto& instance = Iocp::GetInstance();
	if (instance.networkSet() == false)
		cout << " network err " << endl;

	if (instance.acceptStart() == false)
		cout << " acceptstart err " << endl;

	int num_thread = thread::hardware_concurrency();

	for (int i = 0; i < num_thread; ++i)
	{
		instance._threads.emplace_back(&Iocp::dispatch,&instance);
	}

	for (auto& thread : instance._threads)
	{
		if (thread.joinable())
			thread.join();
	}

}