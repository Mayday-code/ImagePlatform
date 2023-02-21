#include "Camera.h"

#include <iostream>

using namespace std::literals;

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
	std::this_thread::sleep_for(300ms);
	std::cout << "StopSequenceAcquisition..." << std::endl;
}