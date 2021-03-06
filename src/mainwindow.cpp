#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    flags  = windowFlags();
    setWindowFlags(flags ^ Qt::FramelessWindowHint);
    setSizePolicy(
                QSizePolicy(
                    QSizePolicy::MinimumExpanding,
                    QSizePolicy::MinimumExpanding));
    setFocusPolicy(Qt::WheelFocus);
    setFocus(Qt::MouseFocusReason);
    setContentsMargins(0, 0, 0, 5);
    setWindowTitle("CPU Frequence Utility");
    toolBar = new ToolBar(this);
    addToolBar(toolBar);
    baseLayout = NULL;
    baseWdg = NULL;
    scrolled = NULL;
    CPU_COUNT = 0;
    timerID = 0;
    initTrayIcon();
    readSettings();
    connect(toolBar->firstForAll, SIGNAL(toggled(bool)),
            this, SLOT(setFirstForAll(bool)));
    connect(toolBar->reload, SIGNAL(released()),
            this, SLOT(reloadCPUItems()));
    connect(toolBar->apply, SIGNAL(released()),
            this, SLOT(applyChanges()));
    connect(toolBar->exit, SIGNAL(released()),
            this, SLOT(close()));
    connect(toolBar->resize, SIGNAL(toggled(bool)),
            this, SLOT(resizeApp(bool)));
    if ( toolBar->getShowAtStartState() ) {
        _show();
    } else {
        timerID = startTimer(3);
    };
}

void MainWindow::initTrayIcon()
{
    trayIcon = new TrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon->hideAction, SIGNAL(triggered(bool)),
            this, SLOT(changeVisibility()));
    connect(trayIcon->closeAction, SIGNAL(triggered(bool)),
            this, SLOT(close()));
    trayIcon->show();
}

void MainWindow::initCPU_Items(QStringList &cpus)
{
    if (baseLayout!=NULL) {
        for (int i=0; i<baseLayout->count(); i++) {
            CPU_Item *wdg = static_cast<CPU_Item*>(
                        baseLayout->itemAt(i)->widget());
            if ( NULL!=wdg && i==0 ) {
                disconnect(wdg, SIGNAL(curr_gov(QString&)),
                           this, SLOT(receiveCurrGovernor(QString&)));
                disconnect(wdg, SIGNAL(max_freq(QString&)),
                           this, SLOT(receiveCurrMaxFreq(QString&)));
                disconnect(wdg, SIGNAL(min_freq(QString&)),
                           this, SLOT(receiveCurrMinFreq(QString&)));
            };
        };
        delete baseLayout;
        baseLayout = NULL;
    };
    if (baseWdg!=NULL) {
        delete baseWdg;
        baseWdg = NULL;
    };
    if (scrolled!=NULL) {
        delete scrolled;
        scrolled = NULL;
    };
    CPU_COUNT = cpus.count();
    baseLayout = new QVBoxLayout();
    baseWdg = new QWidget(this);
    foreach (QString cpuNum, cpus) {
        CPU_Item *wdg = new CPU_Item(this, cpuNum);
        baseLayout->addWidget(wdg);
        if ( cpuNum=="0" ) {
            connect(wdg, SIGNAL(curr_gov(QString&)),
                    this, SLOT(receiveCurrGovernor(QString&)));
            connect(wdg, SIGNAL(max_freq(QString&)),
                    this, SLOT(receiveCurrMaxFreq(QString&)));
            connect(wdg, SIGNAL(min_freq(QString&)),
                    this, SLOT(receiveCurrMinFreq(QString&)));
            QString gov_Icon =
                    wdg->getGovernor();
            if (gov_Icon=="undefined") gov_Icon="dialog-error";
            else if (gov_Icon=="userspace") gov_Icon = "user-idle";
            trayIcon->setIcon(
                        QIcon::fromTheme(
                            gov_Icon,
                            QIcon(QString(":/%1.png")
                                  .arg(gov_Icon))));
        };
    };
    baseWdg->setLayout(baseLayout);
    scrolled = new QScrollArea(this);
    scrolled->setWidget(baseWdg);
    setCentralWidget(scrolled);
    setFirstForAll(toolBar->getFirstForAllState());
}

void MainWindow::_hide()
{
    this->hide();
    trayIcon->hideAction->setText (QString("Up"));
    trayIcon->hideAction->setIcon (QIcon::fromTheme("go-up"));
    if ( timerID ) {
        killTimer(timerID);
        timerID = 0;
    }
}

