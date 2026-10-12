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

#include "RepeaterCache.h"

// Repeaters rarely change gateway
const std::chrono::hours REPEATER_LIFETIME(24);

CRepeaterCache::CRepeaterCache()
{
}

CRepeaterCache::~CRepeaterCache()
{
	for (std::unordered_map<std::string, CRepeaterRecord *>::iterator it = m_cache.begin(); it != m_cache.end(); ++it)
		delete it->second;
}

CRepeaterRecord* CRepeaterCache::find(const std::string& repeater)
{
	std::unordered_map<std::string, CRepeaterRecord *>::iterator it = m_cache.find(repeater);
	if (it == m_cache.end())
		return NULL;

	if (!it->second->isLocked() && it->second->isExpired(REPEATER_LIFETIME)) {
		delete it->second;
		m_cache.erase(it);
		return NULL;
	}

	return it->second;
}

void CRepeaterCache::update(const std::string& repeater, const std::string& gateway, bool locked)
{
	std::unordered_map<std::string, CRepeaterRecord *>::iterator it = m_cache.find(repeater);

	if (it == m_cache.end()) {
		// A brand new record is needed
		m_cache[repeater] = new CRepeaterRecord(repeater, gateway, locked);
		return;
	}

	// ircDDB never overrides a local repeater
	CRepeaterRecord* rec = it->second;
	if (rec->isLocked() && !locked)
		return;

	// Update an existing record
	rec->setGateway(gateway);
	if (locked)
		rec->lock();
	rec->touch();
}

void CRepeaterCache::prune()
{
	for (std::unordered_map<std::string, CRepeaterRecord *>::iterator it = m_cache.begin(); it != m_cache.end();) {
		if (!it->second->isLocked() && it->second->isExpired(REPEATER_LIFETIME)) {
			delete it->second;
			it = m_cache.erase(it);
		} else {
			++it;
		}
	}
}

unsigned int CRepeaterCache::getCount() const
{
	return m_cache.size();
}
