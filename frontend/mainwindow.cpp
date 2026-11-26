
#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QImage>
#include <QMessageBox>
#include <QFont>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QDate>
#include <QAbstractItemView>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

// =================================================
// BACKGROUND IMAGE
// =================================================

static QPixmap makeBlurredPixmap(const QPixmap &source)
{
    if (source.isNull())
        return QPixmap();

    QGraphicsScene scene;
    QGraphicsPixmapItem *item = scene.addPixmap(source);

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(24);
    blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    item->setGraphicsEffect(blur);

    scene.setSceneRect(source.rect());

    QImage blurredImage(
        source.size(),
        QImage::Format_ARGB32_Premultiplied
        );

    blurredImage.fill(Qt::transparent);

    QPainter painter(&blurredImage);
    scene.render(
        &painter,
        QRectF(blurredImage.rect()),
        QRectF(source.rect())
        );
    painter.end();

    return QPixmap::fromImage(blurredImage);
}

class BackgroundWidget : public QWidget
{
public:
    explicit BackgroundWidget(
        const QPixmap &background,
        QWidget *parent = nullptr
        )
        : QWidget(parent), m_background(background)
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(
            QPainter::SmoothPixmapTransform,
            true
            );

        if (!m_background.isNull()) {

            const QPixmap scaled = m_background.scaled(
                size(),
                Qt::KeepAspectRatioByExpanding,
                Qt::SmoothTransformation
                );

            const QPoint centeredPosition(
                (width() - scaled.width()) / 2,
                (height() - scaled.height()) / 2
                );

            painter.drawPixmap(centeredPosition, scaled);

        } else {
            painter.fillRect(rect(), QColor(7, 20, 38));
        }

        painter.fillRect(
            rect(),
            QColor(4, 17, 34, 170)
            );
    }

private:
    QPixmap m_background;
};


// =================================================
// ADMIN DASHBOARD
// =================================================

class AdminDashboard : public QWidget
{
public:

    explicit AdminDashboard(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setObjectName("adminDashboard");

        setStyleSheet(R"(

            QWidget#adminDashboard {
                background: #F3F7FC;
                color: #172B45;
            }

            QFrame#sidebar {
                background: #0B1B30;
                border: none;
            }

