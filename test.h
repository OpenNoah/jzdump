#pragma once

#include <qt.h>
#include <stdint.h>

extern struct ui_t {
	QWidget *top;
	QLabel *status;
	QProgressBar *progress;
	QLabel *gpio[4][4];

	void updateStatus(QString s)
	{
		status->setText(s);
		qApp->processEvents(0);
	}
} ui;

class Test : public QObject
{
	Q_OBJECT
public:
	Test(QObject *parent);
	virtual ~Test();
	bool failed() {return _failed;}

protected:
	virtual void timerEvent(QTimerEvent *e);

private:
	void init();
	void uninit();
	void runTest();
	void updateGPIO();
	char charGPIO(uint32_t idx, uint32_t fun, uint32_t sel, uint32_t dir, uint32_t dat, uint32_t pin);

	bool _failed;
	int _tGPIO, _tTest;
	uint32_t _test;
};
