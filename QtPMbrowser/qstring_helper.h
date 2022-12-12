/*
	Copyright 2021 Christian R. Halaszovich

	 This file is part of PMbrowser.

	PMbrowser is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PMbrowser is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PMbrowser.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once
#ifndef QSTRING_HELPER_H
#define QSTRING_HELPER_H

#include <string_view>

#if QT_VERSION < QT_VERSION_CHECK(6,4,0)
#include <QString>

/// <summary>
/// create a QString from a latin-1 encoded std::string_view
/// </summary>
/// <param name="sv">std::string_view object to be converted</param>
/// <returns>newly created QString</returns>
inline QString qs_from_sv(const std::string_view& sv)
{
	return QString::fromLatin1(sv.data(), static_cast<int>(sv.size()));
}

#else
#include <QLatin1StringView>

inline const QLatin1StringView  qs_from_sv(const std::string_view& sv)
{
	return {sv.data(), static_cast<qsizetype>(sv.size())};
}

#endif

#endif // !QSTRING_HELPER_H
