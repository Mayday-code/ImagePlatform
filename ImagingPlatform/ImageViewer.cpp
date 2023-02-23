#include "ImageViewer.h"

#include <QPainter>
#include <iostream>

void ImageViewer::setPixmap(const QPixmap& rhs)
{
	std::lock_guard<std::mutex> lck(updateLock);
	m_pixmap = rhs;
#ifndef _DEBUG
	std::cout << "get QPixmap : " << m_pixmap.height() << ' ' << m_pixmap.width() << std::endl;
#endif // !_DEBUG
}

QRectF ImageViewer::boundingRect() const
{
	return QRectF(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap.width(), m_pixmap.height());
}

void ImageViewer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	std::lock_guard<std::mutex> lck(updateLock);
#ifndef _DEBUG
	static int index = 0;
	std::cout << "painting!!!  " << index++ << std::endl;
#endif // !_DEBUG

	painter->drawPixmap(-m_pixmap.width() / 2, -m_pixmap.height() / 2, m_pixmap);
}