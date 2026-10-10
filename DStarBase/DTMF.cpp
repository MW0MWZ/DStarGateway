/*
 *   Copyright (C) 2012,2013,2015,2026 by Jonathan Naylor G4KLX
 *   Copyright (C) 2011 by DV Developer Group. DJ0ABR
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include <cstdio>
#include <stdexcept>
#include "DTMF.h"
#include "Log.h"

CDTMF::CDTMF() :
m_data(),
m_command(),
m_pressed(false),
m_releaseCount(0U),
m_pressCount(0U),
m_lastChar(' ')
{
}

CDTMF::~CDTMF()
{
}

bool CDTMF::decode(const unsigned char* ambe, bool end)
{
	// DTMF begins with these byte values
	if (!end && (ambe[0] & DTMF_MASK[0]) == DTMF_SIG[0] && (ambe[1] & DTMF_MASK[1]) == DTMF_SIG[1] &&
				(ambe[2] & DTMF_MASK[2]) == DTMF_SIG[2] && (ambe[3] & DTMF_MASK[3]) == DTMF_SIG[3] &&
				(ambe[4] & DTMF_MASK[4]) == DTMF_SIG[4] && (ambe[5] & DTMF_MASK[5]) == DTMF_SIG[5] &&
				(ambe[6] & DTMF_MASK[6]) == DTMF_SIG[6] && (ambe[7] & DTMF_MASK[7]) == DTMF_SIG[7] &&
				(ambe[8] & DTMF_MASK[8]) == DTMF_SIG[8]) {
		unsigned char sym0 = ambe[4] & DTMF_SYM_MASK[0];
		unsigned char sym1 = ambe[5] & DTMF_SYM_MASK[1];
		unsigned char sym2 = ambe[7] & DTMF_SYM_MASK[2];
		unsigned char sym3 = ambe[8] & DTMF_SYM_MASK[3];

		char c = ' ';
		if (sym0 == DTMF_SYM0[0] && sym1 == DTMF_SYM0[1] && sym2 == DTMF_SYM0[2] && sym3 == DTMF_SYM0[3])
			c = '0';
		else if (sym0 == DTMF_SYM1[0] && sym1 == DTMF_SYM1[1] && sym2 == DTMF_SYM1[2] && sym3 == DTMF_SYM1[3])
			c = '1';
		else if (sym0 == DTMF_SYM2[0] && sym1 == DTMF_SYM2[1] && sym2 == DTMF_SYM2[2] && sym3 == DTMF_SYM2[3])
			c = '2';
		else if (sym0 == DTMF_SYM3[0] && sym1 == DTMF_SYM3[1] && sym2 == DTMF_SYM3[2] && sym3 == DTMF_SYM3[3])
			c = '3';
		else if (sym0 == DTMF_SYM4[0] && sym1 == DTMF_SYM4[1] && sym2 == DTMF_SYM4[2] && sym3 == DTMF_SYM4[3])
			c = '4';
		else if (sym0 == DTMF_SYM5[0] && sym1 == DTMF_SYM5[1] && sym2 == DTMF_SYM5[2] && sym3 == DTMF_SYM5[3])
			c = '5';
		else if (sym0 == DTMF_SYM6[0] && sym1 == DTMF_SYM6[1] && sym2 == DTMF_SYM6[2] && sym3 == DTMF_SYM6[3])
			c = '6';
		else if (sym0 == DTMF_SYM7[0] && sym1 == DTMF_SYM7[1] && sym2 == DTMF_SYM7[2] && sym3 == DTMF_SYM7[3])
			c = '7';
		else if (sym0 == DTMF_SYM8[0] && sym1 == DTMF_SYM8[1] && sym2 == DTMF_SYM8[2] && sym3 == DTMF_SYM8[3])
			c = '8';
		else if (sym0 == DTMF_SYM9[0] && sym1 == DTMF_SYM9[1] && sym2 == DTMF_SYM9[2] && sym3 == DTMF_SYM9[3])
			c = '9';
		else if (sym0 == DTMF_SYMA[0] && sym1 == DTMF_SYMA[1] && sym2 == DTMF_SYMA[2] && sym3 == DTMF_SYMA[3])
			c = 'A';
		else if (sym0 == DTMF_SYMB[0] && sym1 == DTMF_SYMB[1] && sym2 == DTMF_SYMB[2] && sym3 == DTMF_SYMB[3])
			c = 'B';
		else if (sym0 == DTMF_SYMC[0] && sym1 == DTMF_SYMC[1] && sym2 == DTMF_SYMC[2] && sym3 == DTMF_SYMC[3])
			c = 'C';
		else if (sym0 == DTMF_SYMD[0] && sym1 == DTMF_SYMD[1] && sym2 == DTMF_SYMD[2] && sym3 == DTMF_SYMD[3])
			c = 'D';
		else if (sym0 == DTMF_SYMS[0] && sym1 == DTMF_SYMS[1] && sym2 == DTMF_SYMS[2] && sym3 == DTMF_SYMS[3])
			c = '*';
		else if (sym0 == DTMF_SYMH[0] && sym1 == DTMF_SYMH[1] && sym2 == DTMF_SYMH[2] && sym3 == DTMF_SYMH[3])
			c = '#';

		LogDebug("Received DTMF Tone %c", c);

		if (c == m_lastChar) {
			m_pressCount++;
		} else {
			m_lastChar = c;
			m_pressCount = 0U;
		}

		if (c != ' ' && !m_pressed && m_pressCount >= 3U) {
			m_data.push_back(c);
			m_releaseCount = 0U;
			m_pressed = true;
		}

		return c != ' ';
	} else {
		// If it is not a DTMF Code
		if ((end || m_releaseCount >= 100U) && m_data.length() > 0U) {
			m_command = m_data;
			LogDebug("Received DTMF Command %s", m_command.c_str());
			m_data.clear();
			m_releaseCount = 0U;
		}

		m_pressed = false;
		m_releaseCount++;
		m_pressCount = 0U;
		m_lastChar = ' ';

		return false;
	}
}

bool CDTMF::hasCommand() const
{
	return m_command.size() > 0;
}

// DTMF to YOUR call command
std::string CDTMF::translate()
{
	std::string command = m_command;
	m_command.clear();

	// Last line of defence: a bad command must never crash the gateway
	try {
		return translateCommand(command);
	} catch (const std::exception& e) {
		LogWarning("DTMF command \"%s\" could not be parsed (%s), ignored", command.c_str(), e.what());
		return std::string("");
	}
}

std::string CDTMF::translateCommand(const std::string& command) const
{
	if (0 == command.size())
		return std::string("");

	if (0 == command.compare("#"))
		return "       U";

	if (0 == command.compare("0"))
		return "       I";

#ifdef USE_CCS
	if (0 == command.compare("A"))
		return "CA      ";
#endif

	if (0 == command.compare("00"))
		return "       I";

	if (0 == command.compare("**"))
		return "       L";

	if      (command.at(0) == '*')
		return processReflector("REF", command.substr(1));
	else if (command.at(0) == 'B')
		return processReflector("XRF", command.substr(1));
	else if (command.at(0) == 'D')
		return processReflector("DCS", command.substr(1));
	else
#ifdef USE_CCS
		return processCCS(command);
#else
		return std::string("");		// CCS is compiled out
#endif
}

void CDTMF::reset()
{
	LogDebug("DTMF Reset");
	m_data.clear();
	m_command.clear();
	m_pressed = false;
	m_pressCount = 0U;
	m_releaseCount = 0U;
	m_lastChar = ' ';
}

// Non-empty and all digits, so safe for std::stoul
bool CDTMF::isNumber(const std::string& str)
{
	return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

std::string CDTMF::processReflector(const std::string& prefix, const std::string& command) const
{
	unsigned int len = command.size();

	if(len == 0U)
		return std::string("");

	char c = command.at(len - 1U);
	if (c == 'A' || c == 'B' || c == 'C' || c == 'D') {
		if (len < 2U || len > 4U || !isNumber(command.substr(0, len-1U)))
			return std::string("");

		unsigned long n = std::stoul(command.substr(0, len-1U));
		if (n == 0UL)
			return std::string("");

		char ostr[32];
		snprintf(ostr, 32, "%s%03lu%cL", prefix.c_str(), n, c);
	
		return std::string(ostr);
	} else {
		if (len < 3U || len > 5U || !isNumber(command.substr(0, len-2U)) || !isNumber(command.substr(len-2U)))
			return std::string("");

		unsigned long n1 = std::stoul(command.substr(0,len-2U));
		if (n1 == 0UL)
			return std::string("");

		unsigned long n2 = std::stoul(command.substr(len-2U));
		if (n2 == 0UL || n2 > 26UL)
			return std::string("");

		c = 'A' + n2 - 1UL;

		char ostr[32];
		std::snprintf(ostr, 32, "%s%03lu%cL", prefix.c_str(), n1, c);
	
		return std::string(ostr);
	}
}

std::string CDTMF::processCCS(const std::string& command) const
{
	unsigned int len = command.size();
	if (len == 0U)
		return std::string("");

	// Digits, plus an optional trailing band letter
	char band = command.at(len - 1U);
	bool hasBand = band == 'A' || band == 'B' || band == 'C' || band == 'D';
	std::string digits = hasBand ? command.substr(0U, len - 1U) : command;
	if (!isNumber(digits))
		return std::string("");

	// Only these CCS7 formats are valid
	unsigned int count = digits.size();
	if (count != 3U && count != 4U && count != 6U && count != 7U)
		return std::string("");

	unsigned long n = std::stoul(digits);
	if (n == 0UL)
		return std::string("");

	char ostr[32];
	switch (count) {
		case 3U:	// local repeater
			if (hasBand)
				snprintf(ostr, 32, "C%03lu%c   ", n, band);
			else
				snprintf(ostr, 32, "C%03lu    ", n);
			break;
		case 4U:	// local user, or local hotspot with band
			if (hasBand)
				snprintf(ostr, 32, "C%04lu%c  ", n, band);
			else
				snprintf(ostr, 32, "C%04lu   ", n);
			break;
		case 6U:	// full repeater
			if (hasBand)
				snprintf(ostr, 32, "C%06lu%c", n, band);
			else
				snprintf(ostr, 32, "C%06lu ", n);
			break;
		case 7U:	// full user or hotspot, or full hotspot with band
			if (hasBand)
				snprintf(ostr, 32, "C%07lu%c", n, band);	// 9 chars, as in ircDDBGateway
			else
				snprintf(ostr, 32, "C%07lu", n);
			break;
		default:
			return std::string("");
	}

	return std::string(ostr);
}