            QLabel#brandTitle {
                color: #FFFFFF;
                font-size: 17px;
                font-weight: 800;
            }

            QLabel#brandSub {
                color: #8FAAC4;
                font-size: 10px;
            }

            QLabel#menuCaption {
                color: #66839F;
                font-size: 10px;
                font-weight: 800;
                padding: 12px 12px 4px;
            }

            QPushButton#navButton {
                color: #B8C9DA;
                background: transparent;
                border: none;
                border-radius: 8px;
                text-align: left;
                padding: 12px 14px;
                font-size: 13px;
            }

            QPushButton#navButton:hover {
                background: #183653;
                color: white;
            }

            QPushButton#navButton[active="true"] {
                background: #1265A8;
                color: white;
                font-weight: 700;
            }

            QLabel#topTitle {
                color: #102A43;
                font-size: 22px;
                font-weight: 800;
            }

            QLabel#muted {
                color: #7890A8;
                font-size: 12px;
            }

            QFrame#statCard,
            QFrame#panel {
                background: white;
                border: 1px solid #E1EAF3;
                border-radius: 13px;
            }

            QLabel#statLabel {
                color: #71869B;
                font-size: 12px;
            }

            QLabel#statValue {
                color: #102A43;
                font-size: 27px;
                font-weight: 800;
            }

            QLabel#statNote {
                color: #16A085;
                font-size: 11px;
            }

            QLabel#panelTitle {
                color: #18334F;
                font-size: 15px;
                font-weight: 750;
            }

            QLabel#placeholderTitle {
                color: #17324D;
                font-size: 24px;
                font-weight: 800;
            }

            QPushButton#logoutButton {
                color: #D8E7F5;
                background: #173653;
                border: none;
                border-radius: 8px;
                padding: 11px;
                text-align: left;
            }

            QPushButton#logoutButton:hover {
                background: #244B70;
            }

            QTableWidget {
                background: white;
                border: none;
                gridline-color: #EDF2F7;
                color: #29445F;
            }

            QHeaderView::section {
                background: #F5F8FC;
                color: #71869B;
                border: none;
                padding: 9px;
                font-weight: 700;
            }

        )");

        // ROOT LAYOUT

        QHBoxLayout *root = new QHBoxLayout(this);

        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        // =================================================
        // SIDEBAR
        // =================================================

        QFrame *sidebar = new QFrame;
        sidebar->setObjectName("sidebar");
        sidebar->setFixedWidth(235);

        QVBoxLayout *sideLayout = new QVBoxLayout(sidebar);

        sideLayout->setContentsMargins(16, 22, 16, 18);
        sideLayout->setSpacing(7);

        QLabel *brand = new QLabel("❄  FROSTFLOW");
        brand->setObjectName("brandTitle");

        QLabel *brandSub = new QLabel(
            "COLD STORAGE MANAGEMENT"
            );
        brandSub->setObjectName("brandSub");

        sideLayout->addWidget(brand);
        sideLayout->addWidget(brandSub);
        sideLayout->addSpacing(25);

        QLabel *mainCaption = new QLabel("MAIN MENU");
        mainCaption->setObjectName("menuCaption");

        sideLayout->addWidget(mainCaption);

        // =================================================
        // CONTENT STACK
        // =================================================

        QStackedWidget *contentStack = new QStackedWidget;

        QWidget *homePage = new QWidget;

        QVBoxLayout *homeLayout = new QVBoxLayout(homePage);

        homeLayout->setContentsMargins(0, 0, 0, 0);
        homeLayout->setSpacing(18);

        QLabel *welcome = new QLabel("Overview");
        welcome->setObjectName("topTitle");

        QLabel *welcomeSub = new QLabel(
            "Here's what's happening in your cold storage facility today."
            );

        welcomeSub->setObjectName("muted");

        homeLayout->addWidget(welcome);
        homeLayout->addWidget(welcomeSub);

        // =================================================
        // STATISTICS CARDS
        // =================================================

        QGridLayout *stats = new QGridLayout;

        stats->setHorizontalSpacing(14);
        stats->setVerticalSpacing(14);

        auto addStat = [stats](
                           int row,
                           int col,
                           const QString &label,
                           const QString &value,
                           const QString &note,
                           const QString &symbol
                           ) {

            QFrame *card = new QFrame;
            card->setObjectName("statCard");

            QVBoxLayout *layout = new QVBoxLayout(card);

            layout->setContentsMargins(18, 16, 18, 16);

            QLabel *icon = new QLabel(symbol);

            icon->setStyleSheet(
                "color:#1688C9;"
                "font-size:20px;"
                "font-weight:800;"
                );

            QLabel *name = new QLabel(label);
            name->setObjectName("statLabel");

            QLabel *number = new QLabel(value);
            number->setObjectName("statValue");

            QLabel *detail = new QLabel(note);
            detail->setObjectName("statNote");

            layout->addWidget(icon);
            layout->addWidget(name);
            layout->addWidget(number);
            layout->addWidget(detail);

            stats->addWidget(card, row, col);
        };

        addStat(
            0, 0,
            "TOTAL PRODUCTS",
            "1,248",
            "Products registered",
            "▦"
            );

        addStat(
            0, 1,
            "COLD ROOMS",
            "08",
            "6 rooms operational",
            "❄"
            );

        addStat(
            1, 0,
            "STOCK ENTRIES",
            "36",
            "Today's incoming stock",
            "↓"
            );

        addStat(
            1, 1,
            "ACTIVE ALERTS",
            "02",
            "Require your attention",
            "!"
            );

        homeLayout->addLayout(stats);

        // =================================================
        // LOWER SECTION
        // =================================================

        QHBoxLayout *lower = new QHBoxLayout;

        lower->setSpacing(14);

        // RECENT ACTIVITY

        QFrame *activityPanel = new QFrame;
        activityPanel->setObjectName("panel");

        QVBoxLayout *activityLayout =
            new QVBoxLayout(activityPanel);

        QLabel *activityTitle =
            new QLabel("Recent Activity");

        activityTitle->setObjectName("panelTitle");

        activityLayout->addWidget(activityTitle);

        QTableWidget *activityTable =
            new QTableWidget(4, 3);

        activityTable->setHorizontalHeaderLabels({
            "ACTIVITY",
            "REFERENCE",
            "STATUS"
        });

        activityTable->verticalHeader()->setVisible(false);

        activityTable->setEditTriggers(
            QAbstractItemView::NoEditTriggers
            );

        activityTable->setSelectionMode(
            QAbstractItemView::NoSelection
            );

        activityTable->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch
            );

        activityTable->setMinimumHeight(190);

        const QString rows[4][3] = {

        {
            "Stock received",
            "ENT-0241",
            "Completed"
        },

            {
                "Product dispatched",
                "EXT-0188",
                "Completed"
            },

            {
                "Temperature check",
                "ROOM-03",
                "Normal"
            },

            {
                "Inventory updated",
                "INV-0092",
                "Updated"
            }

    };

    for (int r = 0; r < 4; ++r) {

        for (int c = 0; c < 3; ++c) {

            activityTable->setItem(
                r,
                c,
                new QTableWidgetItem(rows[r][c])
                );

        }

    }

    activityLayout->addWidget(activityTable);

    // COLD ROOM STATUS

    QFrame *temperaturePanel = new QFrame;

    temperaturePanel->setObjectName("panel");
    temperaturePanel->setMinimumWidth(220);

    QVBoxLayout *temperatureLayout =
        new QVBoxLayout(temperaturePanel);

    QLabel *temperatureTitle =
        new QLabel("Cold Room Status");

    temperatureTitle->setObjectName("panelTitle");

    temperatureLayout->addWidget(temperatureTitle);

    const QString rooms[] = {

    "Room 01       -18 °C",
        "Room 02       -20 °C",
        "Room 03       -16 °C",
        "Room 04       -19 °C"

};

