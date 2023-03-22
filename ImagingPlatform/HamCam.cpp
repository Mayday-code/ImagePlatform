#include "HamCam.h"
#include "dcamprop.h"

HamCam::HamCam()
{
	if (!init()) return;
	if (!open()) return;

	m_state = CameraState::ONLINE;

	//初始设置要与界面的显示同步
	setROI(0, 0, 2048, 2048);
	setExposure(100);

	m_pixDepth = 2;
	m_channel = 1;

	m_cbuf.initialize(m_channel, m_width, m_height, m_pixDepth);

	m_hdcam = m_devopen.hdcam;

	std::cout << "相机连接成功" << std::endl;
}

HamCam::~HamCam()
{
	stopSequenceAcquisition();
	dcamdev_close(m_hdcam);
	printf("与相机断开连接\n");
	dcamapi_uninit();
}


bool HamCam::init()
{
	DCAMAPI_INIT apiinit;
	memset(&apiinit, 0, sizeof(apiinit));
	apiinit.size = sizeof(apiinit);
	DCAMERR err = dcamapi_init(&apiinit);
	if (failed(err)) {
		printf("初始化出错!!!错误代码: %x\n", err);
		return false;
	}
	printf("检测到 %d 台相机\n", apiinit.iDeviceCount);
	return true;
}

bool HamCam::open()
{
	memset(&m_devopen, 0, sizeof(m_devopen));
	m_devopen.size = sizeof(m_devopen);
	m_devopen.index = 0;
	DCAMERR err = dcamdev_open(&m_devopen);
	if (failed(err)) {
		printf("打开相机出错!!!错误代码: %x\n", err);
		return false;
	}
	return true;
}

bool HamCam::startCapturing()
{
	DCAMERR err;
	memset(&m_waitopen, 0, sizeof(m_waitopen));
	m_waitopen.size = sizeof(m_waitopen);
	m_waitopen.hdcam = m_hdcam;
	err = dcamwait_open(&m_waitopen);
	if (failed(err)) {
		printf("wait_open出错!!!错误代码: %x\n", err);
		return;
	}
	m_hwait = m_waitopen.hwait;

	err = dcambuf_release(m_hdcam);
	if (failed(err)) {
		printf("释放缓冲区出错!!!错误代码: %x\n", err);
		return;
	}

	err = dcambuf_alloc(m_hdcam, 10);
	if (failed(err)) {
		printf("分配缓冲区出错!!!错误代码: %x\n", err);
		return;
	}

	err = dcamcap_start(m_hdcam, DCAMCAP_START_SEQUENCE);
	if (failed(err)) {
		printf("无法启动采集!!!错误代码: %x\n", err);
		return;
	}

	//waitstart
	memset(&m_waitstart, 0, sizeof(m_waitstart));
	m_waitstart.size = sizeof(m_waitstart);
	m_waitstart.eventmask = DCAMWAIT_CAPEVENT_FRAMEREADY;
	m_waitstart.timeout = 500;

	//bufframe
	memset(&m_bufframe, 0, sizeof(m_bufframe));
	m_bufframe.size = sizeof(m_bufframe);
	m_bufframe.iFrame = -1;

	std::lock_guard<std::mutex> lck(m_stateMutex);
	m_state = CameraState::LIVING;

	//新开线程采集数据
	std::thread thread_capture([this]() {
		DCAMERR err;
		while (isCapturing()) {
			//等待图像
			err = dcamwait_start(m_hwait, &m_waitstart);
			if (failed(err)) {
				printf("wait_start出错!!!错误代码: %x\n", err);
				//return 0;
			}

			//访问图像
			err = dcambuf_lockframe(m_hdcam, &m_bufframe);
			if (failed(err)) {
				printf("buf_lockframe出错!!!错误代码: %x\n", err);
				//return 0;
			}

			//往buffer写入数据,需要flip数据吗
			this->m_cbuf.insertImage((unsigned char*)m_bufframe.buf,
				static_cast<int>(m_bufframe.width), 
				static_cast<int>(m_bufframe.height));

		}
		std::cout << "退出采图! " << std::endl;
		stopCap();
	});
	thread_capture.detach();
	return true;
}

bool HamCam::setDeviceExp(double exp_ms)
{
	m_exp = exp_ms / 1000;
	//滨松相机提供的api需要将参数设置为以μm为单位
	DCAMERR err = dcamprop_setgetvalue(m_hdcam, DCAM_IDPROP_EXPOSURETIME, &m_exp);
	if (failed(err)) {
		std::cout << "设置曝光失败" << std::endl;
		return false;
	}
	m_exp *= 1000;
	return true;
}

bool HamCam::setDeviceROI(unsigned hPos, unsigned vPos, unsigned hSize, unsigned vSize)
{
	double tmp_hPos = hPos;
	double tmp_vPos = vPos;
	double tmp_width = hSize;
	double tmp_height = vSize;

	DCAMERR err;
	bool isOk = true;
	err = dcamprop_setvalue(m_hdcam, DCAM_IDPROP_SUBARRAYMODE, DCAMPROP_MODE__OFF);
	if (failed(err)) { isOk = false; }

	err = dcamprop_setgetvalue(m_hdcam, DCAM_IDPROP_SUBARRAYHPOS, &tmp_hPos);
	if (failed(err)) { isOk = false; }

	err = dcamprop_setgetvalue(m_hdcam, DCAM_IDPROP_SUBARRAYVPOS, &tmp_vPos);
	if (failed(err)) { isOk = false; }

	err = dcamprop_setgetvalue(m_hdcam, DCAM_IDPROP_SUBARRAYHSIZE, &tmp_width);
	if (failed(err)) { isOk = false; }

	err = dcamprop_setgetvalue(m_hdcam, DCAM_IDPROP_SUBARRAYVSIZE, &tmp_height);
	if (failed(err)) { isOk = false; }

	err = dcamprop_setvalue(m_hdcam, DCAM_IDPROP_SUBARRAYMODE, DCAMPROP_MODE__ON);
	if (failed(err)) { isOk = false; }

	m_hPos = tmp_hPos;
	m_vPos = tmp_vPos;
	m_width = tmp_width;
	m_height = tmp_height;

	if (!isOk) {
		printf("设置ROI出错!!!错误代码: %x\n", err);
		return;
	}

	return true;
}

void HamCam::stopCap()
{
	dcamcap_stop(m_hdcam);
	dcambuf_release(m_hdcam);
	dcamwait_abort(m_hwait);
}