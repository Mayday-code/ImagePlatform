#include "Camera.h"

#include <iostream>

bool Camera::isCapturing() 
{
	std::lock_guard<std::mutex> lck(m_stopLock);
	return !m_stop;
}

void Camera::stopSequenceAcquisition()
{
	std::unique_lock<std::mutex> lck(m_stopLock);
	m_stop = true;
	lck.unlock();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "StopSequenceAcquisition..." << std::endl;
}