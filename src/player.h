/*
 *  player.h
 *  StatusSpec project
 *
 *  Copyright (c) 2014-2015 Forward Command Post
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#pragma once

#include "ehandle.h"

class C_BaseCombatWeapon;
class C_BaseEntity;
class CSteamID;
class IClientEntity;

#include "tfdefs.h"

class Player {
public:
	Player(int entindex);
	Player(IClientEntity *entity);
	Player() {};

	Player& operator=(int entindex);
	Player& operator=(IClientEntity *entity);
	Player& operator=(const Player &player);

	bool operator==(int entindex) const;
	bool operator==(IClientEntity *entity) const;
	bool operator==(const Player &player) const;

	bool operator!=(int entindex) const;
	bool operator!=(IClientEntity *entity) const;
	bool operator!=(const Player &player) const;

	bool operator<(int entindex) const;
	bool operator<(IClientEntity *entity) const;
	bool operator<(const Player &player) const;

	bool operator<=(int entindex) const;
	bool operator<=(IClientEntity *entity) const;
	bool operator<=(const Player &player) const;

	bool operator>(int entindex) const;
	bool operator>(IClientEntity *entity) const;
	bool operator>(const Player &player) const;

	bool operator>=(int entindex) const;
	bool operator>=(IClientEntity *entity) const;
	bool operator>=(const Player &player) const;

	operator bool() const;
	operator IClientEntity *() const;

	IClientEntity *operator->() const;

	IClientEntity *GetEntity() const;

	bool CheckCondition(TFCond condition) const;
	TFClassType GetClass() const;
	int GetHealth() const;
	int GetMaxHealth() const;
	std::string GetName() const;
	int GetObserverMode() const;
	C_BaseEntity *GetObserverTarget() const;
	CSteamID GetSteamID() const;
	TFTeam GetTeam() const;
	int GetUserID() const;
	C_BaseCombatWeapon *GetWeapon(int i) const;
	bool IsAlive() const;

	class Iterator {
		friend class Player;

	public:
		Iterator(const Iterator&);
		~Iterator() = default;
		Iterator& operator=(const Iterator&);
		Iterator& operator++();
		Player operator*() const;
		friend void swap(Iterator& lhs, Iterator& rhs);
		Iterator operator++(int);
		Player *operator->() const;
		friend bool operator==(const Iterator&, const Iterator&);
		friend bool operator!=(const Iterator&, const Iterator&);
		Iterator();
		Iterator& operator--();
		Iterator operator--(int);

	private:
		Iterator(int index);
		int index;
	};

	static Iterator begin();
	static Iterator end();

	class Iterable {
	public:
		Iterator begin();
		Iterator end();
	};

	static bool CheckDependencies();
	static bool classRetrievalAvailable;
	static bool comparisonAvailable;
	static bool conditionsRetrievalAvailable;
	static bool nameRetrievalAvailable;
	static bool steamIDRetrievalAvailable;
	static bool userIDRetrievalAvailable;
private:
	CHandle<IClientEntity> playerEntity;

	bool IsValid() const;
	bool IsEqualTo(const Player &player) const;
	bool IsNotEqualTo(const Player &player) const;
	bool IsLessThan(const Player &player) const;
	bool IsLessThanOrEqualTo(const Player &player) const;
	bool IsGreaterThan(const Player &player) const;
	bool IsGreaterThanOrEqualTo(const Player &player) const;
};