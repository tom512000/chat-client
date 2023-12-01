#include <QMessageBox>
#include "Chat.h"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Chat ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Processeurs.
const std::map<QString, Chat::Processor> Chat::PROCESSORS {
  {"#error", &Chat::process_error}
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
      QString m = socket.readLine ().chopped (1);

      // Flot de lecture.
      QTextStream stream (&m);
      // Lecture d'une commande potentielle.
      QString command;
      stream >> command;

      // Recherche de la commande serveur dans le tableau associatif.
      // - si elle existe, traitement du reste du message par le processeur ;
      // - sinon, émission du signal "message" contenant la ligne entière.
      // TODO
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
// TODO

// Commande "#connected"
// TODO

// Commande "#disconnected"
// TODO

// Commande "#renamed"
// TODO

// Commande "#list"
// TODO

// Commande "#private"
// TODO

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
  // TODO

  // Désactivation de la zone de saisie.
  input.setEnabled (false);

  // Envoi de messages lorsque la touche "entrée" est pressée.
  // - transmission du texte au moteur de messagerie instantanée ;
  // - effacement de la zone de saisie.
  // TODO

  // Connexion.
  // - affichage d'un message confirmant la connexion ;
  // - saisie de l'alias ;
  // - envoi de l'alias ;
  // - activation de la zone de saisie.
  // TODO

  // Déconnexion.
  // - désactivation de la zone de saisie.
  // - affichage d'un message pour signaler la déconnexion.
  // TODO

  // Messages.
  connect (&chat, &Chat::message, [this] (const QString & message) {
    text.append (message);
  });

  // Liste des utilisateurs.
  // Connexion d'un utilisateur.
  // Déconnexion d'un utilisateur.
  // Nouvel alias d'un utilisateur.
  // Message privé.
  // TODO

  // Gestion des erreurs.
  connect (&chat, &Chat::error, [this] (const QString & id) {
    QMessageBox::critical (this, tr("Error"), id);
  });

  // CONNEXION !
  text.append (tr("<b>Connecting...</b>"));
}

