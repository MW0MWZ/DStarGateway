/*
 *   Copyright (C) 2010 by Jonathan Naylor G4KLX
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

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

class CRepeaterRecord {
public:
	CRepeaterRecord(const std::string& repeater, const std::string& gateway, bool locked) :
	m_repeater(repeater),
	m_gateway(gateway),
	m_locked(locked),
	m_received(std::chrono::steady_clock::now())
	{
	}

	std::string getRepeater() const
	{
		return m_repeater;
	}

	std::string getGateway() const
	{
		return m_gateway;
	}

	void setGateway(const std::string& gateway)
	{
		m_gateway = gateway;
	}

	// Local repeaters are locked, only ircDDB entries are learned
	bool isLocked() const
	{
		return m_locked;
	}

	void lock()
	{
		m_locked = true;
	}

	// Restart the lifetime, called whenever the entry is received
	void touch()
	{
		m_received = std::chrono::steady_clock::now();
	}

	bool isExpired(std::chrono::steady_clock::duration lifetime) const
	{
		return std::chrono::steady_clock::now() - m_received > lifetime;
	}

private:
	std::string m_repeater;
	std::string m_gateway;
	bool        m_locked;
	std::chrono::steady_clock::time_point m_received;
};

class CRepeaterCache {
public:
	CRepeaterCache();
	~CRepeaterCache();

	CRepeaterRecord* find(const std::string& repeater);

	void update(const std::string& repeater, const std::string& gateway, bool locked);

	void prune();

	unsigned int getCount() const;

private:
	std::unordered_map<std::string, CRepeaterRecord *> m_cache;
};
