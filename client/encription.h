#ifndef ENCRIPTION_H
#define ENCRIPTION_H

#include <QtCore>
//          Vigenere Cipher
QByteArray k = "-XEpzq^^yEfA9dpr";

QString encript(QString text)
{
    for (int i = 0; i < text.length(); i++) {
        text[i] = (QChar(text[i]).unicode()+QChar(k[i%k.length()]).unicode())%65535;
    }
    return text;
}

QString decript(QString text)
{
    for (int i = 0; i < text.length(); i++) {
        text[i] = (QChar(text[i]).unicode()-QChar(k[i%k.length()]).unicode()+65535)%65535;
    }
    return text;
}

#endif // ENCRIPTION_H
