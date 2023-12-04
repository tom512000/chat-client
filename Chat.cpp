#include <QMessageBox>
#include "Chat.h"

#include <iostream>
#include <map>
#include <QDockWidget>
#include <QInputDialog>

////////////////////////////////////////////////////////////////////////////////
// Chat ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Processeurs.
const std::map<QString, Chat::Processor> Chat::PROCESSORS {
  {"#error", &Chat::process_error},
  {"#alias", &Chat::process_alias},
  {"#connected", &Chat::process_user_connected},
  {"#disconnected", &Chat::process_user_disconnected},
  {"#renamed", &Chat::process_user_renamed},
  {"#list", &Chat::process_user_list},
  {"#private", &Chat::process_user_private}
};

// Constructeur.
Chat::Chat (const QString & host, quint16 port, QObject * parent) :
  QObject (parent),
  socket ()
{
  // Signal "connected" émis lorsque la connexion est effectuée.
  connect(&socket, &QTcpSocket::connected, [this, host, port]() {
      emit connected(host, port);
  });

  // Signal "disconnected" émis lors d'une déconnexion du socket.
  connect(&socket, &QTcpSocket::disconnected, [this]() {
      emit disconnected();
  });

  // Lecture.
  connect (&socket, &QIODevice::readyRead, [this] () {
    // Tant que l'on peut lire une ligne...
    while (socket.canReadLine ())
    {
      // Lecture d'une ligne et suppression du "newline".
      QString m = socket.readLine().chopped(1);

      // Flot de lecture.
      QTextStream stream (&m);
      // Lecture d'une commande potentielle.
      QString command;
      stream >> command;

      // Recherche de la commande serveur dans le tableau associatif.
      // - si elle existe, traitement du reste du message par le processeur ;
      // - sinon, émission du signal "message" contenant la ligne entière.

      auto i = PROCESSORS.find(command);
      if(i != PROCESSORS.end()) {
          (this->*(i->second))(stream);
      } else {
          emit message(m);
      }
    }
  });

  // CONNEXION !
  socket.connectToHost (host, port, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
}

Chat::~Chat ()
{
  // Déconnexion des signaux.
  socket.disconnect ();
}

// Commande "#alias"
void Chat::process_alias(QTextStream &is)
{
    QString newAlias;
    is >> newAlias;
    emit alias(newAlias);
}

// Commande "#connected"
void Chat::process_user_connected(QTextStream &is)
{
    QString username;
    is >> username;
    emit user_connected(username);
}

// Commande "#disconnected"
void Chat::process_user_disconnected(QTextStream &is)
{
    QString username;
    is >> username;
    emit user_disconnected(username);
}

// Commande "#renamed"
void Chat::process_user_renamed(QTextStream &is)
{
    QString oldUsername, newUsername;
    is >> oldUsername >> newUsername;
    emit user_renamed(oldUsername, newUsername);
}

// Commande "#list"
void Chat::process_user_list(QTextStream &is)
{
    QStringList userList;
     while (!is.atEnd()) {
         QString user;
         is >> user;
         userList.append(user);
     }
     emit user_list(userList);
}

// Commande "#private"
void Chat::process_user_private(QTextStream &is)
{
    QString sender, message;
        is >> sender >> ws;
        message = is.readLine();
        emit user_private(sender, message);
}

// Commande "#error"
void Chat::process_error (QTextStream & is)
{
  QString id;
  is >> id >> ws;
  emit error (id);
}

// Envoi d'un message à travers le socket.
void Chat::write (const QString & message)
{
  socket.write (message.toUtf8 () + '\n');
}

////////////////////////////////////////////////////////////////////////////////
// ChatWindow //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ChatWindow::ChatWindow (const QString & host, quint16 port, QWidget * parent) :
  QMainWindow (parent),
  chat (host, port, this),
  text (this),
  input (this)
{
  text.setReadOnly (true);
  setCentralWidget (&text);

  // Insertion de la zone de saisie.
  // QDockWidget insérable en haut ou en bas, inséré en bas.
  QDockWidget *dockWidget = new QDockWidget(tr("Message Input"), this);
  dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
  dockWidget->setWidget(&input);
  addDockWidget(Qt::BottomDockWidgetArea, dockWidget);


  // Désactivation de la zone de saisie.
  input.setEnabled (false);

  // Envoi de messages lorsque la touche "entrée" est pressée.
  // - transmission du texte au moteur de messagerie instantanée ;
  // - effacement de la zone de saisie.
  connect(&input, &QLineEdit::returnPressed, [this]() {
      chat.write(input.text());
      input.clear();
  });

  // Connexion.
  // - affichage d'un message confirmant la connexion ;
  // - saisie de l'alias ;
  // - envoi de l'alias ;
  // - activation de la zone de saisie.
  connect(&chat, &Chat::connected, [this, host]() {
      text.append("Vous êtes connecté !");
      QString alias = QInputDialog::getText(this, tr("Entrez votre pseudonyme : "), tr("Pseudonyme : "));
      chat.write(alias);
      input.setEnabled(true);
  });

  // Déconnexion.
  // - désactivation de la zone de saisie.
  // - affichage d'un message pour signaler la déconnexion.
  connect(&chat, &Chat::disconnected, [this]() {
      input.setEnabled(false);
      text.append(tr("Vous êtes déconnecté !"));
  });

  // Messages.
  connect (&chat, &Chat::message, [this] (const QString & message) {
    text.append (message);
  });

  // Liste des utilisateurs.
  connect(&chat, &Chat::user_list, [this](const QStringList &userList) {
      QString userListStr = tr("<i>Utilisateurs connectés : </i> ") + userList.join(", ");
      text.append(userListStr);
  });

  // Message privé.
  connect(&chat, &Chat::user_private, [this](const QString &sender, const QString &message) {
      QString privateMessage = tr("<i>Message privé de %1 : </i> %2").arg(sender, message);
      text.append(privateMessage);
  });

  // Nouvel alias d'un utilisateur.
  connect(&chat, &Chat::alias, [this](const QString &newAlias) {
      text.append(tr("<i>Nouveau pseudo : </i> ") + newAlias);
  });

  // Connexion d'un utilisateur.
  connect(&chat, &Chat::user_connected, [this](const QString &username) {
      text.append(tr("<i>Utilisateur connecté : </i> ") + username);
  });

  // Déconnexion d'un utilisateur.
  connect(&chat, &Chat::user_disconnected, [this](const QString &username) {
      text.append(tr("<i>Utilisateur déconnecté :</i> ") + username);
  });

  // Gestion des erreurs.
  connect (&chat, &Chat::error, [this] (const QString & id) {
    QMessageBox::critical (this, tr("Error"), id);
  });

  // CONNEXION !
  text.append (tr("<b>Connecting...</b>"));
}