void MainWindow::_show()
{
    this->show();
    trayIcon->hideAction->setText (QString("Down"));
    trayIcon->hideAction->setIcon (QIcon::fromTheme("go-down"));
    if ( timerID==0 ) {
        timerID = startTimer(25000);
    }
}

void MainWindow::changeVisibility()
{
    (this->isVisible()) ? _hide() : _show();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason r)
{
    if (r==QSystemTrayIcon::Trigger) changeVisibility();
}

void MainWindow::onResult(ExecuteJob *job)
{
    if (NULL==job) {
        KNotification::event(
                    KNotification::Notification,
                    "CPUFreqUtility",
                    QString("Processing with CPU data failed."),
                    this);
        return;
    };
    if (job->error()) {
        KNotification::event(
                    KNotification::Notification,
                    "CPUFreqUtility",
                    QString("ERROR: %1\n%2")
                    .arg(job->error()).arg(job->errorText()),
                    this);
    } else if ( job->data().keys().contains("filename") ) {
        if ( job->data().value("filename")=="present" ) {
            // possible format: 0-3,5-7,12,14-31
            QStringList cpus, seq;
            seq = job->data()
                    .value("contents").toString()
                    .replace("\n", "").split(",");
            foreach (QString _cpu, seq) {
                if ( _cpu.contains("-") ) {
                    QStringList _seq = _cpu.split("-");
                    int _first, _last;
                    _first = _seq.first().toInt();
                    _last  = _seq.last().toInt();
                    while (_first<=_last) {
                        cpus.append(QString::number(_first));
                        _first++;
                    };
                } else
                    cpus.append(_cpu);
            };
            initCPU_Items(cpus);
        };
    };
}

void MainWindow::readCPUCount()
{
    QVariantMap args;
    args["procnumb"] = "null";
    args["filename"] = "present";

    Action act("org.freedesktop.auth.cpufrequtility.read");
    act.setHelperId("org.freedesktop.auth.cpufrequtility");
    act.setArguments(args);
    ExecuteJob *job = act.execute();
    job->setAutoDelete(true);
    if (job->exec()) {
        onResult(job);
    } else {
        KNotification::event(
                    KNotification::Notification,
                    "CPUFreqUtility",
                    QString("ERROR: %1\n%2")
                    .arg(job->error()).arg(job->errorText()),
                    this);
    };
}

void MainWindow::setFirstForAll(bool state)
{
    for (int i=0; i<baseLayout->count(); i++) {
        CPU_Item *wdg = static_cast<CPU_Item*>(
                    baseLayout->itemAt(i)->widget());
        if ( NULL!=wdg )
            wdg->setFirstForAllState(state);
    };
}

void MainWindow::reloadCPUItems()
{
    toolBar->setEnabled(false);
    readCPUCount();
    toolBar->setEnabled(true);
}

void MainWindow::applyChanges()
{
    toolBar->setEnabled(false);
    baseWdg->setEnabled(false);
    for (int i=0; i<baseLayout->count(); i++) {
        CPU_Item *wdg = static_cast<CPU_Item*>(
                    baseLayout->itemAt(i)->widget());
        if ( NULL!=wdg )
            wdg->applyNewSettings();
    };
    baseWdg->setEnabled(true);
    reloadCPUItems();
    toolBar->setEnabled(true);
}

void MainWindow::resizeApp(bool state)
{
    baseWdg->setEnabled(!state);
    toolBar->setResizingState(!state);
    if ( state ) {
        setWindowFlags(flags);
    } else {
        setWindowFlags(flags ^ Qt::FramelessWindowHint);
    };
    show();
}

void MainWindow::receiveCurrGovernor(QString &arg)
{
    for (int i=1; i<baseLayout->count(); i++) {
        CPU_Item *wdg = static_cast<CPU_Item*>(
                    baseLayout->itemAt(i)->widget());
        if ( NULL!=wdg )
            wdg->setCurrGovernor(arg);
    };
}

void MainWindow::receiveCurrMaxFreq(QString &arg)
{
    for (int i=1; i<baseLayout->count(); i++) {
        CPU_Item *wdg = static_cast<CPU_Item*>(
                    baseLayout->itemAt(i)->widget());
        if ( NULL!=wdg )
            wdg->setCurrMaxFreq(arg);
    };
}

