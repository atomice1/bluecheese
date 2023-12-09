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

#ifndef APPLICATIONBASE_H
#define APPLICATIONBASE_H

#include <QObject>
#include "options.h"

class ApplicationFacade;

class ApplicationBase : public QObject
{
    Q_OBJECT
public:
    enum class Feature {
        Ai
    };

    explicit ApplicationBase(const Options *options, QObject *parent = nullptr);
    virtual ~ApplicationBase();

    bool isFeatureEnabled(Feature feature) const;

protected:
    ApplicationFacade *facade() { return m_facade; }
    template<typename T>
    const T *options() const {
        static_assert(std::is_base_of<Options, T>::value, "T not derived from Options");
        return static_cast<const T *>(m_options);
    }

private:
    ApplicationFacade *m_facade {};
    const Options *m_options {};
};

#endif // APPLICATIONBASE_H
