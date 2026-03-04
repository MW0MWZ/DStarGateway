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

#include <cassert>
#include <vector>

#include "RepeaterHandler.h"
#ifdef USE_STARNET
#include "StarNetHandler.h"
#endif
#include "RemoteHandler.h"
#include "DExtraHandler.h"
#include "DPlusHandler.h"
#include "DStarDefines.h"
#include "DCSHandler.h"
#include "Utils.h"
#include "Log.h"

CRemoteHandler::CRemoteHandler()
{
}

CRemoteHandler::~CRemoteHandler()
{
}

std::string CRemoteHandler::process(const std::string& text)
{
	std::vector<std::string> m_tokens = CUtils::stringTokenizer(text);

	if (m_tokens.empty())
		return "NAK No command received";

	std::string command = m_tokens.at(0);

	if (command == "Callsigns") {
		return sendCallsigns();

	} else if (command == "Repeater") {
		if (m_tokens.size() < 2)
			return "NAK No callsign received";

		std::string callsign = parseCallsignIn(m_tokens.at(1));
		return sendRepeater(callsign);
	}

#ifdef USE_STARNET
	else if (command == "StarNet") {
		if (m_tokens.size() < 2)
			return "NAK No callsign received";

		std::string callsign = parseCallsignIn(m_tokens.at(1));
		return sendStarNetGroup(callsign);
	}
#endif

	else if (command == "Link") {
		if (m_tokens.size() < 4)
			return "NAK No callsign and/or reconnect and/or reflector received";

		std::string callsign  = parseCallsignIn(m_tokens.at(1));
		RECONNECT reconnect   = stringToReconnect(m_tokens.at(2));
		std::string reflector = parseCallsignIn(m_tokens.at(3));

		if (reflector.empty())
			LogInfo("Remote control user requesting link \"%s\" to \"None\" with reconnect %s", callsign.c_str(), reconnectToString(reconnect));
		else
			LogInfo("Remote control user requesting link \"%s\" to \"%s\" with reconnect %s", callsign.c_str(), reflector.c_str(), reconnectToString(reconnect));

		return link(callsign, reconnect, reflector, true);

	} else if (command == "Unlink") {
		if (m_tokens.size() < 4)
			return "NAK No callsign and/or protocol and/or reflector received";

		std::string callsign  = parseCallsignIn(m_tokens.at(1));
		PROTOCOL protocol     = stringToProtocol(m_tokens.at(2));
		std::string reflector = parseCallsignIn(m_tokens.at(3));

		LogInfo("Remote control user requesting unlink \"%s\" from \"%s\" for protocol %s", callsign.c_str(), reflector.c_str(), protocolToString(protocol));
		return unlink(callsign, protocol, reflector);

	}

#ifdef USE_STARNET
	else if (command == "Logoff") {
		if (m_tokens.size() < 3)
			return "NAK No callsign and/or user received";

		std::string callsign = parseCallsignIn(m_tokens.at(1));
		std::string user     = parseCallsignIn(m_tokens.at(2));

		m_handler.readLogoff(callsign, user);
		LogInfo("Remote control user has logged off \"%s\" from \"%s\"", user.c_str(), callsign.c_str());
		return logoff(callsign, user);
	}
#endif

	else {
		return "NAK Invalid command received";
	}
}

std::string CRemoteHandler::sendCallsigns() const
{
	std::string response = "Callsigns";

	std::vector<std::string> repeaters = CRepeaterHandler::listDVRepeaters();
	for (const auto& it : repeaters)
		response += " R:" + parseCallsignOut(it);
#if USE_STARNET
	std::vector<std::string> starNets  = CStarNetHandler::listStarNets();
	for (const auto& it : starNets)
		response += " S:" + parseCallsignOut(it);
#endif

	return response;
}

