/*
 *   Copyright (C) 2010,2012,2026 by Jonathan Naylor G4KLX
 *   copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef	RemoteHandler_H
#define	RemoteHandler_H

#include <string>

class CRemoteHandler {
public:
	CRemoteHandler();
	~CRemoteHandler();

	std::string process(const std::string& text);

private:
	std::string sendCallsigns() const;
	std::string sendRepeater(const std::string& callsign) const;
#if USE_STARNET
	std::string sendStarNetGroup(const std::string& callsign) const;
#endif
	std::string link(const std::string& callsign, RECONNECT reconnect, const std::string& reflector, bool respond);
	std::string unlink(const std::string& callsign, PROTOCOL protocol, const std::string& reflector);
#if USE_STARNET
	std::string logoff(const std::string& callsign, const std::string& user);
#endif

	std::string parseCallsignIn(const std::string& original) const;
	std::string parseCallsignOut(const std::string& original) const;
};

#endif
