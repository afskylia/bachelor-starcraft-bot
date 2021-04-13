#pragma once

class WorkerManager
{
public:
	WorkerManager();
	void sendIdleWorkersToMinerals();
	void trainAdditionalWorkers();
	void sendScout();
	void onFrame();
};
