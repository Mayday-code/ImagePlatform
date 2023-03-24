#pragma once

// 单个预览图高度
const int PREVIEWHEIGHT = 70;

// 初始实时预览图高度
const int FOVHEIGHT = 600;

enum class CameraState
{
	OFFLINE = 1,
	ONLINE = 2,
	LIVING = 3,
};

enum class StageState
{
	OFFLINE = 1,
	ONLINE = 2,
};