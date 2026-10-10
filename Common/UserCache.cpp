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

#include "UserCache.h"

// Users move between repeaters, so re-query ircDDB once an entry is this old
const std::chrono::minutes USER_LIFETIME(5);

CUserCache::CUserCache()
{
}

CUserCache::~CUserCache()
{
	for (std::unordered_map<std::string, CUserRecord *>::iterator it = m_cache.begin(); it != m_cache.end(); ++it)
		delete it->second;
	m_cache.clear();
}

CUserRecord* CUserCache::find(const std::string& user)
{
	std::unordered_map<std::string, CUserRecord *>::iterator it = m_cache.find(user);
	if (it == m_cache.end())
		return NULL;

	if (it->second->isExpired(USER_LIFETIME)) {
		delete it->second;
		m_cache.erase(it);
		return NULL;
	}

	return it->second;
}

void CUserCache::update(const std::string& user, const std::string& repeater, const std::string& timestamp)
{
	std::unordered_map<std::string, CUserRecord *>::iterator it = m_cache.find(user);

	if (it == m_cache.end()) {
		// A brand new record is needed
		m_cache[user] = new CUserRecord(user, repeater, timestamp);
		return;
	}

	CUserRecord* rec = it->second;
	int age = timestamp.compare(rec->getTimeStamp());
	if (age > 0) {
		// Update an existing record, but only if the received timestamp is newer
		rec->setRepeater(repeater);
		rec->setTimestamp(timestamp);
	}

	// An older report doesn't confirm where the user is now
	if (age >= 0)
		rec->touch();
}

void CUserCache::prune()
{
	for (std::unordered_map<std::string, CUserRecord *>::iterator it = m_cache.begin(); it != m_cache.end();) {
		if (it->second->isExpired(USER_LIFETIME)) {
			delete it->second;
			it = m_cache.erase(it);
		} else {
			++it;
		}
	}
}

void CUserCache::clear()
{
	for (std::unordered_map<std::string, CUserRecord *>::iterator it = m_cache.begin(); it != m_cache.end(); ++it)
		delete it->second;
	m_cache.clear();
}

unsigned int CUserCache::getCount() const
{
	return m_cache.size();
}
