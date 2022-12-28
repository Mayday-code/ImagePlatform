#pragma once

#include <QLabel>
#include <QMouseEvent>

/*!
 * \class PreviewItem
 * \brief Containing a field of view and corresponding stage coordinate.
 *
 * \author XYH
 * \date 12 2022
 */
class PreviewItem : public QLabel
{
	Q_OBJECT
public:
	PreviewItem(const QPoint& t_point, QWidget* parent = nullptr) : m_point(t_point), QLabel(parent) { }

signals:
	void mouseClicked(const QPoint& point);

protected:
	void mousePressEvent(QMouseEvent *ev) override
	{
		if (ev->button() == Qt::LeftButton) {
			emit mouseClicked(m_point);
		}
		QLabel::mousePressEvent(ev);
	}

private:
	QPoint m_point;       // corresponding stage coordiante of this field of view
};
