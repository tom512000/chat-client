#ifndef CHAT_H
#define CHAT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QTcpSocket>
#include <QListWidget>

// Chat hérite de QObject
class Chat : public QObject
{
  Q_OBJECT // signaux + slots

  private:
    // Signature d'une méthode dédiée au traitement d'un type de message.
    typedef void (Chat::*Processor) (QTextStream &);
    // Tableau associatif statique qui associe une chaîne de caractères à un processeur.
    static const std::map<QString, Processor> PROCESSORS;

  private:
    QTcpSocket socket;

  private:
    // Traitement d'un message.
    void process (const QString & message);
    // Gestion des erreurs.
    void process_error (QTextStream &);
    // Gestion de l'alias
    void process_alias(QTextStream &is);
    // Gestion de l'utilisateur connecté
    void process_user_connected(QTextStream &is);
    // Gestion de l'utilisateur déconnecté
    void process_user_disconnected(QTextStream &is);
    // Gestion du renommage de l'utilisateur
    void process_user_renamed(QTextStream &is);
    // Gestion de l'affichage des utilisateurs
    void process_user_list(QTextStream &is);
    // Gestion des messages privés
    void process_user_private(QTextStream &is);

  public:
    // constructeur : nom du serveur, port et, éventuellement, objet parent.
    Chat (const QString & host, quint16 port, QObject * parent = nullptr);
    ~Chat ();

    // Envoi d'un message.
    void write (const QString &);

  signals:
    // Connexion
    void connected (const QString & host, quint16 port);
    // Déconnexion
    void disconnected ();
    // Message.
    void message (const QString & message);
    // Error
    void error (const QString & id);
    // Alias
    void alias(const QString &newAlias);
    // Utilisateur connecté
    void user_connected(const QString &username);
    // Utilisateur déconnecté
    void user_disconnected(const QString &username);
    // Utilisateur renommé
    void user_renamed(const QString &oldUsername, const QString &newUsername);
    // Liste des utilisateurs
    void user_list(const QStringList &userList);
    // Envoie d'un message privé
    void user_private(const QString &sender, const QString &message);
};

// ChatWindow hérite de QMainWindow.
class ChatWindow : public QMainWindow
{
  Q_OBJECT

  private:
    // Moteur de messagerie instantanée.
    Chat chat;
    // Zone de texte.
    QTextEdit text;
    // Zone de saisie.
    QLineEdit input;
    // Liste des utilisateurs
    QListWidget userListWidget;

  private slots:
    void onUserListDoubleClicked(QListWidgetItem *item);

  public:
    // Constructeur.
    ChatWindow (const QString & host, quint16 port, QWidget * parent = nullptr);
};

#endif // CHAT_H
