#include <QApplication>
#include "Chat.h"

int main (int argc, char * argv [])
{
    QApplication a (argc, argv);

    QCoreApplication::setOrganizationName ("aassif");
    QCoreApplication::setApplicationName ("chat");

    ChatWindow w ("linux", 3101);
    w.show ();

    return a.exec ();
}