for (const QString &room : rooms) {

    QLabel *roomLabel =
        new QLabel("●  " + room);

    roomLabel->setStyleSheet(
        "color:#287A70;"
        "background:#F0FAF8;"
        "border-radius:7px;"
        "padding:11px;"
        "font-size:12px;"
        );

    temperatureLayout->addWidget(roomLabel);

}

temperatureLayout->addStretch();

lower->addWidget(activityPanel, 3);
lower->addWidget(temperaturePanel, 2);

homeLayout->addLayout(lower, 1);

contentStack->addWidget(homePage);

// =================================================
// NAVIGATION
// =================================================

QStringList labels = {

"Dashboard",
    "Farmers",
    "Products",
    "Cold Rooms",
    "Inventory",
    "Stock Entries",
    "Stock Exits",
    "Reports",
    "Users & Roles"

};

for (int i = 0; i < labels.size(); ++i) {

    QPushButton *button =
        new QPushButton("  " + labels[i]);

    button->setObjectName("navButton");

    button->setProperty("active", i == 0);

    button->setCursor(Qt::PointingHandCursor);

    sideLayout->addWidget(button);

    button->setProperty("pageIndex", i);

    connect(
        button,
        &QPushButton::clicked,
        this,
        [=]() {

            const int index =
                button->property("pageIndex").toInt();

            for (
                QPushButton *other :
                sidebar->findChildren<QPushButton *>("navButton")
                ) {

                other->setProperty(
                    "active",
                    other == button
                    );

                other->style()->unpolish(other);
                other->style()->polish(other);

            }

            if (index == 0) {

                contentStack->setCurrentIndex(0);
                return;

            }

            // Placeholder page

            QWidget *placeholder = new QWidget;

            QVBoxLayout *placeholderLayout =
                new QVBoxLayout(placeholder);

            placeholderLayout->setAlignment(
                Qt::AlignCenter
                );

            QLabel *title =
                new QLabel(labels[index]);

            title->setObjectName("placeholderTitle");

            QLabel *message = new QLabel(
                "This section is ready for the next development stage."
                );

            message->setObjectName("muted");

            placeholderLayout->addWidget(
                title,
                0,
                Qt::AlignCenter
                );

            placeholderLayout->addWidget(
                message,
                0,
                Qt::AlignCenter
                );

            contentStack->addWidget(placeholder);

            contentStack->setCurrentWidget(placeholder);

        }
        );

}

