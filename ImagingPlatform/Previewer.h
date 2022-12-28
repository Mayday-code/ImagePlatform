#pragma once

#include "PreviewItem.h"

#include <QWidget>
#include <QGridLayout>

/*!
 * \class Previewer
 * \brief Realizing the function of previewing multiple fields of view.
 *
 * \author XYH
 * \date 12 2022
 */
class Previewer : public QWidget
{
	Q_OBJECT

signals:
	void moveTo(const QPoint& point);

public:
	Previewer(QWidget *parent);
	~Previewer();

	void addPreviewItem(PreviewItem* item, int r, int c);

private:
	QGridLayout* layout;
};