void MainWindow::receiveCurrMinFreq(QString &arg)
{
    for (int i=1; i<baseLayout->count(); i++) {
        CPU_Item *wdg = static_cast<CPU_Item*>(
                    baseLayout->itemAt(i)->widget());
        if ( NULL!=wdg )
            wdg->setCurrMinFreq(arg);
    };
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if ( ev->type()==QEvent::Close ) {
        saveSettings();
        trayIcon->hide();
        ev->accept();
    };
}

void MainWindow::readSettings()
{
    bool firstForAll, restore, showAtStart;
    restoreGeometry(settings.value("Geometry").toByteArray());
    firstForAll = settings.value("FirstForAll", false).toBool();
    restore = settings.value("Restore", false).toBool();
    toolBar->setRestoreState(restore);
    showAtStart = settings.value("ShowAtStart", false).toBool();
    toolBar->setShowAtStartState(showAtStart);
    settings.beginGroup("CPUs");
    QStringList cpus = settings.childGroups();
    CPU_COUNT = cpus.count();
    if (restore && CPU_COUNT) {
        // set FirstForAll only if app restore data
        toolBar->setFirstForAllState(firstForAll);
        baseLayout = new QVBoxLayout();
        baseWdg = new QWidget(this);
        foreach (QString cpuName, cpus) {
            settings.beginGroup(cpuName);
            QString cpuNum = settings.value("Number", "0").toString();
            CPU_Item *wdg = new CPU_Item(this, cpuNum);
            QString curr_gov, max_freq, min_freq;
            curr_gov = settings.value("Governor", "undefined").toString();
            max_freq = settings.value("MaxFreq", "undefined").toString();
            min_freq = settings.value("MinFreq", "undefined").toString();
            wdg->setCurrGovernor(curr_gov);
            wdg->setCurrMaxFreq(max_freq);
            wdg->setCurrMinFreq(min_freq);
            baseLayout->addWidget(wdg);
            if ( cpuNum=="0" ) {
                connect(wdg, SIGNAL(curr_gov(QString&)),
                        this, SLOT(receiveCurrGovernor(QString&)));
                connect(wdg, SIGNAL(max_freq(QString&)),
                        this, SLOT(receiveCurrMaxFreq(QString&)));
                connect(wdg, SIGNAL(min_freq(QString&)),
                        this, SLOT(receiveCurrMinFreq(QString&)));
                QString gov_Icon =
                        wdg->getGovernor();
                trayIcon->setIcon(
                            QIcon::fromTheme(
                                gov_Icon,
                                QIcon(QString(":/%1.png")
                                      .arg(gov_Icon))));
            };
            settings.endGroup();
        };
        baseWdg->setLayout(baseLayout);
        setCentralWidget(baseWdg);
        setFirstForAll(firstForAll);
        applyChanges();
    } else
        readCPUCount();
    settings.endGroup();
}

void MainWindow::saveSettings()
{
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("ShowAtStart", toolBar->getShowAtStartState());
    settings.setValue("Restore", toolBar->getRestoreState());
    settings.setValue("FirstForAll", toolBar->getFirstForAllState());
    settings.beginGroup("CPUs");
    for (int i=0; i<baseLayout->count(); i++) {
        CPU_Item *wdg = static_cast<CPU_Item*>(
                    baseLayout->itemAt(i)->widget());
        if ( NULL!=wdg ) {
            settings.beginGroup(QString("CPU%1").arg(i));
            settings.setValue("Number", wdg->getCPUNumber());
            settings.setValue("Online", wdg->getOnlineState());
            settings.setValue("Governor", wdg->getGovernor());
            settings.setValue("MaxFreq", wdg->getMaxFreq());
            settings.setValue("MinFreq", wdg->getMinFreq());
            settings.endGroup();
        };
    };
    settings.endGroup();
    settings.sync();
}

void MainWindow::focusInEvent(QFocusEvent *ev)
{
    ev->accept();
    if ( timerID ) {
        killTimer(timerID);
        timerID = 0;
    }
}

void MainWindow::focusOutEvent(QFocusEvent *ev)
{
    ev->accept();
    if ( timerID==0 ) {
        timerID = startTimer(25000);
    }
}

void MainWindow::timerEvent(QTimerEvent *ev)
{
    if ( timerID && ev->timerId()==timerID ) {
        killTimer(timerID);
        timerID = 0;
        _hide();
    }
}
