/*
 * bluecheese
 * Copyright (C) 2022-2023 Chris January
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef APPLICATIONFACTORY_H
#define APPLICATIONFACTORY_H

#include <QCommandLineOption>
#include <QObject>
#include <QString>

class ApplicationBase;
class QCommandLineParser;

class ApplicationFactoryBase
{
public:
    ApplicationFactoryBase(const QString& applicationName,
                           const QString& translationName = QString());
    virtual QCommandLineParser *createCommandLineParser();
    virtual void addCommandLineOptions(QCommandLineParser *parser);
    virtual ApplicationBase *create(QCommandLineParser *parser) = 0;
    virtual bool validateArguments(QCommandLineParser *parser, QString *errorMessage);
    const QString applicationName() const { return m_applicationName; }
    const QString translationName() const { return m_translationName; }
private:
    void addCommonCommandLineOptions(QCommandLineParser *parser);
    QString m_applicationName;
    QString m_translationName;
};

int common_main(ApplicationFactoryBase *factory);

#endif // APPLICATIONFACTORY_H