std::string CRemoteHandler::sendRepeater(const std::string& callsign) const
{
	CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(callsign);
	if (repeater == nullptr) {
		LogInfo("Remote requesting invalid repeater \"%s\"", callsign.c_str());
		return "NAK Invalid repeater callsign";
	}

	std::string response = "Repeater";

	CRemoteRepeaterData* data = repeater->getInfo();
	if (data != nullptr) {
		CDExtraHandler::getInfo(repeater, *data);
		CDPlusHandler::getInfo(repeater, *data);
		CDCSHandler::getInfo(repeater, *data);
#ifdef USE_CCS
		CCCSHandler::getInfo(repeater, *data);
#endif
		char buffer[50U];

		::sprintf(buffer, " %s %s %s", data->getCallsign().c_str(), reconnectToString(data->getReconnect()), data->getReflector().c_str());
		response += buffer;

		for (unsigned int n = 0U; n < data->getLinkCount(); n++) {
			CRemoteLinkData* link = data->getLink(n);

			::sprintf(buffer, " %s %s %d %s %d", link->getCallsign().c_str(), protocolToString(link->getProtocol()), int(link->isLinked()), directionToString(link->getDirection()), int(link->isDongle()));
			response += buffer;
		}
	}

	delete data;

	return response;
}

#ifdef USE_STARNET
std::string CRemoteHandler::sendStarNetGroup(const std::string& callsign) const
{
	CStarNetHandler* starNet = CStarNetHandler::findStarNet(callsign);
	if (starNet == nullptr)
		return "NAK Invalid STARnet Group callsign";

	std::string response = "StarNet";

	CRemoteStarNetGroup* data = starNet->getInfo();
	if (data != nullptr)
		char buffer[50U];

		::sprintf(buffer, " %s %s %d %d ", data->getCallsign().c_str(), data->getLogoff().c_str(), data->getTimer(), data->getTimeout());
		response += buffer;

		for (unsigned int n = 0U; n < data->.getUserCount(); n++) {
			CRemoteStarNetUser& user = data->getUser(n);

			::sprintf(buffer, "%s %d %d ", user.getCallsign().c_str(), user.getTimer(), user.getTimeout());
			response += buffer;
		}
	}

	delete data;

	return response;
}
#endif

std::string CRemoteHandler::link(const std::string& callsign, RECONNECT reconnect, const std::string& reflector, bool respond)
{
	CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(callsign);
	if (repeater == nullptr) {
		LogInfo("Remote requesting link for invalid repeater \"%s\"", callsign.c_str());
		return "NAK Invalid repeater callsign";
	}

	repeater->link(reconnect, reflector);

	if (respond)
		return "ACK";
	else
		return "";
}

std::string CRemoteHandler::unlink(const std::string& callsign, PROTOCOL protocol, const std::string& reflector)
{
	CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(callsign);
	if (repeater == nullptr) {
		LogInfo("Remote requesting unlink for invalid repeater \"%s\"", callsign.c_str());
		return "NAK Invalid repeater callsign";
	}

	repeater->unlink(protocol, reflector);

	return "ACK";
}

#if USE_STARNET
std::string CRemoteHandler::logoff(const std::string& callsign, const std::string& user)
{
	CStarNetHandler* starNet = CStarNetHandler::findStarNet(callsign);
	if (starNet == nullptr)
		return "NAK Invalid STARnet group callsign";

	bool res = starNet->logoff(user);
	if (!res)
		return "NAK Invalid STARnet user callsign";
	else
		return "ACK";
}
#endif

std::string CRemoteHandler::parseCallsignIn(const std::string& original) const
{
	std::string callsign = original;

	CUtils::replaceChar(callsign, '_', ' ');

	callsign.resize(LONG_CALLSIGN_LENGTH, ' ');

	return callsign;
}

std::string CRemoteHandler::parseCallsignOut(const std::string& original) const
{
	std::string callsign = original;

	CUtils::trim(callsign);

	CUtils::replaceChar(callsign, ' ', '_');

	return callsign;
}

