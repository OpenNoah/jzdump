#ifdef NOAH_MOBILE
#include <noahmobile/nmqmainwindow.h>
#include <noahmobile/nmqmessagebox.h>
#include <noahmobile/nmqwidget.h>
#include <noahmobile/nmqdialog.h>
#include <opie2/oapplication.h>
#else
#if 1
#define NMQMainWindow	QMainWindow
#include <qt.h>
#else
#define NMQMainWindow	QMainWindow
#include <qpe/qpeapplication.h>
#endif
#endif
#include <qt.h>
#include "test.h"

#define TR(s)	QObject::tr(s)

struct ui_t ui;

void setupUI(QWidget *top)
{
	ui.top = top;

	QVBoxLayout *vl = new QVBoxLayout(top);
	vl->setSpacing(4);
	vl->setMargin(4);

	vl->addWidget(ui.status = new QLabel(TR("正在初始化..."), top));
	vl->addStretch();

	QHBoxLayout *hl = new QHBoxLayout;
	hl->setSpacing(4);
	hl->setMargin(4);
	vl->addLayout(hl);

	QGroupBox *gb = new QGroupBox(5, Qt::Horizontal, TR("GPIO"), top);
	gb->setFont(QFont("Simsun", 13));
	hl->addWidget(gb);

	for (int ip = 0; ip < 4; ip++) {
		new QLabel(TR("Port %1").arg((char)('A' + ip)), gb);
		for (int i = 0; i < 4; i++)
			ui.gpio[ip][i] = new QLabel("01234567", gb);
	}

	vl->addWidget(ui.progress = new QProgressBar(top));
	ui.progress->setCenterIndicator(true);
	ui.progress->setTotalSteps(100);
	ui.progress->setProgress(39);
}

int main(int argc, char *argv[])
{
#ifdef NOAH_MOBILE
	Opie::Core::OApplication a(argc, argv);
#elif 1
	QApplication a(argc, argv);
#else
	QPEApplication a(argc, argv);
#endif
	a.setFont(QFont("noah", 12));
	QTextCodec *codec = QTextCodec::codecForName("UTF8");
	a.setDefaultCodec(codec);

	NMQMainWindow w(0, "jzdump", Qt::WType_TopLevel);
	w.setCaption(TR("jzdump"));
	w.setCentralWidget(new QWidget(&w));
	setupUI(w.centralWidget());
	a.setMainWidget(&w);
	w.showMaximized();
	//w.showFullScreen();

	Test *t = new Test(&w);
	if (t->failed())
		return 1;
	return a.exec();
}
