#include "CPUWidget.h"
#include "ui_CPUWidget.h"

CPUWidget::CPUWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CPUWidget)
{
    ui->setupUi(this);
    setDefaultDisposition();

    mDisas = new CPUDisassembly(this);
    mSideBar = new CPUSideBar(mDisas);
    connect(mDisas, SIGNAL(tableOffsetChanged(dsint)), mSideBar, SLOT(changeTopmostAddress(dsint)));
    connect(mDisas, SIGNAL(viewableRows(int)), mSideBar, SLOT(setViewableRows(int)));
    connect(mDisas, SIGNAL(selectionChanged(dsint)), mSideBar, SLOT(setSelection(dsint)));
    connect(Bridge::getBridge(), SIGNAL(dbgStateChanged(DBGSTATE)), mSideBar, SLOT(debugStateChangedSlot(DBGSTATE)));
    connect(Bridge::getBridge(), SIGNAL(updateSideBar()), mSideBar, SLOT(repaint()));

    QSplitter* splitter = new QSplitter(this);
    splitter->addWidget(mSideBar);
    splitter->addWidget(mDisas);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(1);

    ui->mTopLeftUpperFrameLayout->addWidget(splitter);

    mInfo = new CPUInfoBox();
    ui->mTopLeftLowerFrameLayout->addWidget(mInfo);
    int height = mInfo->getHeight();
    ui->mTopLeftLowerFrame->setMinimumHeight(height + 2);
    ui->mTopLeftLowerFrame->setMaximumHeight(height + 2);

    connect(mDisas, SIGNAL(selectionChanged(dsint)), mInfo, SLOT(disasmSelectionChanged(dsint)));

    mGeneralRegs = new RegistersView(0);
    mGeneralRegs->setFixedWidth(1000);
    mGeneralRegs->setFixedHeight(1400);
    mGeneralRegs->ShowFPU(true);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidget(mGeneralRegs);

    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal{border:1px solid grey;background:#f1f1f1;height:10px}QScrollBar::handle:horizontal{background:#aaa;min-width:20px;margin:1px}QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;height:0}");
    scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{border:1px solid grey;background:#f1f1f1;width:10px}QScrollBar::handle:vertical{background:#aaa;min-height:20px;margin:1px}QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{width:0;height:0}");

    QPushButton* button_changeview = new QPushButton("");
    button_changeview->setStyleSheet("Text-align:left;padding: 4px;padding-left: 10px;");
    connect(button_changeview, SIGNAL(clicked()), mGeneralRegs, SLOT(onChangeFPUViewAction()));
    mGeneralRegs->SetChangeButton(button_changeview);

    ui->mTopRightFrameLayout->addWidget(button_changeview);
    ui->mTopRightFrameLayout->addWidget(scrollArea);

    mDump = new CPUMultiDump(mDisas, 5, 0); //dump widget
    ui->mBotLeftFrameLayout->addWidget(mDump);

    mStack = new CPUStack(mDump, 0); //stack widget
    ui->mBotRightFrameLayout->addWidget(mStack);
}

CPUWidget::~CPUWidget()
{
    delete ui;
}

void CPUWidget::setDefaultDisposition(void)
{
    QList<int> sizesList;
    int wTotalSize;

    // Vertical Splitter
    wTotalSize = ui->mVSplitter->widget(0)->size().height() + ui->mVSplitter->widget(1)->size().height();

    sizesList.append(wTotalSize * 70 / 100);
    sizesList.append(wTotalSize - wTotalSize * 70 / 100);

    ui->mVSplitter->setSizes(sizesList);

    // Top Horizontal Splitter
    wTotalSize = ui->mTopHSplitter->widget(0)->size().height() + ui->mTopHSplitter->widget(1)->size().height();

    sizesList.append(wTotalSize * 70 / 100);
    sizesList.append(wTotalSize - wTotalSize * 70 / 100);

    ui->mTopHSplitter->setSizes(sizesList);

    // Bottom Horizontal Splitter
    wTotalSize = ui->mBotHSplitter->widget(0)->size().height() + ui->mBotHSplitter->widget(1)->size().height();

    sizesList.append(wTotalSize * 70 / 100);
    sizesList.append(wTotalSize - wTotalSize * 70 / 100);

    ui->mBotHSplitter->setSizes(sizesList);
}

void CPUWidget::setDisasmFocus()
{
    mDisas->setFocus();
}

QVBoxLayout* CPUWidget::getTopLeftUpperWidget()
{
    return ui->mTopLeftUpperFrameLayout;
}

QVBoxLayout* CPUWidget::getTopLeftLowerWidget()
{
    return ui->mTopLeftLowerFrameLayout;
}

QVBoxLayout* CPUWidget::getTopRightWidget()
{
    return ui->mTopRightFrameLayout;
}

QVBoxLayout* CPUWidget::getBotLeftWidget()
{
    return ui->mBotLeftFrameLayout;
}

QVBoxLayout* CPUWidget::getBotRightWidget()
{
    return ui->mBotRightFrameLayout;
}

CPUSideBar* CPUWidget::getSidebarWidget()
{
    return mSideBar;
}

CPUDisassembly* CPUWidget::getDisasmWidget()
{
    return mDisas;
}

CPUMultiDump* CPUWidget::getDumpWidget()
{
    return mDump;
}

CPUStack* CPUWidget::getStackWidget()
{
    return mStack;
}
