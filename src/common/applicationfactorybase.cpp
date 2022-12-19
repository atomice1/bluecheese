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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QIODevice>
#include <QLocale>
#include <QStringList>
#include <QTextStream>
#include <QTranslator>
#include <memory>

#include "applicationbase.h"
#include "applicationfactorybase.h"
#include "version.h"

namespace {
    const QLatin1String CHESSBOARD_COMMON_TRANSLATION("chessboard-common");
}

ApplicationFactoryBase::ApplicationFactoryBase(const QString& applicationName,
                                               const QString& translationName) :
    m_applicationName(applicationName),
    m_translationName(!translationName.isNull() ? translationName : applicationName)
{
}

QCommandLineParser *ApplicationFactoryBase::createCommandLineParser()
{
    std::unique_ptr<QCommandLineParser> parser(new QCommandLineParser());
    parser->addHelpOption();
    parser->addVersionOption();
    addCommonCommandLineOptions(parser.get());
    addCommandLineOptions(parser.get());
    return parser.release();
}

void ApplicationFactoryBase::addCommandLineOptions(QCommandLineParser *parser)
{
}

void ApplicationFactoryBase::addCommonCommandLineOptions(QCommandLineParser *parser)
{
}

bool ApplicationFactoryBase::validateArguments(QCommandLineParser *, QString *)
{
    return true;
}

int common_main(ApplicationFactoryBase *factory)
{
    QCoreApplication::setApplicationName(factory->applicationName());
    QCoreApplication::setApplicationVersion(VERSION);

    const QStringList uiLanguages = QLocale::system().uiLanguages();
    QStringList components;
    components << factory->translationName() << CHESSBOARD_COMMON_TRANSLATION;
    for (const QString& component : components) {
        QTranslator *translator = new QTranslator(qApp);
        for (const QString &locale : uiLanguages) {
            const QString baseName = component + "_" + QLocale(locale).name();
            if (translator->load(":/i18n/" + baseName)) {
                qApp->installTranslator(translator);
                break;
            }
        }
    }

    QScopedPointer<QCommandLineParser> parser(factory->createCommandLineParser());
    parser->process(qApp->arguments());
    QString errorMessage;
    bool success = factory->validateArguments(parser.get(), &errorMessage);
    if (!success) {
        QTextStream ts(stderr, QIODevice::WriteOnly);
        ts << errorMessage << "\n";
        return 1;
    }
    QScopedPointer<ApplicationBase> app(factory->create(parser.get()));
    return qApp->exec();
}
