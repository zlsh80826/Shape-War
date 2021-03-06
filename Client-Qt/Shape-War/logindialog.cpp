#include "logindialog.h"

/*!
 * \brief LoginDialog::LoginDialog construtor
 * \param parent
 */
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setUpGUI();
    setWindowTitle(tr("User Login"));
    setModal(true);
}

/*!
 * \brief LoginDialog::setUpGUI set up the GUI of this dialog
 */
void LoginDialog::setUpGUI() {
    // set up the layout
    QGridLayout *formGridLayout = new QGridLayout(this);

    // initialize the editlines
    editServerIP = new QLineEdit(this);
    editServerIP->setText("localhost");
    editPort = new QLineEdit(this);
    editPort->setText("8888");
    editUsername = new QLineEdit(this);
    // initialize the password field so that it does not echo characters
    // editPassword = new QLineEdit(this);
    // editPassword->setEchoMode(QLineEdit::Password);

    // initialize the labels
    labelServerIP = new QLabel(this);
    labelServerIP->setText(tr("ServerIP"));
    labelServerIP->setBuddy(editServerIP);

    labelPort = new QLabel(this);
    labelPort->setText(tr("Port"));
    labelPort->setBuddy(editPort);

    labelUsername = new QLabel(this);
    labelUsername->setText(tr("Username"));
    labelUsername->setBuddy(editUsername);

    /*
    labelPassword = new QLabel(this);
    labelPassword->setText(tr("Password"));
    labelPassword->setBuddy(editPassword);

    labelRegister = new QLabel(this);
    labelRegister->setOpenExternalLinks(true);
    labelRegister->setText("<a href=https://www.google.com.tw>Register</a>");
    */

    // initialize buttons
    buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Login"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Abort"));

    // connects slots
    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this,
            SLOT(close()));

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this,
            SLOT(slotAcceptLogin()));

    // anonymousCheck = new QCheckBox("Play anonymous", this);
    // connect(anonymousCheck, SIGNAL(clicked()), this,
    //        SLOT(anonymousCheckOnclicked()));

    // place components into the dialog
    int rowCounting = 0;
    formGridLayout->addWidget(labelServerIP, rowCounting, 0);
    formGridLayout->addWidget(editServerIP, rowCounting, 1);
    rowCounting++;
    formGridLayout->addWidget(labelPort, rowCounting, 0);
    formGridLayout->addWidget(editPort, rowCounting, 1);
    rowCounting++;
    formGridLayout->addWidget(labelUsername, rowCounting, 0);
    formGridLayout->addWidget(editUsername, rowCounting, 1);
    // rowCounting++;
    // formGridLayout->addWidget(labelPassword, rowCounting, 0);
    // formGridLayout->addWidget(editPassword, rowCounting, 1);
    // rowCounting++;
    // formGridLayout->addWidget(anonymousCheck, rowCounting, 0);
    // formGridLayout->addWidget(labelRegister, rowCounting, 1);
    rowCounting++;
    formGridLayout->addWidget(buttons, rowCounting, 0, 1, 2);

    setLayout(formGridLayout);
}

/*void LoginDialog::setPassword(QString &password) {
    editPassword->setText(password);
}*/

void LoginDialog::slotAcceptLogin() {
    QString serverIP = editServerIP->text();
    QString port = editPort->text();
    QString username = editUsername->text();
    // QString password = editPassword->text();
    // bool isAnonymous = anonymousCheck->isChecked();
    emit acceptLogin(serverIP, port, username);
    // password, // current password
    // isAnonymous);

    // close this dialog
    close();
}
/*void LoginDialog::anonymousCheckOnclicked() {
    if (anonymousCheck->isChecked()) {
        editPassword->setEnabled(false);
    } else {
        editPassword->setEnabled(true);
    }
}*/
