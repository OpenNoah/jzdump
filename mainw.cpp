#include <iostream>
#include <opie2/oapplicationfactory.h>
#include <opie2/oapplication.h>
#include <qt.h>
#include "mainw.h"

#if 1
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	std::cout << a.desktop() << std::endl;
	QWidget ww(0, "top");
	a.setMainWidget(&ww);
	ww.show();
	return a.exec();
#if 0
	Opie::Core::OApplication a(argc, argv);
	a.setPalette(QPalette());
	std::cout << a.desktop() << std::endl;
	//std::cout << std::hex << Qt::WType_Desktop << ", " << std::hex << Qt::WType_TopLevel << std::endl;
	//QWidget *dw = new QWidget( 0, "desktop", Qt::WType_Desktop|Qt::WType_TopLevel );
	//std::cout << dw->parentWidget() << std::endl;
	//a.desktop();
	//QWidget *dw = new QWidget( 0, "desktop", Qt::WType_Desktop|Qt::WType_TopLevel );
	//std::cout << a.palette().active().background().name() << std::endl;
	//QPEApplication a(argc, argv);
	//QWidget ww( 0, "desktop", Qt::WType_Desktop|Qt::WType_TopLevel );
	NMQMainWindow w(0, "example", Qt::WType_Desktop|Qt::WType_TopLevel);//, Qt::WType_Desktop);
	w.update();
	//std::cout << w.palette().normal().background().name() << std::endl;
	//std::cout << w.isTopLevel() << std::endl;
	//std::cout << w.isDesktop() << std::endl;
	//a.setMainWidget(&w);
	//w.showMaximized();
	//MainWindow w;
	w.show();
	return a.exec();
#endif
}
#else
//OPIE_EXPORT_APP(Opie::Core::OApplicationFactory<MainWindow>)
OPIE_EXPORT_APP(Opie::Core::OApplicationFactory<NMQMainWindow>)
#endif

#if 1
MainWindow::MainWindow(QWidget *parent, const char *name, WFlags flags)
	: NMQMainWindow(parent, name, flags)
{
	setCaption(tr("Example MainWindow"));
}

MainWindow::~MainWindow()
{
}
#endif
