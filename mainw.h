#pragma once

#include <noahmobile/nmqmainwindow.h>
#include <noahmobile/nmqwidget.h>

class MainWindow: public NMQMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0, const char *name = 0, WFlags flags = 0);
	~MainWindow();

	static QString appName() {return tr("example");}
};