// =================================================
// SIDEBAR FOOTER
// =================================================

sideLayout->addStretch();

QLabel *adminLabel = new QLabel(
    "SIGNED IN AS\nAdministrator"
    );

adminLabel->setStyleSheet(
    "color:#D7E7F5;"
    "font-size:12px;"
    "padding:8px;"
    );

sideLayout->addWidget(adminLabel);

m_logoutButton = new QPushButton("↪   Log out");

m_logoutButton->setObjectName("logoutButton");

sideLayout->addWidget(m_logoutButton);

// =================================================
// RIGHT CONTENT AREA
// =================================================

QWidget *right = new QWidget;

QVBoxLayout *rightLayout = new QVBoxLayout(right);

rightLayout->setContentsMargins(
    28, 22, 28, 24
    );

rightLayout->setSpacing(18);

// TOP BAR

QHBoxLayout *topbar = new QHBoxLayout;

QLabel *section =
    new QLabel("ADMINISTRATION");

section->setObjectName("muted");

QLabel *date =
    new QLabel(
        QDate::currentDate().toString(
            "dddd, dd MMMM yyyy"
            )
        );

date->setObjectName("muted");

topbar->addWidget(section);
topbar->addStretch();
topbar->addWidget(date);

rightLayout->addLayout(topbar);

rightLayout->addWidget(contentStack, 1);

root->addWidget(sidebar);
root->addWidget(right, 1);
}

QPushButton *logoutButton() const
{
    return m_logoutButton;
}

private:

QPushButton *m_logoutButton = nullptr;

};


// =================================================
// MAIN WINDOW
// =================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Cold Storage Management System");

    resize(1280, 800);

    setMinimumSize(900, 680);

    QFont appFont("Segoe UI", 10);

    QApplication::setFont(appFont);

    // =================================================
    // LOAD BACKGROUND IMAGE
    // =================================================

    QPixmap originalWarehouseImage(
        ":/images/images/cold_storage_hero.png"
        );

    if (!originalWarehouseImage.isNull()) {

        qDebug() << "Image loaded successfully!";

    } else {

        qDebug() << "Image not found! Check the resource path.";

    }

    QPixmap warehouseImage =
        makeBlurredPixmap(originalWarehouseImage);

    BackgroundWidget *central =
        new BackgroundWidget(warehouseImage, this);

    central->setObjectName("central");

    QVBoxLayout *mainLayout =
        new QVBoxLayout(central);

    mainLayout->setContentsMargins(
        34, 25, 34, 22
        );

    mainLayout->setSpacing(0);

    // =================================================
    // TAGLINE
    // =================================================

    QLabel *tagline =
        new QLabel("KEEP IT FRESH, MANAGE IT SMART");

    tagline->setObjectName("tagline");

    tagline->setAlignment(Qt::AlignCenter);

    tagline->setSizePolicy(
        QSizePolicy::Fixed,
        QSizePolicy::Fixed
        );

    mainLayout->addWidget(
        tagline,
        0,
        Qt::AlignRight
        );

    mainLayout->addStretch(1);

    // =================================================
    // LOGIN CARD
    // =================================================

    QFrame *loginCard = new QFrame;

    loginCard->setObjectName("loginCard");

    loginCard->setMinimumWidth(470);

    loginCard->setMaximumWidth(560);

    loginCard->setSizePolicy(
        QSizePolicy::Preferred,
        QSizePolicy::Fixed
        );

    QVBoxLayout *loginLayout =
        new QVBoxLayout(loginCard);

    loginLayout->setContentsMargins(
        48, 34, 48, 32
        );

    loginLayout->setSpacing(13);

    // USER ICON

    QLabel *userIcon = new QLabel("❄");

    userIcon->setObjectName("userIcon");

    userIcon->setAlignment(Qt::AlignCenter);

    userIcon->setFixedSize(72, 72);

    loginLayout->addWidget(
        userIcon,
        0,
        Qt::AlignCenter
        );

    // WELCOME TITLE

    QLabel *welcomeTitle = new QLabel(
        "Welcome <span style='color:#22D3EE;'>Back</span>"
        );

    welcomeTitle->setObjectName("welcomeTitle");

    welcomeTitle->setAlignment(Qt::AlignCenter);

    welcomeTitle->setTextFormat(Qt::RichText);

    QLabel *welcomeSubtitle =
        new QLabel("Sign in to access your account");

    welcomeSubtitle->setObjectName("welcomeSubtitle");

    welcomeSubtitle->setAlignment(Qt::AlignCenter);

    loginLayout->addWidget(welcomeTitle);

    loginLayout->addWidget(welcomeSubtitle);

    // SEPARATOR

    QFrame *separator = new QFrame;

    separator->setObjectName("separator");

    separator->setFixedSize(120, 3);

    loginLayout->addWidget(
        separator,
        0,
        Qt::AlignCenter
        );

    loginLayout->addSpacing(9);

    // =================================================
    // USERNAME
    // =================================================

    QLabel *usernameLabel =
        new QLabel("USERNAME");

    usernameLabel->setObjectName("fieldLabel");

    QFrame *usernameFrame = new QFrame;

    usernameFrame->setObjectName("inputFrame");

    QHBoxLayout *usernameLayout =
        new QHBoxLayout(usernameFrame);

    usernameLayout->setContentsMargins(
        15, 0, 15, 0
        );

    usernameLayout->setSpacing(12);

    QLabel *usernameIcon = new QLabel("@");

    usernameIcon->setObjectName("inputIcon");

    QLineEdit *usernameInput = new QLineEdit;

    usernameInput->setObjectName("input");

    usernameInput->setPlaceholderText(
        "Enter your username"
        );

    usernameInput->setMinimumHeight(58);

    usernameInput->setFrame(false);

    usernameLayout->addWidget(usernameIcon);

    usernameLayout->addWidget(usernameInput, 1);

    loginLayout->addWidget(usernameLabel);

    loginLayout->addWidget(usernameFrame);

    // =================================================
    // PASSWORD
    // =================================================

    QLabel *passwordLabel =
        new QLabel("PASSWORD");

    passwordLabel->setObjectName("fieldLabel");

    QFrame *passwordFrame = new QFrame;

    passwordFrame->setObjectName("inputFrame");

    QHBoxLayout *passwordLayout =
        new QHBoxLayout(passwordFrame);

    passwordLayout->setContentsMargins(
        15, 0, 12, 0
        );

    passwordLayout->setSpacing(12);

    QLabel *passwordIcon = new QLabel("▣");

    passwordIcon->setObjectName("inputIcon");

    QLineEdit *passwordInput = new QLineEdit;

    passwordInput->setObjectName("input");

    passwordInput->setPlaceholderText(
        "Enter your password"
        );

    passwordInput->setEchoMode(
        QLineEdit::Password
        );

    passwordInput->setMinimumHeight(58);

    passwordInput->setFrame(false);

    QToolButton *showPassword =
        new QToolButton;

    showPassword->setObjectName("showPassword");

    showPassword->setText("Show");

    showPassword->setCursor(
        Qt::PointingHandCursor
        );

    showPassword->setToolTip(
        "Show or hide password"
        );

    passwordLayout->addWidget(passwordIcon);

    passwordLayout->addWidget(passwordInput, 1);

    passwordLayout->addWidget(showPassword);

    loginLayout->addWidget(passwordLabel);

    loginLayout->addWidget(passwordFrame);

    // SHOW / HIDE PASSWORD

    connect(
        showPassword,
        &QToolButton::clicked,
        this,
        [passwordInput, showPassword]() {

            const bool hidden =
                passwordInput->echoMode()
                == QLineEdit::Password;

            passwordInput->setEchoMode(
                hidden
                    ? QLineEdit::Normal
                    : QLineEdit::Password
                );

            showPassword->setText(
                hidden ? "Hide" : "Show"
                );

        }
        );

    loginLayout->addSpacing(3);

    // =================================================
    // REMEMBER ME + FORGOT PASSWORD
    // =================================================

    QHBoxLayout *optionsLayout =
        new QHBoxLayout;

    QCheckBox *rememberMe =
        new QCheckBox("Remember Me");

    rememberMe->setObjectName("rememberMe");

    QPushButton *forgotPassword =
        new QPushButton("Forgot Password?  ›");

    forgotPassword->setObjectName("forgotPassword");

    forgotPassword->setCursor(
        Qt::PointingHandCursor
        );

    optionsLayout->addWidget(rememberMe);

    optionsLayout->addStretch();

    optionsLayout->addWidget(forgotPassword);

    loginLayout->addLayout(optionsLayout);

    loginLayout->addSpacing(5);

    // =================================================
    // SIGN IN BUTTON
    // =================================================

    QPushButton *loginButton =
        new QPushButton("Sign In");

    loginButton->setObjectName("loginButton");

    loginButton->setMinimumHeight(64);

    loginButton->setCursor(
        Qt::PointingHandCursor
        );

    loginLayout->addWidget(loginButton);

    loginLayout->addSpacing(10);

    // FOOTER

    QFrame *footerLine = new QFrame;

    footerLine->setObjectName("footerLine");

    footerLine->setFixedHeight(1);

    loginLayout->addWidget(footerLine);

    QLabel *footer = new QLabel(
        "❄    Cold Storage Management System    |    v1.0"
        );

    footer->setObjectName("footer");

    footer->setAlignment(Qt::AlignCenter);

    loginLayout->addWidget(footer);

    mainLayout->addWidget(
        loginCard,
        0,
        Qt::AlignHCenter
        );

    mainLayout->addStretch(1);

    QLabel *securityText = new QLabel(
        "SECURE ACCESS  •  AUTHORIZED STAFF ONLY"
        );

    securityText->setObjectName("securityText");

    securityText->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(securityText);

    // =================================================
    // PAGE STACK
    // =================================================

    QStackedWidget *pageStack =
        new QStackedWidget(this);

    AdminDashboard *adminDashboard =
        new AdminDashboard;

    pageStack->addWidget(central);

    pageStack->addWidget(adminDashboard);

    setCentralWidget(pageStack);

    // LOGOUT

    connect(
        adminDashboard->logoutButton(),
        &QPushButton::clicked,
        this,
        [pageStack, usernameInput, passwordInput]() {

            usernameInput->clear();

            passwordInput->clear();

            pageStack->setCurrentIndex(0);

        }
        );

    // =================================================
    // LOGIN ACTION
    // =================================================

    connect(
        loginButton,
        &QPushButton::clicked,
        this,
        [this,
         usernameInput,
         passwordInput,
         pageStack,
         adminDashboard]() {

            if (
                usernameInput->text().trimmed().isEmpty()
                || passwordInput->text().isEmpty()
                ) {

                QMessageBox::warning(
                    this,
                    "Missing Information",
                    "Please enter your username and password."
                    );

                return;
            }

            // Temporary login:
            // API authentication will be added later.

            pageStack->setCurrentWidget(
                adminDashboard
                );

        }
        );

    // FORGOT PASSWORD

    connect(
        forgotPassword,
        &QPushButton::clicked,
        this,
        [this]() {

            QMessageBox::information(
                this,
                "Password Recovery",
                "Please contact your system administrator."
                );

        }
        );

    // =================================================
    // LOGIN STYLE SHEET
    // =================================================

    setStyleSheet(R"(

        QLabel#tagline {
            color: #E5F5FF;
            font-size: 12px;
            font-weight: 700;
            letter-spacing: 1.5px;
            background-color: rgba(4, 20, 36, 205);
            border: 1px solid rgba(125, 211, 252, 85);
            border-radius: 9px;
            padding: 8px 12px;
        }

        QFrame#loginCard {
            background-color: rgba(5, 22, 39, 246);
            border: 1px solid rgba(125, 211, 252, 115);
            border-radius: 20px;
        }

        QLabel#userIcon {
            background-color: #0A3D63;
            color: #7DD3FC;
            border: 2px solid #22D3EE;
            border-radius: 33px;
            font-size: 22px;
        }

        QLabel#welcomeTitle {
            color: #F8FAFC;
            font-size: 35px;
            font-weight: 800;
        }

        QLabel#welcomeSubtitle {
            color: #C5D9EA;
            font-size: 15px;
        }

        QFrame#separator {
            background-color: #22D3EE;
            border: none;
            border-radius: 2px;
        }

        QLabel#fieldLabel {
            color: #E5F3FF;
            font-size: 12px;
            font-weight: 800;
            letter-spacing: 1.4px;
        }

        QFrame#inputFrame {
            background-color: rgba(3, 17, 31, 245);
            border: 1px solid #527895;
            border-radius: 11px;
        }

        QLabel#inputIcon {
            color: #8BDFFF;
            font-size: 18px;
        }

        QLineEdit#input {
            background: transparent;
            color: #FFFFFF;
            border: none;
            font-size: 15px;
            selection-background-color: #0284C7;
        }

        QLineEdit#input::placeholder {
            color: #A9BED0;
        }

        QToolButton#showPassword {
            background: transparent;
            color: #8BDFFF;
            border: none;
            font-size: 19px;
            padding: 5px;
        }

        QCheckBox#rememberMe {
            color: #E2EDF7;
            font-size: 13px;
            spacing: 8px;
        }

        QCheckBox#rememberMe::indicator {
            width: 18px;
            height: 18px;
        }

        QCheckBox#rememberMe::indicator:unchecked {
            background-color: #091D32;
            border: 1px solid #7896AE;
            border-radius: 4px;
        }

        QCheckBox#rememberMe::indicator:checked {
            background-color: #0EA5E9;
            border: 1px solid #38BDF8;
            border-radius: 4px;
        }

        QPushButton#forgotPassword {
            background: transparent;
            color: #67E8F9;
            border: none;
            font-size: 13px;
            font-weight: 700;
        }

        QPushButton#forgotPassword:hover {
            color: #FFFFFF;
        }

        QPushButton#loginButton {
            background-color: #0789EA;
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 18px;
            font-weight: 800;
        }

        QPushButton#loginButton:hover {
            background-color: #06B6D4;
        }

        QPushButton#loginButton:pressed {
            background-color: #0369A1;
        }

        QFrame#footerLine {
            background-color: rgba(125, 170, 200, 100);
            border: none;
        }

        QLabel#footer {
            color: #B8CDDF;
            font-size: 11px;
        }

        QLabel#securityText {
            color: #E0F2FE;
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 1.5px;
            background-color: rgba(4, 20, 36, 155);
            border: 1px solid rgba(125, 211, 252, 55);
            border-radius: 8px;
            padding: 9px 14px;
        }

    )");
}
